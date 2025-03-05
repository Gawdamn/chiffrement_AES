#include "headers/historymodel.h"
#include <QSettings>
#include <QJsonDocument>

HistoryModel::HistoryModel(QObject *parent) : QObject(parent)
{
}

void HistoryModel::addEntry(const QString &operation, const QString &inputFile, const QString &outputFile, const bool status)
{
    QSettings settings("PFE", "chiffrementAES");
    bool historyEnabled = settings.value("historyEnabled", true).toBool();

    if (historyEnabled) {
        QString historyJson = settings.value("operationHistory", "[]").toString();
        QJsonDocument doc = QJsonDocument::fromJson(historyJson.toUtf8());
        QJsonArray historyArray = doc.array();

        // Créer une nouvelle entrée
        QJsonObject entry;
        entry["date"]       = QDateTime::currentDateTime().toString(Qt::ISODate);
        entry["operation"]  = operation;
        entry["inputFile"]  = inputFile;
        entry["outputFile"] = outputFile;
        entry["status"]     = status ? "Succès" : "Échec";;

        historyArray.append(entry);

        // Sauvegarder l'historique mis à jour
        QJsonDocument newDoc(historyArray);
        settings.setValue("operationHistory", newDoc.toJson(QJsonDocument::Compact));
        settings.sync();

        // Notifier que l'historique a été mis à jour
        emit historyUpdated();
    }
}



QJsonArray HistoryModel::getHistory() const
{
    QSettings settings("PFE", "chiffrementAES");
    QString historyJson = settings.value("operationHistory", "[]").toString();
    QJsonDocument doc = QJsonDocument::fromJson(historyJson.toUtf8());
    return doc.array();
}



void HistoryModel::clearHistory()
{
    // Effacer l'historique dans QSettings
    QSettings settings("PFE", "chiffrementAES");
    settings.setValue("operationHistory", "[]");
    settings.sync();

    emit historyUpdated();  // transmettre le signal pour mettre à jour l'affichage de l'historique
}
