#-------------------------------------------------
#
# Project created by QtCreator 2019-08-26T22:17:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageLabeler
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
        canvas.cpp \
        filemanager.cpp \
        labeldialog.cpp \
        labellineedit.cpp \
        labellistwidget.cpp \
        labelmanager.cpp \
        main.cpp \
        mainwindow.cpp \
        rectannotations.cpp \
        utils.cpp

HEADERS += \
        canvas.h \
        filemanager.h \
        labeldialog.h \
        labellineedit.h \
        labellistwidget.h \
        labelmanager.h \
        mainwindow.h \
        rectannotations.h \
        utils.h

FORMS += \
        labeldialog.ui \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
