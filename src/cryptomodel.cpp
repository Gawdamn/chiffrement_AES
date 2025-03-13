#include "headers/cryptomodel.h"
#include "headers/mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <openssl/evp.h>
#include <openssl/rand.h>


CryptoModel::CryptoModel(QObject *parent) : QObject(parent)
{
    QSettings settings("PFE", "chiffrementAES");
    m_aesKeySize = settings.value("aesKeySize", 256).toInt(); // Valeur par défaut 256 bits
    deleteOriginal = settings.value("deleteOriginal", false).toBool();
}


// Implémentation de encryptFile
bool CryptoModel::encryptFile(const QString &inputFile, const QString &outputFile, const QString &password)
{
    const int AES_KEY_LENGTH = m_aesKeySize / 8;  // taille de la clé AES utilisée (division par 8 pour la conversion en octets)
    const int AES_BLOCK_SIZE = 16; // Taille d'un bloc AES
    const int GCM_IV_LENGTH = 12;  // Taille de l'IV pour GCM
    const int GCM_TAG_LENGTH = 16; // Taille du tag pour GCM

    // Générer un IV aléatoire pour GCM
    unsigned char iv[GCM_IV_LENGTH];
    if (!RAND_bytes(iv, sizeof(iv))) {
        emit errorOccurred("Erreur lors de la génération de l'IV");
        return false;
    }


    // Dériver une clé à partir du mot de passe en utilisant PBKDF2
    unsigned char key[AES_KEY_LENGTH];
    unsigned char salt[16];
    if (!RAND_bytes(salt, sizeof(salt))) {
        emit errorOccurred("Erreur lors de la génération du sel");
        return false;
    }


    if (!PKCS5_PBKDF2_HMAC(
            password.toStdString().c_str(),            // Mot de passe
            password.size(),                           // Taille du mot de passe
            salt,                                      // Sel
            sizeof(salt),                              // Taille du sel
            10000,                                     // Nombre d'itérations
            EVP_sha256(),                              // Fonction de hachage (SHA-256)
            AES_KEY_LENGTH,                            // Taille de la clé AES (32 pour AES-256)
            key                                       // Clé générée
            )) {
        emit errorOccurred("Erreur lors de la dérivation de la clé");
        return false;
    }


    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        emit errorOccurred("Impossible d'ouvrir le fichier d'entrée");
        return false;
    }
    QByteArray originalHash = computeFileHash(inputFile);   // Calcul du hash du fichier original

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        emit errorOccurred("Impossible d'ouvrir le fichier de sortie");
        return false;
    }

    // Écrire le sel, l'IV et le hash original (32 octets)
    outFile.write(reinterpret_cast<const char *>(salt), sizeof(salt));
    outFile.write(reinterpret_cast<const char *>(iv), sizeof(iv));
    outFile.write(originalHash);


    // Initialiser le contexte de chiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        emit errorOccurred("Erreur lors de l'initialisation du contexte de chiffrement");
        return false;
    }

    const EVP_CIPHER *cipher = nullptr;
    //choix dynamique du cipher en fonction de la clé AES utilisée
    if(m_aesKeySize == 128) {
        cipher = EVP_aes_128_gcm();
    } else if(m_aesKeySize == 192) {
        cipher = EVP_aes_192_gcm();
    } else { // m_aesKeySize == 256
        cipher = EVP_aes_256_gcm();
    }

    if (!EVP_EncryptInit_ex(ctx, cipher, nullptr, nullptr, nullptr)) {
        emit errorOccurred("Erreur lors de l'initialisation du chiffrement");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Configurer la clé et l'IV
    if (!EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv)) {
        emit errorOccurred("Erreur lors de la configuration de la clé et de l'IV");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    emit setVisible(true);
    emit progressChanged(0);

    // Lire et chiffrer les données
    qint64 totalFileSize = inFile.size();
    qint64 totalRead = 0;
    unsigned char inBuffer[4096];
    unsigned char outBuffer[4096 + AES_BLOCK_SIZE];
    int bytesRead;
    int outLen;

    while ((bytesRead = inFile.read(reinterpret_cast<char *>(inBuffer), sizeof(inBuffer))) > 0) {
        if (!EVP_EncryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead)) {
            emit errorOccurred("Erreur lors du chiffrement");
            EVP_CIPHER_CTX_free(ctx);
            emit setVisible(false);
            return false;
        }
        totalRead += bytesRead;
        int progress = static_cast<int>((totalRead * 100) / totalFileSize);
        emit progressChanged(progress);     // Mise à jour de la barre de progression
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
    }

    // Finaliser le chiffrement
    if (!EVP_EncryptFinal_ex(ctx, outBuffer, &outLen)) {
        emit errorOccurred("Erreur lors de la finalisation du chiffrement");
        EVP_CIPHER_CTX_free(ctx);
        emit setVisible(false);
        return false;
    }
    outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);

    // Obtenir et écrire le tag d'authentification
    unsigned char tag[GCM_TAG_LENGTH];
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag)) {
        emit errorOccurred("Erreur lors de la génération du tag");
        EVP_CIPHER_CTX_free(ctx);
        emit setVisible(false);
        return false;
    }
    outFile.write(reinterpret_cast<const char *>(tag), sizeof(tag));

    emit progressChanged(100);

    // Nettoyer et fermer les fichiers
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    emit setVisible(false);

    // Appliquer ou non la suppression du fichier original
    if (deleteOriginal) {
        if (!QFile::remove(inputFile)) {
            emit errorOccurred("Échec de la suppression du fichier original.");
        }
    }
    return true;
}

