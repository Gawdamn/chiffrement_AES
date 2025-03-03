#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog();

signals:
    void darkModeChanged(bool enabled);
    void aesKeySizeChanged(int keySize);
    void originalDeletionPreferenceChanged(bool enalbed);
    void historyEnabledChanged(bool enabled);

private slots:
    void on_buttonBox_accepted();   //slot pour traiter la validation

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
