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

INCLUDEPATH = widgets canvas annotations

SOURCES += \
        annotationcontainer.cpp \
        annotations/annotationitem.cpp \
        annotations/cubeannotationitem.cpp \
        annotations/rectannotationitem.cpp \
        annotations/segannotationitem.cpp \
        canvas/canvas2d.cpp \
        canvas/canvas3d.cpp \
        canvas/canvasbase.cpp \
        canvas/childcanvas3d.cpp \
        common.cpp \
        filemanager.cpp \
        labelmanager.cpp \
        main.cpp \
        mainwindow.cpp \
        widgets/labeldialog.cpp \
        widgets/labellineedit.cpp \
        widgets/labellistwidget.cpp

HEADERS += \
        annotationcontainer.h \
        annotations/annotationitem.h \
        annotations/cubeannotationitem.h \
        annotations/rectannotationitem.h \
        annotations/segannotationitem.h \
        canvas/canvas2d.h \
        canvas/canvas3d.h \
        canvas/canvasbase.h \
        canvas/childcanvas3d.h \
        common.h \
        filemanager.h \
        labelmanager.h \
        mainwindow.h \
        widgets/labeldialog.h \
        widgets/labellineedit.h \
        widgets/labellistwidget.h

FORMS += \
        mainwindow.ui \
        widgets/labeldialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qdarkstyle/style.qrc \
    resource.qrc
