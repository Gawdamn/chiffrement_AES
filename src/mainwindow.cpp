#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "headers/passworddialog.h"
#include <QDialog>
#include <openssl/evp.h>
#include <openssl/rand.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // appel à la fonction test d'OpenSSL
    testOpenSSL();
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

        // TEST : Afficher le mot de passe pour voir si la récupération se fait correctement
        // ui->passwordDisplayLabel->setText("Mot de passe saisi : " + password);

        // Appeler la fonction pour chiffrer le fichier
        //encryptFile(fileName, password);

        // Afficher un message de confirmation
        //ui->messageLabel->setText("Fichier chiffré avec succès.");
    }
}


void MainWindow::on_decryptButton_clicked()
{
    // Ouvrir une boîte de dialogue pour sélectionner le fichier
    QString fileName = QFileDialog::getOpenFileName(this, "Sélectionner un fichier à déchiffrer");

    // Vérifier si un fichier a été sélectionné
    if (fileName.isEmpty())
        return;

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

        // Appeler la fonction pour déchiffrer le fichier (à implémenter)
        // decryptFile(fileName, password);

        // Afficher un message de confirmation (temporaire)
        //ui->messageLabel->setText("Fichier déchiffré avec succès.");
    }
}


void MainWindow::testOpenSSL()  //mini-fonction test pour voir si openSSL fonctionne correctement dans mon projet
{
    unsigned char key[32]; // AES-256 key size

    if (RAND_bytes(key, sizeof(key))) {
        qDebug("Key generated successfully:");  // la sortie se fait au niveaudu terminal d'où le projet à été lancé
                                                // (ne fonctionne pas si on lance le projet depuis l'interface Qt Creator)
        for (size_t i = 0; i < sizeof(key); ++i) {
            qDebug("%02x", key[i]);
        }
    } else {
        qDebug("Failed to generate key.");
    }
}
