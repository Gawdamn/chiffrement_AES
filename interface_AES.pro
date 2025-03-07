QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/cryptomodel.cpp \
    src/historymodel.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/optionsdialog.cpp \
    src/passworddialog.cpp \
    src/passwordstrengthmodel.cpp

HEADERS += \
    headers/mainwindow.h \
    headers/passworddialog.h \
    headers/optionsdialog.h \
    headers/cryptomodel.h \
    headers/historymodel.h \
    headers/passwordstrengthmodel.h

FORMS += \
    UI/mainwindow.ui \
    UI/passworddialog.ui \
    UI/optionsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/resources.qrc

LIBS += -lssl -lcrypto
INCLUDEPATH += /usr/include/openssl
QT += concurrent

TARGET = "ChiffrAES"
