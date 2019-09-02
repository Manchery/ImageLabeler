#include "mainwindow.h"
#include <QApplication>
#include <ctime>
#include <QtDebug>

int main(int argc, char *argv[])
{
    qsrand(static_cast<uint>(time(nullptr)));

    QApplication a(argc, argv);
    MainWindow w;
//    MainWindowTest w;
    w.show();
    try {
        return a.exec();
    } catch (const char *errorStr) {
        qDebug()<<errorStr;
        return -1;
    }
}
