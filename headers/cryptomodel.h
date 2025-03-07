#ifndef CRYPTOMODEL_H
#define CRYPTOMODEL_H

#include <QObject>
#include <QString>
#include <QCryptographicHash>

class CryptoModel : public QObject
{
    Q_OBJECT
public:
    explicit CryptoModel(QObject *parent = nullptr);

    // Les méthodes de chiffrement et déchiffrement, qui renvoient true en cas de succès, false sinon.
    bool encryptFile(const QString &inputFile, const QString &outputFile, const QString &password);
    bool decryptFile(const QString &inputFile, const QString &outputFile, const QString &password);
    QByteArray computeFileHash(const QString &filePath);
    void setAesKeySize(int keySize);
    void getDeleteSetting(bool choice);
    int m_aesKeySize; // 128, 192 ou 256 bits
    bool deleteOriginal;

signals:
    // Signal pour indiquer la progression et afficher la barre de progression
    void progressChanged(int value);
    void setVisible(bool appears);
    void errorOccurred(const QString &errorMessage);    // Signal pour afficher les messages d'erreur
};

#endif // CRYPTOMODEL_H
