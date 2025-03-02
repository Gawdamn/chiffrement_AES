#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/passworddialog.h"
#include "headers/optionsdialog.h"



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Récupérer l'ancien widget central
    QWidget *oldCentral = this->centralWidget();

    // Créer le QTabWidget
    QTabWidget *tabWidget = new QTabWidget(this);

    // Ajouter l'ancien widget central comme premier onglet
    tabWidget->addTab(oldCentral, "Interface principale");

    // Créer le widget pour l'onglet Historique
    QWidget *historyWidget = new QWidget(this);
    QVBoxLayout *historyLayout = new QVBoxLayout(historyWidget);

    // Créer et configurer le QTableWidget pour l'historique
    m_historyTableWidget = new QTableWidget(historyWidget);
    m_historyTableWidget->setColumnCount(5);
    QStringList headers;
    headers << "Date" << "Opération" << "Fichier source" << "Fichier résultat" << "Statut";
    m_historyTableWidget->setHorizontalHeaderLabels(headers);

    // Ajouter le QTableWidget au layout
    historyLayout->addWidget(m_historyTableWidget);

    // Créer le bouton Clear History
    QPushButton *clearHistoryButton = new QPushButton("Clear History", historyWidget);
    historyLayout->addWidget(clearHistoryButton);

    // Connecter le signal du bouton à un slot clearHistory() dans MainWindow
    connect(clearHistoryButton, &QPushButton::clicked, this, &MainWindow::clearHistory);

    // Définir le layout et ajouter l'onglet Historique au QTabWidget
    historyWidget->setLayout(historyLayout);
    tabWidget->addTab(historyWidget, "Historique");

    // Définir le QTabWidget comme nouveau widget central
    this->setCentralWidget(tabWidget);

    loadHistory();

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);  // Masquée par défaut
    statusBar()->addPermanentWidget(m_progressBar);

    // Connecter le signal de progression à la barre de progression (connexion en mode Qt::QueuedConnection)
    connect(this, &MainWindow::progressChanged, m_progressBar, &QProgressBar::setValue);

    statusBar()->showMessage("Prêt");

    // Charger et appliquer les options enregistrées
    QSettings settings("PFE", "chiffrementAES");
    bool darkModeEnabled = settings.value("darkMode", false).toBool(); //mode clair par défaut
    if (darkModeEnabled) {
        qApp->setStyleSheet(
            "QWidget { background-color: #121212; color: #ffffff; }"
            "QPushButton { background-color: #1e1e1e; border: 1px solid #3a3a3a; padding: 5px; }"
            "QLineEdit { background-color: #1e1e1e; color: #ffffff; border: 1px solid #3a3a3a; }"
            "QLabel { color: #ffffff; }"
            "QCheckBox { color: #ffffff; }"
            );
    }
    m_aesKeySize = settings.value("aesKeySize", 256).toInt(); // clé 256 bits par défaut
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButton_clicked()
{
    QApplication::quit();
}



void MainWindow::on_actionA_propos_triggered()
{
    QMessageBox::about(this, "À propos de l'application",
                        "Application de chiffrement AES de fichier\nVersion 1.0\n\nDéveloppé dans le cadre de mon projet de fin d'études.\n© 2025 Nathan Lestrade");
}



void MainWindow::on_actionOptions_triggered()
{
    OptionsDialog optionsDialog(this);

    // Connecter les signaux
    connect(&optionsDialog, &OptionsDialog::darkModeChanged, this, [this](bool enabled) {
        if(enabled)
            qApp->setStyleSheet("QWidget { background-color: #121212; color: #ffffff; }"
                                "QPushButton { background-color: #1e1e1e; border: 1px solid #3a3a3a; padding: 5px; }"
                                "QLineEdit { background-color: #1e1e1e; color: #ffffff; border: 1px solid #3a3a3a; }"
                                "QLabel { color: #ffffff; }"
                                "QCheckBox { color: #ffffff; }");
        else
            qApp->setStyleSheet("");
    });
    connect(&optionsDialog, &OptionsDialog::aesKeySizeChanged, this, [this](int keySize) {
        m_aesKeySize = keySize;
    });
    connect(&optionsDialog, &OptionsDialog::originalDeletionPreferenceChanged, this, [this](bool enabled) {
        m_deleteOriginal = enabled;
    });
    optionsDialog.exec(); // Affiche la pop-up modale
}



