#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "headers/passworddialog.h"
#include <QDialog>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QCryptographicHash>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // appel à la fonction test d'OpenSSL
    // testOpenSSL();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QApplication::quit();
}


void MainWindow::on_encryptButton_clicked()
{
    // Ouvrir une boîte de dialogue pour sélectionner un fichier
    QString fileName = QFileDialog::getOpenFileName(this, "Sélectionner un fichier à chiffrer");

    // Vérifier si un fichier a été sélectionné
    if (fileName.isEmpty())
        return;

    // Calculer et stocker le hash du fichier original
    m_originalHash = computeFileHash(fileName);
    qDebug() << "Hash original stocké : " << m_originalHash.toHex();

    //affichage du fichier sélectionné sur l'interface principale
    ui->fileLabel->setText("Fichier sélectionné : " + fileName);

    // Ouvrir la fenêtre pour saisir le mot de passe
    PasswordDialog passwordDialog(this);
    if (passwordDialog.exec() == QDialog::Accepted) {
        QString password = passwordDialog.getPassword();

        // Vérifier que le mot de passe n'est pas vide
        if (password.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir un mot de passe.");
            return;
        }

        // Ouvrir la fenêtre pour choisir le nom et l'emplacement du nouveau fichier chiffré
        QString outputFile = QFileDialog::getSaveFileName(this, "Enregistrer le fichier chiffré");
        if (outputFile.isEmpty())
            return;

        // Appeler la fonction pour chiffrer le fichier
        encryptFile(fileName, outputFile, password);
    }
}


void MainWindow::on_decryptButton_clicked()
{
    // Ouvrir une boîte de dialogue pour sélectionner le fichier
    QString inputFile = QFileDialog::getOpenFileName(this, "Sélectionner un fichier à déchiffrer");

    // Vérifier si un fichier a été sélectionné
    if (inputFile.isEmpty())
        return;

    //affichage du fichier sélectionné sur l'interface principale
    ui->fileLabel->setText("Fichier sélectionné : " + inputFile);

    // Ouvrir la fenêtre pour saisir le mot de passe
    PasswordDialog passwordDialog(this);
    if (passwordDialog.exec() == QDialog::Accepted) {
        QString password = passwordDialog.getPassword();

        // Vérifier que le mot de passe n'est pas vide
        if (password.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir un mot de passe.");
            return;
        }

        // Ouvrir la fenêtre pour choisir le nom et l'emplacement du nouveau fichier déchiffré
        QString outputFile = QFileDialog::getSaveFileName(this, "Enregistrer le fichier déchiffré");
        if (outputFile.isEmpty())
            return;

        // Appeler la fonction pour déchiffrer le fichier
        decryptFile(inputFile, outputFile, password);


        // Après déchiffrement, calculer le hash du fichier déchiffré
        QByteArray decryptedHash = computeFileHash(outputFile);
        qDebug() << "Hash du fichier déchiffré : " << decryptedHash.toHex();

        // Comparer les deux hash
        if (decryptedHash == m_originalHash) {
            qDebug() << "Aucune corruption : le fichier déchiffré est identique à l'original.";
        } else {
            qDebug() << "Corruption détectée : le fichier déchiffré diffère de l'original.";
        }
    }
}



/*
void MainWindow::testOpenSSL()  //mini-fonction test pour voir si openSSL fonctionne correctement dans mon projet
{
    unsigned char key[32]; // AES-256 key size

    if (RAND_bytes(key, sizeof(key))) {
        ////QDebug("Key generated successfully:");  // la sortie se fait au niveaudu terminal d'où le projet à été lancé
                                                // (ne fonctionne pas si on lance le projet depuis l'interface Qt Creator)
        for (size_t i = 0; i < sizeof(key); ++i) {
            //QDebug("%02x", key[i]);
        }
    } else {
        //QDebug("Failed to generate key.");
    }
}
*/



