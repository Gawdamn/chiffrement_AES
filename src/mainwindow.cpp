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
#include <QTemporaryFile>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    // Sélectionner le fichier chiffré
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


        // Créer un fichier temporaire pour contenir le déchiffrement
        QTemporaryFile tempFile;
        // On choisit un suffixe pour le fichier temporaire (optionnel)
        tempFile.setFileTemplate(QDir::tempPath() + "/tempDecryptedXXXXXX");
        if (!tempFile.open()) {
            QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier temporaire.");
            return;
        }
        QString tempFilePath = tempFile.fileName();
        tempFile.close(); // On va le réutiliser via son nom avec decryptFile


        // Tenter de déchiffrer dans le fichier temporaire
        if (!decryptFile(inputFile, tempFilePath, password)) {
            // En cas d'erreur, ne pas continuer
            QFile::remove(tempFilePath);
            return;
        }


        // Si la décryption a réussi, on demande si l'utilisateur souhaite conserver le fichier chiffré
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Conserver le fichier chiffré ?",
                                      "Voulez-vous conserver le fichier chiffré une fois le déchiffrement terminé ?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            if (QFile::remove(inputFile)) {
                qDebug() << "Fichier chiffré supprimé.";
            } else {
                qDebug() << "Échec de la suppression du fichier chiffré.";
            }
        }

        // À ce stade, le fichier temporaire contient le résultat déchiffré.
        // Demander à l'utilisateur de choisir le nom et l'emplacement du fichier déchiffré.
        QString outputFile = QFileDialog::getSaveFileName(this, "Enregistrer le fichier déchiffré");
        if (outputFile.isEmpty()) {
            // Si l'utilisateur annule, on peut supprimer le fichier temporaire
            QFile::remove(tempFilePath);
            return;
        }

        // Copier (ou renommer) le fichier temporaire vers le fichier de destination
        if (QFile::rename(tempFilePath, outputFile)) {
            QMessageBox::information(this, "Succès", "Fichier déchiffré avec succès !");
        } else {
            // Si renommer échoue, on peut essayer une copie
            if (QFile::copy(tempFilePath, outputFile)) {
                QFile::remove(tempFilePath);
                QMessageBox::information(this, "Succès", "Fichier déchiffré avec succès !");
            } else {
                QMessageBox::warning(this, "Erreur", "Erreur lors de l'enregistrement du fichier déchiffré.");
            }
        }


        // Après déchiffrement, calculer le hash du fichier déchiffré
        QByteArray decryptedHash = computeFileHash(outputFile);

        // Comparer le hash original et le hash du fichier déchiffré
        if (decryptedHash != m_originalHash) {
            QMessageBox::warning(this, "Erreur d'intégrité", "Le fichier déchiffré diffère de l'original.");
        }
    }
}



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


    // Dériver une clé à partir du mot de passe en utilisant PBKDF2
    unsigned char key[AES_KEY_LENGTH];
    unsigned char salt[16];
    if (!RAND_bytes(salt, sizeof(salt))) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la génération du sel.");
        return;
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
        QMessageBox::critical(this, "Erreur", "Erreur lors de la dérivation de la clé.");
        return;
    }


    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier d'entrée.");
        return;
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
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
    }

    // Finaliser le chiffrement
    if (!EVP_EncryptFinal_ex(ctx, outBuffer, &outLen)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la finalisation du chiffrement.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);

    // Obtenir et écrire le tag d'authentification
    unsigned char tag[GCM_TAG_LENGTH];
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la génération du tag.");
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    outFile.write(reinterpret_cast<const char *>(tag), sizeof(tag));

    // Nettoyer et fermer les fichiers
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    QMessageBox::information(this, "Succès", "Fichier chiffré avec succès !");
}



// FONCTION DE DÉCHIFFREMENT
bool MainWindow::decryptFile(const QString &inputFile, const QString &outputFile, const QString &password)
{
    const int AES_KEY_LENGTH = 32;  // Taille de la clé pour AES-256
    const int GCM_IV_LENGTH = 12;  // Taille de l'IV pour GCM
    const int GCM_TAG_LENGTH = 16; // Taille du tag pour GCM

    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier chiffré.");
        return false;
    }

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier de sortie.");
        return false;
    }


    // Lire le sel
    unsigned char salt[16];
    if (inFile.read(reinterpret_cast<char *>(salt), sizeof(salt)) != sizeof(salt)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la lecture du sel.");
        return false;
    }


    // Lire l'IV
    unsigned char iv[GCM_IV_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(iv), sizeof(iv)) != sizeof(iv)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la lecture de l'IV.");
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
        QMessageBox::critical(this, "Erreur", "Erreur lors de la dérivation de la clé.");
        return false;
    }


    // Initialiser le contexte de déchiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'initialisation du contexte de déchiffrement.");
        return false;
    }
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key, iv)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la configuration de la clé et de l'IV.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Se déplacer à la position du tag en utilisant la taille actuelle du fichier
    inFile.seek(inFile.size() - GCM_TAG_LENGTH);

    unsigned char tag[GCM_TAG_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(tag), sizeof(tag)) != sizeof(tag)) {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la lecture du tag d'authentification.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Définir le tag pour la vérification
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_LENGTH, tag);


    // Calculer la longueur du ciphertext.
    // Le fichier chiffré est structuré comme suit :
    // [salt (16 octets)] [IV (12 octets)] [ciphertext] [tag (16 octets)]
    qint64 totalFileSize = inFile.size();
    const int metadataSize = sizeof(salt) + sizeof(iv) + GCM_TAG_LENGTH;
    qint64 ciphertextLength = totalFileSize - metadataSize;
    qint64 totalRead = 0;


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
            return false;
        }
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
        totalRead += bytesRead;
    }


    int finalOutLen;
    if (!EVP_DecryptFinal_ex(ctx, outBuffer, &finalOutLen)) {
        qDebug() << "Erreur : EVP_DecryptFinal_ex a échoué. Taille du dernier bloc : " << finalOutLen;
        QMessageBox::critical(this, "Erreur", "Le mot de passe est incorrect ou les données sont corrompues.");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Nettoyer
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    return true;
}



//FONCTION DE CALCUL DU HASH D'UN FICHIER
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
