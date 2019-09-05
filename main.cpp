#include "mainwindow.h"
#include <QApplication>
#include <ctime>
#include <QMessageBox>
#include <QString>
#include <QtDebug>

int main(int argc, char *argv[])
{
    qsrand(static_cast<uint>(time(nullptr)));

    QApplication a(argc, argv);
    try {
        MainWindow w;
        w.show();
        return a.exec();
    } catch (const char *errorStr) {
        qDebug()<<errorStr;
        QString msg;
        msg = QString("The program has crashed.\n")
                +"Message: "+errorStr;
        QMessageBox::warning(nullptr, "Program crashed", msg);
    }
    return 0;
}
