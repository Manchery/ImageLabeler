#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "canvas2d.h"
#include "canvas3d.h"
#include "labeldialog.h"
#include "labelmanager.h"
#include "annotationcontainer.h"
#include "filemanager.h"
#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    qreal scaleFitWindow() const;
    QString getCurrentLabel() const;

    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void reportMouse2dMoved(QPoint pos);
    void reportMouse3dMoved();
    void reportCanvasMode(QString mode);

    void adjustFitWindow();

    void getNewRect(QRect rect);
    void getNewStrokes(const QList<SegStroke> &strokes);
    void getNewCube(Cuboid cube);
    void getNewStrokes3D(const QList<SegStroke3D> &strokes);

    void newLabelRequest(QString newLabel);
    void removeLabelRequest(QString label);
    void provideLabelContextMenu(const QPoint &pos);
    void provideAnnoContextMenu(const QPoint &pos);

    bool switchFile(int idx);

    void canvasUpdate();

private slots:
    void on_actionOpen_File_triggered();
    void on_actionOpen_Dir_triggered();
    void on_actionClose_triggered();

    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionLoad_triggered();

    void on_actionExit_triggered();

    //    void on_actionAbout_triggered();

    void enableFileActions();
    void unableFileActions();

    void taskModeChanged();
    void drawModeChanged();

private:
    Ui::MainWindow *ui;

    CanvasBase *curCanvas; // always equals to canvas2d or canvas3d
    Canvas2D *canvas2d;
    Canvas3D *canvas3d;

    LabelManager labelManager;
    AnnotationContainer annoContainer;
    FileManager fileManager;

    QLabel *mousePosLabel;
    QComboBox *taskComboBox;
    QComboBox *drawComboBox;
    QSpinBox *penWidthBox;

    QList<QAction*> fileRelatedActions;

    void _loadJsonFile(QString fileName);
    bool _checkUnsaved();
    void _saveSegmentImageResults();
    void _saveSegment3dImageResults();

    void _setupToolBarAndStatusBar();
    void _setupLabelManager();
    void _setupAnnotationContainer();
    void _setupFileManager();

    QString _labelRequest();
};

#endif // MAINWINDOW_H
