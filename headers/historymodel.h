#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

class HistoryModel : public QObject
{
    Q_OBJECT
public:
    explicit HistoryModel(QObject *parent = nullptr);

    // Ajoute une nouvelle entrée d'historique
    void addEntry(const QString &operation, const QString &inputFile, const QString &outputFile, const bool status);

    // Retourne l'historique sous forme de QJsonArray
    QJsonArray getHistory() const;

    // Efface l'historique
    void clearHistory();

signals:
    // Signal émis lorsque l'historique est mis à jour, pour rafraîchir la vue
    void historyUpdated();
};

#endif // HISTORYMODEL_H
