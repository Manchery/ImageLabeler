#include "mainwindow.h"
#include <QApplication>
#include <ctime>
#include <QMessageBox>
#include <QString>
#include <QtDebug>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    qsrand(static_cast<uint>(time(nullptr)));

    QApplication a(argc, argv);

//    QFile f(":qdarkstyle/style.qss");
//    if (!f.exists())
//    {
//        printf("Unable to set stylesheet, file not found\n");
//    }
//    else
//    {
//        f.open(QFile::ReadOnly | QFile::Text);
//        QTextStream ts(&f);
//        qApp->setStyleSheet(ts.readAll());
//    }

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
