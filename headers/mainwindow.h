#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

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

private slots:
    void on_pushButton_clicked();

    void on_actionA_propos_triggered();

    void on_actionOptions_triggered();

    void on_encryptButton_clicked();

    void on_decryptButton_clicked();

private:
    Ui::MainWindow *ui;
    void encryptFile(const QString &inputFile, const QString &outputFile, const QString &password);
    bool decryptFile(const QString &inputFile, const QString &outputFile, const QString &password);
    QByteArray computeFileHash(const QString &filePath);
    void addHistoryEntry(const QString &operation, const QString &inputFile, const QString &outputFile);
    void loadHistory();
    void clearHistory();
    QByteArray m_originalHash;
    int m_aesKeySize; // 128, 192 ou 256 bits
    bool m_deleteOriginal; // false par défaut
    QTableWidget *m_historyTableWidget;
};
#endif // MAINWINDOW_H
