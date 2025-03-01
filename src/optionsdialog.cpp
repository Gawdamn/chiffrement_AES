#include "headers/optionsdialog.h"
#include "ui_optionsdialog.h"
#include <QSettings>

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    // Charger la préférence du mode sombre à partir de QSettings
    QSettings settings("PFE", "chiffrementAES");
    bool darkModeEnabled = settings.value("darkMode", false).toBool();  // false par défaut
    ui->darkModeCheckBox->setChecked(darkModeEnabled);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_buttonBox_accepted()
{
    // Récupérer l'état de la checkbox dark mode
    bool darkModeEnabled = ui->darkModeCheckBox->isChecked();

    // Enregistrer cette préférence dans QSettings
    QSettings settings("PFE", "chiffrementAES");
    settings.setValue("darkMode", darkModeEnabled);

    // Émettre le signal pour notifier MainWindow (si vous utilisez le signal darkModeChanged)
    emit darkModeChanged(darkModeEnabled);
    accept();
}
