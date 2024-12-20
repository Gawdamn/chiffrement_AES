#include "passworddialog.h"
#include "ui_passworddialog.h"
#include <QDialog>


PasswordDialog::PasswordDialog(QWidget *parent): QDialog(parent), ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);
}

QString PasswordDialog::getPassword() const
{
    return ui->passwordLineEdit->text();
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}