// Implémentation de decryptFile
bool CryptoModel::decryptFile(const QString &inputFile, const QString &outputFile, const QString &password)
{
    const int AES_KEY_LENGTH = m_aesKeySize / 8;  // taille de la clé AES utilisée
    const int GCM_IV_LENGTH = 12;  // Taille de l'IV pour GCM
    const int GCM_TAG_LENGTH = 16; // Taille du tag pour GCM

    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        emit errorOccurred("Impossible d'ouvrir le fichier d'entrée");
        return false;
    }

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        emit errorOccurred("Impossible d'ouvrir le fichier de sortie");
        return false;
    }

    // Lire le sel
    unsigned char salt[16];
    if (inFile.read(reinterpret_cast<char *>(salt), sizeof(salt)) != sizeof(salt)) {
        emit errorOccurred("Erreur lors de la lecture du sel");
        return false;
    }

    // Lire l'IV
    unsigned char iv[GCM_IV_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(iv), sizeof(iv)) != sizeof(iv)) {
        emit errorOccurred("Erreur lors de la lecture de l'IV");
        return false;
    }

    // Lire le hash
    QByteArray storedHash = inFile.read(32);
    if (storedHash.size() != 32) {
        emit errorOccurred("Erreur lors de la lecture du hash original");
        return false;
    }

    // Dériver la clé à partir du mot de passe
    unsigned char key[AES_KEY_LENGTH];
    if (!PKCS5_PBKDF2_HMAC(
            password.toStdString().c_str(),
            password.size(),
            salt,
            sizeof(salt),
            10000,
            EVP_sha256(),
            AES_KEY_LENGTH,
            key)) {
        emit errorOccurred("Erreur lors de la dérivation de la clé");
        return false;
    }


    // Initialiser le contexte de déchiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        emit errorOccurred("Erreur lors de l'initialisation du contexte de déchiffrement");
        return false;
    }

    const EVP_CIPHER *cipher = nullptr;
    //choix dynamique du cipher en fonction de la clé AES utilisée
    if(m_aesKeySize == 128) {
        cipher = EVP_aes_128_gcm();
    } else if(m_aesKeySize == 192) {
        cipher = EVP_aes_192_gcm();
    } else { // m_aesKeySize == 256
        cipher = EVP_aes_256_gcm();
    }

    if (!EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv)) {
        emit errorOccurred("Erreur lors de la configuration de la clé et de l'IV");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Se déplacer à la position du tag en utilisant la taille actuelle du fichier
    inFile.seek(inFile.size() - GCM_TAG_LENGTH);

    unsigned char tag[GCM_TAG_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(tag), sizeof(tag)) != sizeof(tag)) {
        emit errorOccurred("Erreur lors de la lecture du tag d'authentification");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Définir le tag pour la vérification
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_LENGTH, tag);

    // Calculer la longueur du ciphertext.
    // Le fichier chiffré est structuré comme suit :
    // [salt (16 octets)] [IV (12 octets)] [hash (32 octets)] [ciphertext] [tag (16 octets)]
    const int metadataSize = sizeof(salt) + sizeof(iv) + storedHash.size() + GCM_TAG_LENGTH;
    qint64 ciphertextLength = inFile.size() - metadataSize;
    qint64 totalRead = 0;

    unsigned char inBuffer[4096];
    unsigned char outBuffer[4096];
    int bytesRead, outLen;

    // Afficher la barre de progression au début
    emit setVisible(true);
    emit progressChanged(0);

    // Revenir au début du ciphertext (après sel et IV)
    inFile.seek(sizeof(salt) + sizeof(iv) + storedHash.size());

    while (totalRead < ciphertextLength) {
        int bytesToRead = qMin(qint64(sizeof(inBuffer)), ciphertextLength - totalRead);
        bytesRead = inFile.read(reinterpret_cast<char *>(inBuffer), bytesToRead);
        if (bytesRead <= 0)
            break;
        if (!EVP_DecryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead)) {
            emit errorOccurred("Erreur lors du déchiffrement des données");
            EVP_CIPHER_CTX_free(ctx);
            emit setVisible(false);
            return false;
        }
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
        totalRead += bytesRead;
        int progress = static_cast<int>((totalRead * 100) / ciphertextLength);
        emit progressChanged(progress);
    }

    int finalOutLen;
    if (!EVP_DecryptFinal_ex(ctx, outBuffer, &finalOutLen)) {
        emit errorOccurred("Le mot de passe est incorrect ou les données sont corrompues");
        EVP_CIPHER_CTX_free(ctx);
        emit setVisible(false);
        return false;
    }

    // Calculer le hash du fichier déchiffré
    QByteArray decryptedHash = computeFileHash(outputFile);
    if (decryptedHash != storedHash) {  // On compare ensuite ce hash avec le hash original
        emit errorOccurred("Le fichier déchiffré diffère de l'orignal (comparaison des hash)");
        emit setVisible(false);
        return false;
    }

    emit progressChanged(100);

    // Nettoyer
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    emit setVisible(false);

    return true;
}



// FONCTION DE CALCUL DU HASH D'UN FICHIER
QByteArray CryptoModel::computeFileHash(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        return QByteArray(); // Fichier introuvable ou impossible à ouvrir
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while (!file.atEnd()) {
        QByteArray data = file.read(8192);
        hash.addData(data);
    }
    file.close();

    return hash.result();
}



void CryptoModel::setAesKeySize(int keySize) {
    m_aesKeySize = keySize;
}



void CryptoModel::getDeleteSetting(bool choice) {
    deleteOriginal = choice;
}
