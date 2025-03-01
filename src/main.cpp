#include "headers/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("PFE");
    QCoreApplication::setApplicationName("chiffrementAES");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