void MainWindow::on_encryptButton_clicked()
{
    // Sélectionner le fichier à chiffrer (inputFile) et obtenir le mot de passe via la pop-up
    QString inputFile = QFileDialog::getOpenFileName(this, "Sélectionner un fichier à chiffrer");
    if (inputFile.isEmpty())
        return;

    // Calculer et stocker le hash du fichier original
    m_originalHash = computeFileHash(inputFile);

    // Affichage du fichier sélectionné sur l'interface principale
    statusBar()->showMessage("Fichier sélectionné : " + inputFile);

    // Ouvrir la fenêtre pour saisir le mot de passe
    PasswordDialog passwordDialog(this);
    if (passwordDialog.exec() != QDialog::Accepted)
        return;
    QString password = passwordDialog.getPassword();
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un mot de passe.");
        return;
    }

    // Ouvrir la fenêtre pour choisir le nom et l'emplacement du nouveau fichier chiffré
    QString outputFile = QFileDialog::getSaveFileName(this, "Enregistrer le fichier chiffré");
    if (outputFile.isEmpty())
        return;

    // À ce stade, la boîte de dialogue de sauvegarde est fermée, l'interface est active.

    // Utiliser QtConcurrent pour lancer l'opération de chiffrement de manière asynchrone
    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, inputFile, outputFile]() {
        bool success = watcher->result(); // Récupère le résultat du chiffrement
        addHistoryEntry("Chiffrement", inputFile, outputFile, success);
        watcher->deleteLater();
        if(success) {
            QMessageBox::information(this, "Succès", "Fichier chiffré avec succès !");
        }
    });

    QFuture<bool> future = QtConcurrent::run(this, &MainWindow::encryptFile, inputFile, outputFile, password);
    watcher->setFuture(future);

    // Charger la préférence de suppression du fichier original
    QSettings settings("PFE", "chiffrementAES");
    bool deleteOriginal = settings.value("deleteOriginal", false).toBool();
    if (deleteOriginal) {
        if (!QFile::remove(inputFile)) {
            qDebug() << "Échec de la suppression du fichier original.";
        }
    }
}



void MainWindow::on_decryptButton_clicked()
{
    // Sélectionner le fichier chiffré
    QString inputFile = QFileDialog::getOpenFileName(this, "Sélectionner un fichier à déchiffrer");

    // Vérifier si un fichier a été sélectionné
    if (inputFile.isEmpty())
        return;

    // Affichage du fichier sélectionné sur l'interface principale
    statusBar()->showMessage("Fichier sélectionné : " + inputFile);

    // Ouvrir la fenêtre pour saisir le mot de passe
    PasswordDialog passwordDialog(this);
    if (passwordDialog.exec() != QDialog::Accepted)
        return;
    QString password = passwordDialog.getPassword();
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez saisir un mot de passe.");
        return;
    }

    // Créer un fichier temporaire pour contenir le déchiffrement
    QTemporaryFile tempFile;
    // On choisit un suffixe pour le fichier temporaire
    tempFile.setFileTemplate(QDir::tempPath() + "/tempDecryptedXXXXXX");
    if (!tempFile.open()) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier temporaire.");
        return;
    }
    QString tempFilePath = tempFile.fileName();
    tempFile.close(); // On va le réutiliser via son nom avec decryptFile

    // Lancer le déchiffrement en tâche de fond (thread séparé)
    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, inputFile, tempFilePath]() {
        bool success = watcher->result();   // Récupérer le résultat du déchiffrement
        watcher->deleteLater();

        if (!success) {
            // En cas d'échec, on supprime le fichier temporaire et on affiche un message
            QFile::remove(tempFilePath);
            statusBar()->showMessage("Déchiffrement échoué ou corrompu.");
            addHistoryEntry("Déchiffrement", inputFile, tempFilePath, success);
            return;
        }

        // Si la décryption a réussi, on demande si l'utilisateur souhaite conserver le fichier chiffré
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this, "Conserver le fichier chiffré ?",
            "Voulez-vous conserver le fichier chiffré une fois le déchiffrement terminé ?",
            QMessageBox::Yes | QMessageBox::No
            );
        if (reply == QMessageBox::No) {
            QFile::remove(inputFile);
        }

        // À ce stade, le fichier temporaire contient le résultat déchiffré.
        // Demander à l'utilisateur de choisir le nom et l'emplacement du fichier déchiffré.
        QString outputFile = QFileDialog::getSaveFileName(this, "Enregistrer le fichier déchiffré");
        if (outputFile.isEmpty()) {
            QFile::remove(tempFilePath);
            return;
        }

        // Vérifier si le fichier de destination existe déjà et le supprimer si c'est le cas
        if (QFile::exists(outputFile)) {
            if (!QFile::remove(outputFile)) {
                QMessageBox::warning(this, "Erreur", "Impossible de supprimer l'ancien fichier de destination.");
                return;
            }
        }

        // Renommer ou copier le fichier temporaire
        if (QFile::rename(tempFilePath, outputFile)) {
            QMessageBox::information(this, "Succès", "Fichier déchiffré avec succès !");
        } else {
            if (QFile::copy(tempFilePath, outputFile)) {
                QFile::remove(tempFilePath);
                QMessageBox::information(this, "Succès", "Fichier déchiffré avec succès !");
            } else {
                QMessageBox::warning(this, "Erreur", "Erreur lors de l'enregistrement du fichier déchiffré.");
            }
        }

        addHistoryEntry("Déchiffrement", inputFile, outputFile, success);
    });

    // Lancer la tâche asynchrone
    QFuture<bool> future = QtConcurrent::run(this, &MainWindow::decryptFile, inputFile, tempFilePath, password);
    watcher->setFuture(future);
}



