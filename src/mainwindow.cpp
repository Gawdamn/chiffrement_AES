#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "src/passworddialog.h"
#include <QDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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
        ui->passwordDisplayLabel->setText("Mot de passe saisi : " + password);

        // Effacer l'affichage du mot de passe après l'utilisation
        //ui->passwordDisplayLabel->clear();

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

