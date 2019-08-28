#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "canvas.h"
#include "labeldialog.h"
#include "labelmanager.h"
#include <QMainWindow>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString getCurrentLabel();

public slots:
    void reportMouseMoved(QPoint pos);
    void zoomRequest(qreal delta, QPoint pos);
    void adjustFitWindow();
    void getNewRect(QRect rect);
    void newLabelRequest(QString newLabel);
    void removeLabelRequest(QString label);
    void provideLabelContextMenu(const QPoint &pos);
    void provideAnnoContextMenu(const QPoint &pos);

private slots:
    void on_actionOpen_File_triggered();
    void on_actionZoom_in_triggered();
    void on_actionZoom_out_triggered();
//    void on_actionExit_triggered();
//    void on_actionAbout_triggered();

    qreal scaleFitWindow();

private:
    Ui::MainWindow *ui;
    Canvas *canvas;

    LabelManager labelManager;
    RectAnnotations rectAnno;

};

#endif // MAINWINDOW_H