// FONCTION DE CHIFFREMENT
bool MainWindow::encryptFile(const QString &inputFile, const QString &outputFile, const QString &password)
{
    const int AES_KEY_LENGTH = m_aesKeySize / 8;  // taille de la clé AES utilisée (division par 8 pour la conversion en octets)
    const int AES_BLOCK_SIZE = 16; // Taille d'un bloc AES
    const int GCM_IV_LENGTH = 12;  // Taille de l'IV pour GCM
    const int GCM_TAG_LENGTH = 16; // Taille du tag pour GCM

    // Générer un IV aléatoire pour GCM
    unsigned char iv[GCM_IV_LENGTH];
    if (!RAND_bytes(iv, sizeof(iv))) {
        qDebug() << "Erreur lors de la génération de l'IV";
        return false;
    }


    // Dériver une clé à partir du mot de passe en utilisant PBKDF2
    unsigned char key[AES_KEY_LENGTH];
    unsigned char salt[16];
    if (!RAND_bytes(salt, sizeof(salt))) {
        qDebug() << "Erreur lors de la génération du sel";
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
        qDebug() << "Erreur lors de la dérivation de la clé";
        return false;
    }


    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qDebug() << "Impossible d'ouvrir le fichier d'entrée";
        return false;
    }
    QByteArray originalHash = computeFileHash(inputFile);   // Calcul du hash du fichier original

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qDebug() << "Impossible d'ouvrir le fichier de sortie";
        return false;
    }

    // Écrire le sel, l'IV et le hash original (32 octets)
    outFile.write(reinterpret_cast<const char *>(salt), sizeof(salt));
    outFile.write(reinterpret_cast<const char *>(iv), sizeof(iv));
    outFile.write(originalHash);


    // Initialiser le contexte de chiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug() << "Erreur lors de l'initialisation du contexte de chiffrement";
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
        qDebug() << "Erreur lors de l'initialisation du chiffrement";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Configurer la clé et l'IV
    if (!EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv)) {
        qDebug() << "Erreur lors de la configuration de la clé et de l'IV";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    statusBar()->showMessage("Chiffrement en cours...");

    // Lire et chiffrer les données
    qint64 totalFileSize = inFile.size();
    qint64 totalRead = 0;
    unsigned char inBuffer[4096];
    unsigned char outBuffer[4096 + AES_BLOCK_SIZE];
    int bytesRead;
    int outLen;

    // Dès le début, afficher la barre de progression en 0%
    emit progressChanged(0);
    QMetaObject::invokeMethod(m_progressBar, "setVisible", Qt::QueuedConnection, Q_ARG(bool, true));

    while ((bytesRead = inFile.read(reinterpret_cast<char *>(inBuffer), sizeof(inBuffer))) > 0) {
        if (!EVP_EncryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead)) {
            qDebug() << "Erreur lors de chiffrement";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        totalRead += bytesRead;
        int progress = static_cast<int>((totalRead * 100) / totalFileSize);
        emit progressChanged(progress);     // Mise à jour de la barre de progression
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
    }

    // Finaliser le chiffrement
    if (!EVP_EncryptFinal_ex(ctx, outBuffer, &outLen)) {
        qDebug() << "Erreur lors de la finalisation du chiffrement";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);

    // Obtenir et écrire le tag d'authentification
    unsigned char tag[GCM_TAG_LENGTH];
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag)) {
        qDebug() << "Erreur lors de la génération du tag";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    outFile.write(reinterpret_cast<const char *>(tag), sizeof(tag));

    // Masquer la barre de progression à la fin
    QMetaObject::invokeMethod(m_progressBar, "setVisible", Qt::QueuedConnection, Q_ARG(bool, false));
    emit progressChanged(100);

    // Nettoyer et fermer les fichiers
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    statusBar()->showMessage("Chiffrement terminé.");

    return true;
}



