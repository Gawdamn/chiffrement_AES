#include "headers/passworddialog.h"
#include "ui_passworddialog.h"
#include "headers/passwordstrengthmodel.h"


PasswordDialog::PasswordDialog(QWidget *parent): QDialog(parent), ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);

    // Connecter la case à cocher au slot pour afficher/masquer le mot de passe
    connect(ui->showPasswordCheckBox, &QCheckBox::toggled, this, &PasswordDialog::on_showPasswordCheckBox_toggled);

    // Connecter la modification du champ mot de passe à l'évaluation de sa force
    connect(ui->passwordLineEdit, &QLineEdit::textChanged, this, &PasswordDialog::updatePasswordStrength);

}


void PasswordDialog::setEvaluateStrength(bool evaluate)
{
    m_evaluateStrength = evaluate;
    // Masquer l'indicateur de force si l'évaluation est désactivée
    if (!m_evaluateStrength) {
        ui->strengthProgressBar->setVisible(false);
        ui->strengthLabel->clear();
    }
}


QString PasswordDialog::getPassword() const
{
    return ui->passwordLineEdit->text();
}


void PasswordDialog::on_showPasswordCheckBox_toggled(bool checked)
{
    if (checked) {
        ui->passwordLineEdit->setEchoMode(QLineEdit::Normal);   // Afficher le mot de passe
    } else {
        ui->passwordLineEdit->setEchoMode(QLineEdit::Password); // Masquer le mot de passe
    }
}


void PasswordDialog::updatePasswordStrength(const QString &password)
{
    if (!m_evaluateStrength) {
        // Si l'évaluation est désactivée, ne rien faire (et effacer l'affichage)
        ui->strengthProgressBar->setValue(0);
        ui->strengthLabel->clear();
        return;
    }
    else {
        int score = m_strengthModel->evaluateStrength(password);
        QString label = m_strengthModel->evaluateStrengthLabel(password);

        // Mettre à jour la barre de progression pour afficher le score
        ui->strengthProgressBar->setValue(score);

        // Afficher la force dans le QLabel
        ui->strengthLabel->setText(QString("Force: %1").arg(label));
    }
}


PasswordDialog::~PasswordDialog()
{
    delete ui;
}
