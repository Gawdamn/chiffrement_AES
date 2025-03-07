#include "headers/passwordstrengthmodel.h"
#include <QRegularExpression>

PasswordStrengthModel::PasswordStrengthModel(QObject *parent)
    : QObject(parent)
{
}

int PasswordStrengthModel::evaluateStrength(const QString &password) const
{
    int score = 0;
    int len = password.length();

    // Bonus de longueur avec un calcul d'entropie simple (en bits)
    // On suppose ici que l'alphabet de base est de 94 caractères (ASCII imprimable)
    double entropy = len * log2(94);
    score += (static_cast<int>(entropy))*0.7;

    // Bonus pour la diversité des caractères
    int uniqueUpper = password.count(QRegularExpression("[A-Z]"));
    int uniqueLower = password.count(QRegularExpression("[a-z]"));
    int uniqueDigits = password.count(QRegularExpression("[0-9]"));
    int uniqueSymbols = password.count(QRegularExpression("[^A-Za-z0-9]"));
    score += uniqueUpper + uniqueLower + uniqueDigits + uniqueSymbols;

    // Bonus pour diversité globale (si toutes les conditions sont remplies)
    if (len >= 8 && password.contains(QRegularExpression("[A-Z]"))
        && password.contains(QRegularExpression("[0-9]"))
        && password.contains(QRegularExpression("[^A-Za-z0-9]")))
    {
        score += 20;
    }

    //Pénalités pour motifs répétitifs
    int repPenalty = penaltyRepetitions(password);
    score -= repPenalty;

    // Pénalités pour motifs séquentiels
    int seqPenalty = penaltySequential(password);
    score -= seqPenalty;

    // Limiter le score à 100
    return qMin(score, 100);
}

int PasswordStrengthModel::penaltyRepetitions(const QString &password) const
{
    int penalty = 0;
    int count = 1;
    for (int i = 1; i < password.length(); i++) {
        if (password[i] == password[i - 1]) {
            count++;
        } else {
            if (count > 2) {
                if (penalty < 15) {
                    penalty += (count - 2) * 5;  // 5 points de pénalité par répétition excédentaire, avec une limite de 30 points
                }
            }
            count = 1;
        }
    }
    if (count > 2) {
        if (penalty < 15) {
            penalty += (count - 2) * 5;
        }
    }
    return penalty;
}

int PasswordStrengthModel::penaltySequential(const QString &password) const
{
    int penalty = 0;
    for (int i = 0; i < password.length() - 2; i++) {
        // Vérification pour les séquences numériques
        if (password[i].isDigit() && password[i+1].isDigit() && password[i+2].isDigit()) {
            int a = password[i].digitValue();
            int b = password[i+1].digitValue();
            int c = password[i+2].digitValue();
            // Si c'est une séquence ascendante ou descendante
            if ((b == a + 1 && c == b + 1) || (b == a - 1 && c == b - 1))
                penalty += 10;
        }
        // Vérification pour les séquences alphabétiques
        if (password[i].isLetter() && password[i+1].isLetter() && password[i+2].isLetter()) {
            char a = password[i].toLower().toLatin1();
            char b = password[i+1].toLower().toLatin1();
            char c = password[i+2].toLower().toLatin1();
            if ((b == a + 1 && c == b + 1) || (b == a - 1 && c == b - 1))
                penalty += 10;
        }
    }
    return penalty;
}


QString PasswordStrengthModel::evaluateStrengthLabel(const QString &password) const
{
    int score = evaluateStrength(password);
    if (score < 40)
        return "Faible";
    else if (score < 70)
        return "Moyen";
    else
        return "Fort";
}
