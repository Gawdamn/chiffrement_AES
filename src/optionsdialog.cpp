#include "headers/optionsdialog.h"
#include "ui_optionsdialog.h"



OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    // Charger les options enregistrées à partir de QSettings
    QSettings settings("PFE", "chiffrementAES");

    // Dark-mode
    bool darkModeEnabled = settings.value("darkMode", false).toBool();  // false par défaut
    ui->darkModeCheckBox->setChecked(darkModeEnabled);
    // Taille de la clé AES utilisée
    int keySize = settings.value("aesKeySize", 256).toInt(); // 256 par défaut
    switch(keySize) {   // Sélectionner l'index correspondant dans le combo box
    case 128: ui->keySizeComboBox->setCurrentIndex(0); break;
    case 192: ui->keySizeComboBox->setCurrentIndex(1); break;
    case 256: ui->keySizeComboBox->setCurrentIndex(2); break;
    default: ui->keySizeComboBox->setCurrentIndex(2); break;
    }
    // Suppression du fichier original
    bool deleteOriginal = settings.value("deleteOriginal", false).toBool(); // false par défaut
    ui->deleteOriginalCheckBox->setChecked(deleteOriginal);
    // Activation de l'historique
    bool historyEnabled = settings.value("historyEnabled", true).toBool();
    ui->enableHistoryCheckBox->setChecked(historyEnabled);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_buttonBox_accepted()
{
    QSettings settings("PFE", "chiffrementAES");
    bool darkModeEnabled = ui->darkModeCheckBox->isChecked();   // Récupérer l'état de la checkbox dark mode
    settings.setValue("darkMode", darkModeEnabled);    // Enregistrer cette préférence dans QSettings
    settings.sync();
    emit darkModeChanged(darkModeEnabled);  // Émettre le signal pour notifier MainWindow


    int index = ui->keySizeComboBox->currentIndex();    // Récupérer la taille de la clé sélectionnée dans le QComboBox
    int keySize = 256; // 256 par défaut
    if (index == 0)
        keySize = 128;
    else if (index == 1)
        keySize = 192;
    else if (index == 2)
        keySize = 256;
    settings.setValue("aesKeySize", keySize);
    settings.sync();
    emit aesKeySizeChanged(keySize);


    bool deleteOriginal = ui->deleteOriginalCheckBox->isChecked();  // Récupérer l'état de la checkbox pour supprimer le fichier original
    settings.setValue("deleteOriginal", deleteOriginal);
    settings.sync();
    emit originalDeletionPreferenceChanged(deleteOriginal);


    bool historyEnabled = ui->enableHistoryCheckBox->isChecked();
    settings.setValue("historyEnabled", historyEnabled);
    settings.sync();
    emit historyEnabledChanged(historyEnabled);

    accept();
}
