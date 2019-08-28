#include "mainwindow.h"
#include <QApplication>
#include <ctime>

int main(int argc, char *argv[])
{
    qsrand(static_cast<uint>(time(nullptr)));

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
