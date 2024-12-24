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

    void on_encryptButton_clicked();

    void on_decryptButton_clicked();

private:
    Ui::MainWindow *ui;
    //void testOpenSSL(); //fonction test pour OpenSSL
    void encryptFile(const QString &inputFile, const QString &outputFile, const QString &password);
    void decryptFile(const QString &inputFile, const QString &outputFile, const QString &password);

};
#endif // MAINWINDOW_H