// FONCTION DE CHIFFREMENT
void MainWindow::encryptFile(const QString &inputFile, const QString &outputFile, const QString &password)
{
    const int AES_KEY_LENGTH = 32;  // Taille de la clé pour AES-256
    const int AES_BLOCK_SIZE = 16; // Taille d'un bloc AES
    const int GCM_IV_LENGTH = 12;  // Taille de l'IV pour GCM
    const int GCM_TAG_LENGTH = 16; // Taille du tag pour GCM

    // Générer un IV aléatoire pour GCM
    unsigned char iv[GCM_IV_LENGTH];
    if (!RAND_bytes(iv, sizeof(iv))) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la génération de l'IV.");
        return;
    }
    //QDebug() << "IV (chiffrement) : " << QByteArray(reinterpret_cast<const char *>(iv), sizeof(iv)).toHex();


    // Dériver une clé à partir du mot de passe en utilisant PBKDF2
    unsigned char key[AES_KEY_LENGTH];
    unsigned char salt[16];
    if (!RAND_bytes(salt, sizeof(salt))) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la génération du sel.");
        return;
    }
    //QDebug() << "Sel (chiffrement) : " << QByteArray(reinterpret_cast<const char *>(salt), sizeof(salt)).toHex();


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
        QMessageBox::critical(this, "Erreur", "Erreur lors de la dérivation de la clé.");
        return;
    }
    //QDebug() << "Clé (chiffrement) : " << QByteArray(reinterpret_cast<const char *>(key), sizeof(key)).toHex();


    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier d'entrée.");
        return;
    }

    bool isTextFile = inputFile.endsWith(".txt"); // Vérification basique

    if (isTextFile) {
        QTextStream in(&inFile);
        in.setCodec("UTF-8");
    }

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier de sortie.");
        return;
    }

    // Écrire le sel et l'IV dans le fichier de sortie
    outFile.write(reinterpret_cast<const char *>(salt), sizeof(salt));
    outFile.write(reinterpret_cast<const char *>(iv), sizeof(iv));

    // Initialiser le contexte de chiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'initialisation du contexte de chiffrement.");
        return;
    }

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'initialisation du chiffrement.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    // Configurer la clé et l'IV
    if (!EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la configuration de la clé et de l'IV.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    // Lire et chiffrer les données
    unsigned char inBuffer[4096];
    unsigned char outBuffer[4096 + AES_BLOCK_SIZE];
    int bytesRead, bytesWritten;
    int outLen;

    while ((bytesRead = inFile.read(reinterpret_cast<char *>(inBuffer), sizeof(inBuffer))) > 0) {
        if (!EVP_EncryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead)) {
            QMessageBox::critical(this, "Erreur", "Erreur lors du chiffrement.");
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
        qDebug() << "Bloc chiffré (hex) : "
                 << QByteArray(reinterpret_cast<const char *>(outBuffer), outLen).toHex();
        //QDebug() << "Taille des données écrites pour ce bloc (chiffrement) : " << outLen;
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
    }

    // Finaliser le chiffrement
    if (!EVP_EncryptFinal_ex(ctx, outBuffer, &outLen)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la finalisation du chiffrement.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    //QDebug() << "Taille des données écrites lors de la finalisation (chiffrement) : " << outLen;
    outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);

    // Obtenir et écrire le tag d'authentification
    unsigned char tag[GCM_TAG_LENGTH];
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la génération du tag.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    qDebug() << "Position actuelle dans le fichier avant écriture du tag : " << outFile.pos();
    outFile.write(reinterpret_cast<const char *>(tag), sizeof(tag));
    qDebug() << "Position après écriture du tag : " << outFile.pos();

    //QDebug() << "Tag (chiffrement) : " << QByteArray(reinterpret_cast<const char *>(tag), sizeof(tag)).toHex();
    qDebug() << "Tag généré (chiffrement) : " << QByteArray(reinterpret_cast<const char *>(tag), sizeof(tag)).toHex();


    qDebug() << "Taille finale du fichier chiffré (encryptFile) : " << outFile.size();

    // Nettoyer et fermer les fichiers
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    QMessageBox::information(this, "Succès", "Fichier chiffré avec succès !");
}



