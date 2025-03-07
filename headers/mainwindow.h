#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QTemporaryFile>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QProgressBar>
#include <QtConcurrent>
#include <QFutureWatcher>
#include "headers/cryptomodel.h"
#include "headers/historymodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void progressChanged(int value);

private slots:
    void on_pushButton_clicked();

    void on_clearHistoryButton_clicked();

    void on_actionA_propos_triggered();

    void on_actionOptions_triggered();

    void on_actionQuitter_triggered();

    void on_encryptButton_clicked();

    void on_decryptButton_clicked();

    void updateProgressBar(int value);

    void setVisibleProgressBar(bool appears);

    void handleCryptoError(const QString &errorMessage);

private:
    Ui::MainWindow *ui;
    void loadHistory();
    bool m_deleteOriginal; // false par défaut
    QTableWidget *m_historyTableWidget;
    QProgressBar *m_progressBar;
    CryptoModel *m_cryptoModel;
    HistoryModel *m_historyModel;
};
#endif // MAINWINDOW_H
