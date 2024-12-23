#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit PasswordDialog(QWidget *parent = nullptr);
        ~PasswordDialog();

        QString getPassword() const;

    private:
        Ui::PasswordDialog *ui;

    private slots:
        void on_showPasswordCheckBox_toggled(bool checked);

};

#endif // PASSWORDDIALOG_H
