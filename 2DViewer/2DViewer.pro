QT += core gui serialbus serialbus widgets
requires(qtConfig(combobox))

TARGET = debugCan
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bitratebox.cpp \
    connectdialog.cpp \
    main.cpp \
    sceneactions.cpp \
    sceneasservissement.cpp \
    sendframebox.cpp \
    tabcommandes.cpp \
    twodv.cpp \
    mainwindow.cpp

HEADERS += \
    bitratebox.h \
    connectdialog.h \
    includes.h \
    mainwindow.h \
    sceneactions.h \
    sceneasservissement.h \
    sendframebox.h \
    tabcommandes.h \
    twodv.h

FORMS += \
    connectdialog.ui \
    mainwindow.ui \
    sceneactions.ui \
    sceneasservissement.ui \
    sendframebox.ui \
    tabcommandes.ui \
    twodv.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/can
INSTALLS += target

DISTFILES += \
    can/images/application-exit.png \
    can/images/clear.png \
    can/images/connect.png \
    can/images/disconnect.png
