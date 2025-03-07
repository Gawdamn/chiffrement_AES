#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include "headers/passwordstrengthmodel.h"

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

        void setEvaluateStrength(bool evaluate);    // Méthode setter pour activer ou non l'évaluation de la force du MDP depuis MainWindow

    private:
        Ui::PasswordDialog *ui;
        PasswordStrengthModel *m_strengthModel;
        bool m_evaluateStrength;  // true pour chiffrement, false pour déchiffrement

    private slots:
        void on_showPasswordCheckBox_toggled(bool checked);
        void updatePasswordStrength(const QString &password);

};

#endif // PASSWORDDIALOG_H
