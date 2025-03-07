#ifndef PASSWORDSTRENGTHMODEL_H
#define PASSWORDSTRENGTHMODEL_H

#include <QObject>
#include <QString>
#include <cmath>

class PasswordStrengthModel : public QObject
{
    Q_OBJECT
public:
    explicit PasswordStrengthModel(QObject *parent = nullptr);

    // Méthode pour évaluer la force du mot de passe
    int evaluateStrength(const QString &password) const;
    QString evaluateStrengthLabel(const QString &password) const;

private:
    int penaltyRepetitions(const QString &password) const;
    int penaltySequential(const QString &password) const;
};

#endif // PASSWORDSTRENGTHMODEL_H
