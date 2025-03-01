#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    void on_darkModeCheckBox_toggled(bool checked);

    void on_encryptButton_clicked();

    void on_decryptButton_clicked();

private:
    Ui::MainWindow *ui;
    void encryptFile(const QString &inputFile, const QString &outputFile, const QString &password);
    bool decryptFile(const QString &inputFile, const QString &outputFile, const QString &password);
    QByteArray computeFileHash(const QString &filePath);
    QByteArray m_originalHash;


};
#endif // MAINWINDOW_H
