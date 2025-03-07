#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "headers/passworddialog.h"
#include "headers/optionsdialog.h"
#include "headers/cryptomodel.h"


MainWindow::MainWindow(QWidget *parent) :       // Constructeur de mainWindow
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_cryptoModel(new CryptoModel(this))
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/images/ICON_AES.png"));

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
    QPushButton *clearHistoryButton = new QPushButton("Supprimer l'historique", historyWidget);
    historyLayout->addWidget(clearHistoryButton);

    // Connecter le signal du bouton à un slot on_clearHistoryButton_clicked() dans MainWindow
    connect(clearHistoryButton, &QPushButton::clicked, this, &MainWindow::on_clearHistoryButton_clicked);

    // Définir le layout et ajouter l'onglet Historique au QTabWidget
    historyWidget->setLayout(historyLayout);
    tabWidget->addTab(historyWidget, "Historique");

    // Définir le QTabWidget comme nouveau widget central
    this->setCentralWidget(tabWidget);

    // Instancier le modèle d'historique
    m_historyModel = new HistoryModel(this);
    // Connecter le signal pour mettre à jour l'affichage de l'historique
    connect(m_historyModel, &HistoryModel::historyUpdated, this, &MainWindow::loadHistory);
    loadHistory();

    // Connecter le signal pour transmettre les messages d'erreur à l'interface principale
    connect(m_cryptoModel, &CryptoModel::errorOccurred, this, &MainWindow::handleCryptoError);

    // Connecter le signal progressChanged pour mettre à jour la barre de progression
    connect(m_cryptoModel, &CryptoModel::progressChanged, this, &MainWindow::updateProgressBar);

    connect(m_cryptoModel, &CryptoModel::setVisible, this, &MainWindow::setVisibleProgressBar);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);  // Masquée par défaut
    statusBar()->addPermanentWidget(m_progressBar);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButton_clicked()
{
    QApplication::quit();
}


void MainWindow::on_clearHistoryButton_clicked()
{
    // Afficher une confirmation dans l'interface utilisateur
    if (QMessageBox::question(this, "Effacer l'historique", "Voulez-vous vraiment effacer l'historique des opérations ?",
                                                                                          QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        m_historyModel->clearHistory();
    }
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
        m_cryptoModel->setAesKeySize(keySize);
    });
    connect(&optionsDialog, &OptionsDialog::originalDeletionPreferenceChanged, this, [this](bool enabled) {
        m_cryptoModel->getDeleteSetting(enabled);
    });
    optionsDialog.exec(); // Affiche la pop-up modale
}



void MainWindow::on_encryptButton_clicked()
{
    // Sélectionner le fichier à chiffrer (inputFile) et obtenir le mot de passe via la pop-up
    QString inputFile = QFileDialog::getOpenFileName(this, "Sélectionner un fichier à chiffrer");
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

    // Ouvrir la fenêtre pour choisir le nom et l'emplacement du nouveau fichier chiffré
    QString outputFile = QFileDialog::getSaveFileName(this, "Enregistrer le fichier chiffré");
    if (outputFile.isEmpty())
        return;

    // À ce stade, la boîte de dialogue de sauvegarde est fermée, l'interface est active.
    statusBar()->showMessage("Chiffrement en cours...");

    // Utiliser QtConcurrent pour lancer l'opération de chiffrement de manière asynchrone
    QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, inputFile, outputFile]() {
        bool success = watcher->result(); // Récupère le résultat du chiffrement
        m_historyModel->addEntry("Chiffrement", inputFile, outputFile, success);
        watcher->deleteLater();
        if(success) {
            statusBar()->showMessage("Chiffrement terminé.");
            QMessageBox::information(this, "Succès", "Fichier chiffré avec succès !");
        }
        else {
            statusBar()->showMessage("Chiffement échoué.");
        }
    });

    QFuture<bool> future = QtConcurrent::run(m_cryptoModel, &CryptoModel::encryptFile, inputFile, outputFile, password);
    watcher->setFuture(future);
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

    statusBar()->showMessage("Déchiffrement en cours...");

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, inputFile, tempFilePath]() {
        bool success = watcher->result();   // Récupérer le résultat du déchiffrement
        watcher->deleteLater();

        if (!success) {
            // En cas d'échec, on supprime le fichier temporaire et on affiche un message
            QFile::remove(tempFilePath);
            statusBar()->showMessage("Déchiffrement échoué");
            m_historyModel->addEntry("Déchiffrement", inputFile, tempFilePath, success);
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
            statusBar()->showMessage("Déchiffrement terminé.");
            QMessageBox::information(this, "Succès", "Fichier déchiffré avec succès !");
        } else {
            if (QFile::copy(tempFilePath, outputFile)) {
                QFile::remove(tempFilePath);
                statusBar()->showMessage("Déchiffrement terminé.");
                QMessageBox::information(this, "Succès", "Fichier déchiffré avec succès !");
            } else {
                QMessageBox::warning(this, "Erreur", "Erreur lors de l'enregistrement du fichier déchiffré.");
            }
        }

        m_historyModel->addEntry("Déchiffrement", inputFile, outputFile, success);
    });

    // Lancer la tâche asynchrone
    QFuture<bool> future = QtConcurrent::run(m_cryptoModel, &CryptoModel::decryptFile, inputFile, tempFilePath, password);
    watcher->setFuture(future);
}



void MainWindow::updateProgressBar(int value)
{
    if (m_progressBar) {
        m_progressBar->setValue(value);
    }
}


void MainWindow::setVisibleProgressBar(bool appears)
{
    m_progressBar->setVisible(appears);
}


// Fonction d'affichage de l'historique
void MainWindow::loadHistory()
{
    // Charger l'historique
    QJsonArray historyArray = m_historyModel->getHistory();

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



void MainWindow::handleCryptoError(const QString &errorMessage)
{
    QMessageBox::critical(this, "Erreur", errorMessage);
}