// FONCTION DE DÉCHIFFREMENT
void MainWindow::decryptFile(const QString &inputFile, const QString &outputFile, const QString &password)
{
    const int AES_KEY_LENGTH = 32;  // Taille de la clé pour AES-256
    const int GCM_IV_LENGTH = 12;  // Taille de l'IV pour GCM
    const int GCM_TAG_LENGTH = 16; // Taille du tag pour GCM

    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier chiffré.");
        return;
    }

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier de sortie.");
        return;
    }


    bool isTextFile = outputFile.endsWith(".txt");

    if (isTextFile) {
        QTextStream out(&outFile);
        out.setCodec("UTF-8");
    }


    // Lire le sel
    unsigned char salt[16];
    if (inFile.read(reinterpret_cast<char *>(salt), sizeof(salt)) != sizeof(salt)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la lecture du sel.");
        return;
    }


    // Lire l'IV
    unsigned char iv[GCM_IV_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(iv), sizeof(iv)) != sizeof(iv)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la lecture de l'IV.");
        return;
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
        QMessageBox::critical(this, "Erreur", "Erreur lors de la dérivation de la clé.");
        return;
    }


    // Initialiser le contexte de déchiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'initialisation du contexte de déchiffrement.");
        return;
    }

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la configuration de la clé et de l'IV.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }


    qDebug() << "Position actuelle dans le fichier avant lecture du tag : " << inFile.pos();
    // Se déplacer à la position du tag en utilisant la taille actuelle du fichier
    inFile.seek(inFile.size() - GCM_TAG_LENGTH);

    unsigned char tag[GCM_TAG_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(tag), sizeof(tag)) != sizeof(tag)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la lecture du tag d'authentification.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }

    // Définir le tag pour la vérification
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_LENGTH, tag);

    qDebug() << "Tag lu (déchiffrement) : " << QByteArray(reinterpret_cast<const char *>(tag), sizeof(tag)).toHex();


    // Calculer la longueur du ciphertext.
    // Le fichier chiffré est structuré comme suit :
    // [salt (16 octets)] [IV (12 octets)] [ciphertext] [tag (16 octets)]
    qint64 totalFileSize = inFile.size();
    const int metadataSize = sizeof(salt) + sizeof(iv) + GCM_TAG_LENGTH;
    qint64 ciphertextLength = totalFileSize - metadataSize;
    qint64 totalRead = 0;
    qDebug() << "Taille totale du fichier chiffré : " << totalFileSize;
    qDebug() << "Longueur effective du ciphertext : " << ciphertextLength;


    unsigned char inBuffer[4096];
    unsigned char outBuffer[4096];
    int bytesRead, outLen;

    // Revenir au début du ciphertext (après sel et IV)
    inFile.seek(sizeof(salt) + sizeof(iv));

    while (totalRead < ciphertextLength) {
        int bytesToRead = qMin(qint64(sizeof(inBuffer)), ciphertextLength - totalRead);
        bytesRead = inFile.read(reinterpret_cast<char *>(inBuffer), bytesToRead);
        if (bytesRead <= 0)
            break;

        if (!EVP_DecryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead)) {
            QMessageBox::critical(this, "Erreur", "Erreur lors du déchiffrement des données.");
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
        totalRead += bytesRead;
        qDebug() << "Déchiffrement en cours, octets traités dans ce bloc : " << outLen;
    }

    qDebug() << "Données chiffrées avant déchiffrement (hex) : "
             << QByteArray(reinterpret_cast<const char *>(inBuffer), bytesRead).toHex();
    qDebug() << "Données déchiffrées après EVP_DecryptUpdate (hex) : "
             << QByteArray(reinterpret_cast<const char *>(outBuffer), outLen).toHex();


    int finalOutLen;
    if (!EVP_DecryptFinal_ex(ctx, outBuffer, &finalOutLen)) {
        qDebug() << "Erreur : EVP_DecryptFinal_ex a échoué. Taille du dernier bloc : " << finalOutLen;
        QMessageBox::critical(this, "Erreur", "Le mot de passe est incorrect ou les données sont corrompues.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    qDebug() << "Dernier bloc après EVP_DecryptFinal_ex : " << finalOutLen;
    outFile.write(reinterpret_cast<const char *>(outBuffer), finalOutLen);

    qDebug() << "Derniers octets après EVP_DecryptFinal_ex (hex) : "
             << QByteArray(reinterpret_cast<const char *>(outBuffer), finalOutLen).toHex();

    qDebug() << "Taille finale du fichier déchiffré (decryptFile) : " << outFile.size();

    // Nettoyer
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    QMessageBox::information(this, "Succès", "Fichier déchiffré avec succès !");
}




QByteArray MainWindow::computeFileHash(const QString &filePath)
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
