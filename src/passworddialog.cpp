#include "headers/passworddialog.h"
#include "ui_passworddialog.h"
#include <QDialog>


PasswordDialog::PasswordDialog(QWidget *parent): QDialog(parent), ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);

    // Connecter la case à cocher au slot pour afficher/masquer le mot de passe
    connect(ui->showPasswordCheckBox, &QCheckBox::toggled, this, &PasswordDialog::on_showPasswordCheckBox_toggled);

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

PasswordDialog::~PasswordDialog()
{
    delete ui;
}