// FONCTION DE DÉCHIFFREMENT
bool MainWindow::decryptFile(const QString &inputFile, const QString &outputFile, const QString &password)
{
    const int AES_KEY_LENGTH = m_aesKeySize / 8;  // taille de la clé AES utilisée
    const int GCM_IV_LENGTH = 12;  // Taille de l'IV pour GCM
    const int GCM_TAG_LENGTH = 16; // Taille du tag pour GCM

    // Ouvrir les fichiers d'entrée et de sortie
    QFile inFile(inputFile);
    QFile outFile(outputFile);

    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qDebug() << "Impossible d'ouvrir le fichier d'entrée";
        return false;
    }

    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qDebug() << "Impossible d'ouvrir le fichier de sortie";
        return false;
    }

    // Lire le sel
    unsigned char salt[16];
    if (inFile.read(reinterpret_cast<char *>(salt), sizeof(salt)) != sizeof(salt)) {
        qDebug() << "Erreur lors de la lecture du sel";
        return false;
    }

    // Lire l'IV
    unsigned char iv[GCM_IV_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(iv), sizeof(iv)) != sizeof(iv)) {
        qDebug() << "Erreur lors de la lecture de l'IV";
        return false;
    }

    // Lire le hash
    QByteArray storedHash = inFile.read(32);
    if (storedHash.size() != 32) {
        qDebug() << "Erreur lors de la lecture du hash original";
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
        qDebug() << "Erreur lors de la dérivation de la clé";
        return false;
    }


    // Initialiser le contexte de déchiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug() << "Erreur lors de l'initialisation du contexte de déchiffrement";
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
        qDebug() << "Erreur lors de la configuration de la clé et de l'IV";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Se déplacer à la position du tag en utilisant la taille actuelle du fichier
    inFile.seek(inFile.size() - GCM_TAG_LENGTH);

    unsigned char tag[GCM_TAG_LENGTH];
    if (inFile.read(reinterpret_cast<char *>(tag), sizeof(tag)) != sizeof(tag)) {
        qDebug() << "Erreur lors de la lecture du tag d'authentification";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Définir le tag pour la vérification
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_LENGTH, tag);


    statusBar()->showMessage("Déchiffrement en cours...");

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
    emit progressChanged(0);
    QMetaObject::invokeMethod(m_progressBar, "setVisible", Qt::QueuedConnection, Q_ARG(bool, true));

    // Revenir au début du ciphertext (après sel et IV)
    inFile.seek(sizeof(salt) + sizeof(iv) + storedHash.size());

    while (totalRead < ciphertextLength) {
        int bytesToRead = qMin(qint64(sizeof(inBuffer)), ciphertextLength - totalRead);
        bytesRead = inFile.read(reinterpret_cast<char *>(inBuffer), bytesToRead);
        if (bytesRead <= 0)
            break;
        if (!EVP_DecryptUpdate(ctx, outBuffer, &outLen, inBuffer, bytesRead)) {
            qDebug() << "Erreur lors du déchiffrement des données";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        outFile.write(reinterpret_cast<const char *>(outBuffer), outLen);
        totalRead += bytesRead;
        int progress = static_cast<int>((totalRead * 100) / ciphertextLength);
        emit progressChanged(progress);
    }

    int finalOutLen;
    if (!EVP_DecryptFinal_ex(ctx, outBuffer, &finalOutLen)) {
        qDebug() << "Le mot de passe est incorrect ou les données sont corrompues";
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Calculer le hash du fichier déchiffré
    QByteArray decryptedHash = computeFileHash(outputFile);
    if (decryptedHash != storedHash) {  // On compare ensuite ce hash avec le hash original
        qDebug() << "Le fichier déchiffré diffère de l'orignal (comparaison des hash)";
        return false;
    }

    QMetaObject::invokeMethod(m_progressBar, "setVisible", Qt::QueuedConnection, Q_ARG(bool, false));
    emit progressChanged(100);

    // Nettoyer
    EVP_CIPHER_CTX_free(ctx);
    inFile.close();
    outFile.close();

    statusBar()->showMessage("Déchiffrement terminé.");

    return true;
}



// FONCTION DE CALCUL DU HASH D'UN FICHIER
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



// FONCTIONS POUR L'HISTORIQUE
void MainWindow::addHistoryEntry(const QString &operation, const QString &inputFile, const QString &outputFile, bool success)
{
    QSettings settings("PFE", "chiffrementAES");
    bool historyEnabled = settings.value("historyEnabled", true).toBool();

    // Si l'historique n'est pas activé, ne rien faire
    if (historyEnabled) {
        // Récupérer l'historique existant sous forme de JSON (en chaîne)
        QString historyJson = settings.value("operationHistory", "[]").toString();
        QJsonDocument doc = QJsonDocument::fromJson(historyJson.toUtf8());
        QJsonArray historyArray = doc.array();

        // Créer une nouvelle entrée
        QJsonObject entry;
        entry["date"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        entry["operation"] = operation; // "Chiffrement" ou "Déchiffrement"
        entry["inputFile"] = inputFile;
        entry["outputFile"] = outputFile;
        entry["status"]     = success ? "Succès" : "Échec";

        // Ajoute l'entrée à l'historique
        historyArray.append(entry);

        // Sauvegarder l'historique mis à jour
        QJsonDocument newDoc(historyArray);
        settings.setValue("operationHistory", newDoc.toJson(QJsonDocument::Compact));
        settings.sync();

        // Charger l'historique dans l'interface
        loadHistory();
    }
}


void MainWindow::loadHistory()
{
    // Charger l'historique depuis QSettings
    QSettings settings("PFE", "chiffrementAES");
    QString historyJson = settings.value("operationHistory", "[]").toString();
    QJsonDocument doc = QJsonDocument::fromJson(historyJson.toUtf8());
    QJsonArray historyArray = doc.array();

    // Préparer le QTableWidget
    m_historyTableWidget->clearContents();
    m_historyTableWidget->setRowCount(historyArray.size());

    // Remplir le tableau avec les entrées d'historique
    for (int i = 0; i < historyArray.size(); ++i) {
        QJsonObject entry = historyArray.at(i).toObject();
        QTableWidgetItem *dateItem = new QTableWidgetItem(entry["date"].toString());
        QTableWidgetItem *opItem = new QTableWidgetItem(entry["operation"].toString());
        QTableWidgetItem *inputItem = new QTableWidgetItem(entry["inputFile"].toString());
        QTableWidgetItem *outputItem = new QTableWidgetItem(entry["outputFile"].toString());
        QTableWidgetItem *statusItem = new QTableWidgetItem(entry["status"].toString());

        m_historyTableWidget->setItem(i, 0, dateItem);
        m_historyTableWidget->setItem(i, 1, opItem);
        m_historyTableWidget->setItem(i, 2, inputItem);
        m_historyTableWidget->setItem(i, 3, outputItem);
        m_historyTableWidget->setItem(i, 4, statusItem);
    }
}


void MainWindow::clearHistory()
{
    // Demander une confirmation (optionnel)
    if (QMessageBox::question(this, "Effacer l'historique",
                              "Voulez-vous vraiment effacer l'historique des opérations ?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    // Effacer l'historique dans QSettings
    QSettings settings("PFE", "chiffrementAES");
    settings.setValue("operationHistory", "[]");
    settings.sync();

    // Effacer l'affichage dans le QTableWidget
    if (m_historyTableWidget) {
        m_historyTableWidget->clearContents();
        m_historyTableWidget->setRowCount(0);
    }
}
