#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labeldialog.h"
#include "canvas2d.h"
#include "canvas3d.h"
#include "common.h"
#include "rectannotationitem.h"

#include <QtDebug>
#include <QtGlobal>

#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QColorDialog>
#include <QKeyEvent>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>

#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    labelManager(this),
    annoContainer(this),
    fileManager(this)
{
    ui->setupUi(this);

    fileRelatedActions << ui->actionSave
                       << ui->actionSave_As
                       << ui->actionClose
                       << ui->actionLoad
                       << ui->actionPrevious_Image
                       << ui->actionNext_Image
                       << ui->actionZoom_in
                       << ui->actionZoom_out
                       << ui->actionFit_Window;

    canvas2d = new Canvas2D(&labelManager, &annoContainer, ui->scrollArea);
    canvas2d->setVisible(true); canvas2d->setEnabled(true);

    canvas3d = new Canvas3D(&labelManager, &annoContainer, ui->scrollArea);
    canvas3d->setVisible(false); canvas3d->setEnabled(false);

    curCanvas = canvas2d;
    ui->scrollArea->setAlignment(Qt::AlignCenter);
    ui->scrollArea->setWidget(curCanvas);

    _setupToolBarAndStatusBar();
    _setupLabelManager();
    _setupAnnotationContainer();
    _setupFileManager();

    canvas2d->changeTask(DETECTION); // DEFAULT TASK
    unableFileActions();
}

void MainWindow::_setupToolBarAndStatusBar()
{
    taskComboBox = new QComboBox(ui->mainToolBar);
    taskComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    taskComboBox->addItem(taskText.at(DETECTION));
    taskComboBox->addItem(taskText.at(SEGMENTATION));
    taskComboBox->addItem(taskText.at(DETECTION3D));
    taskComboBox->addItem(taskText.at(SEGMENTATION3D));
    ui->mainToolBar->insertWidget(ui->actionOpen_File, taskComboBox);

    drawComboBox = new QComboBox(ui->mainToolBar);
    drawComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    drawComboBox->addItem(drawModeText.at(RECTANGLE));
    ui->mainToolBar->insertWidget(ui->actionOpen_File, drawComboBox);

    penWidthBox = new QSpinBox(ui->mainToolBar);
    penWidthBox->setRange(1,100);
    penWidthBox->setSingleStep(1);
    penWidthBox->setValue(1);
    penWidthBox->setWrapping(false);
    penWidthBox->setEnabled(false); // because the defalut mode is detection
    ui->mainToolBar->insertWidget(ui->actionOpen_File, penWidthBox);

//    ui->mainToolBar->setIconSize(QSize(48,48));

    connect(taskComboBox, &QComboBox::currentTextChanged, this, &MainWindow::taskModeChanged);
    connect(drawComboBox, &QComboBox::currentTextChanged, this, &MainWindow::drawModeChanged);
    connect(penWidthBox, SIGNAL(valueChanged(int)), curCanvas, SLOT(setPenWidth(int)));

    connect(canvas2d, &Canvas2D::modeChanged, this, &MainWindow::reportCanvasMode);
    connect(canvas3d, &Canvas3D::modeChanged, this, &MainWindow::reportCanvasMode);

    mousePosLabel = new QLabel();
    ui->statusBar->addPermanentWidget(mousePosLabel);
    connect(canvas2d, &Canvas2D::mouseMoved, this, &MainWindow::reportMouse2dMoved);
    connect(canvas3d, &Canvas3D::focus3dMoved, this, &MainWindow::reportMouse3dMoved);
    connect(canvas3d, &Canvas3D::cursor3dMoved, this, &MainWindow::reportMouse3dMoved);

    connect(ui->actionFit_Window, &QAction::triggered, this, &MainWindow::adjustFitWindow);
    connect(ui->actionZoom_in, &QAction::triggered, [this](){ curCanvas->setScale(curCanvas->getScale()*1.1); });
    connect(ui->actionZoom_out, &QAction::triggered, [this](){ curCanvas->setScale(curCanvas->getScale()*0.9); });
    connect(ui->actionPrevious_Image, &QAction::triggered, [this](){ switchFile(fileManager.getCurIdx()-1); });
    connect(ui->actionNext_Image, &QAction::triggered, [this](){ switchFile(fileManager.getCurIdx()+1); });
}

void MainWindow::_setupLabelManager()
{
    // signal-slot from-to: ui->list => label manager => canvas
    ui->labelListWidget->setSortingEnabled(true);
    ui->labelListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // right click menu for label: change color & delete
    ui->labelListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->labelListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::provideLabelContextMenu);

    connect(ui->labelListWidget, &QListWidget::itemChanged,
            [this](QListWidgetItem *item){ // changeLabelVisble
                if (item->checkState()==Qt::Checked){
                    labelManager.setVisible(item->text(),true);
                }else{
                    labelManager.setVisible(item->text(),false);
                }
            });

    // label changed -> canvas repaint
    connect(&labelManager, &LabelManager::labelChanged, this, &MainWindow::canvasUpdate);

    // label changed -> ui list changed
    connect(&labelManager, &LabelManager::labelAdded,
            ui->labelListWidget, &LabelListWidget::addCustomItem);
    connect(&labelManager, &LabelManager::labelRemoved,
            ui->labelListWidget, &LabelListWidget::removeCustomItem);
    connect(&labelManager, &LabelManager::colorChanged,
            ui->labelListWidget, &LabelListWidget::changeIconColor);
    connect(&labelManager, &LabelManager::visibelChanged,
            ui->labelListWidget, &LabelListWidget::changeCheckState);
    connect(&labelManager, &LabelManager::allCleared,
            ui->labelListWidget, &QListWidget::clear);

    // maybe give back a label when undo delete a anno
    connect(&annoContainer, &AnnotationContainer::labelGiveBack, this, &MainWindow::newLabelRequest);

    // widgets about add label
    connect(ui->pushButton_addLabel, &QPushButton::clicked, [this](){
        newLabelRequest(ui->lineEdit_addLabel->text());
        ui->lineEdit_addLabel->setText("");
    });
    ui->lineEdit_addLabel->installEventFilter(this);
}

void MainWindow::_setupAnnotationContainer()
{
    // signal-slot from-to: ui->action/canvas => annotations => canvas
    ui->annoListWidget->setSortingEnabled(false);
    ui->annoListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // move button only enable when segment and selected
    ui->pushButton_moveUp->setEnabled(false);
    ui->pushButton_moveDown->setEnabled(false);
    connect(ui->pushButton_moveUp, &QPushButton::clicked, [this](){
        int idx = annoContainer.getSelectedIdx();
        if (idx!=-1 && idx!=0){
            annoContainer.swap(idx-1);
        }
    });
    connect(ui->pushButton_moveDown, &QPushButton::clicked, [this](){
        int idx = annoContainer.getSelectedIdx();
        if (idx!=-1 && idx!=annoContainer.length()-1){
            annoContainer.swap(idx);
        }
    });

    // select a anno to edit it
    connect(ui->annoListWidget, &QListWidget::itemSelectionChanged, [this](){
        auto items = ui->annoListWidget->selectedItems();
        if (items.length()==0){
            curCanvas->changeCanvasMode(DRAW);
            annoContainer.setSelected(-1);
            if (curCanvas->getTaskMode() == SEGMENTATION || curCanvas->getTaskMode() == SEGMENTATION3D){
                ui->pushButton_moveUp->setEnabled(false);
                ui->pushButton_moveDown->setEnabled(false);
            }
        }else{
            curCanvas->changeCanvasMode(SELECT);
            annoContainer.setSelected(ui->annoListWidget->row(items[0]));
            if (curCanvas->getTaskMode() == SEGMENTATION || curCanvas->getTaskMode() == SEGMENTATION3D){
                ui->pushButton_moveUp->setEnabled(true);
                ui->pushButton_moveDown->setEnabled(true);
            }
            if (curCanvas->getTaskMode() == SEGMENTATION3D){
                auto item = Seg3DAnnotationItem::castPointer(annoContainer.getSelectedItem());
                int z = item->strokes[0].z;
                switchFile(z);
            }
        }
        // redundent
//        if (curCanvas == canvas3d && canvas3d->getTaskMode() == SEGMENTATION3D)
//            canvas3d->repaintSegAnnotation();
    });
    connect(ui->annoListWidget, &QListWidget::itemClicked, [this](QListWidgetItem *_item){
        if (curCanvas == canvas3d && canvas3d->getTaskMode() == DETECTION3D){
            int row = ui->annoListWidget->row(_item);
            auto item = CubeAnnotationItem::castPointer(annoContainer.at(row));
            canvas3d->setFocusPos(item->cube.center());
            switchFile(item->cube.center().z);
        }
    });

    // right click menu for anno : delete
    ui->annoListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->annoListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::provideAnnoContextMenu);

    // undo & redo
    ui->actionUndo->setEnabled(false);
    connect(&annoContainer, &AnnotationContainer::UndoEnableChanged,
            ui->actionUndo, &QAction::setEnabled); // sync action enable
    connect(ui->actionUndo, &QAction::triggered, &annoContainer, &AnnotationContainer::undo);
    ui->actionRedo->setEnabled(false);
    connect(&annoContainer, &AnnotationContainer::RedoEnableChanged,
            ui->actionRedo, &QAction::setEnabled);
    connect(ui->actionRedo, &QAction::triggered, &annoContainer, &AnnotationContainer::redo);

    // request from canvas
    connect(canvas2d, &Canvas2D::newRectangleAnnotated, this, &MainWindow::getNewRect);
    connect(canvas2d, &Canvas2D::newStrokesAnnotated, this, &MainWindow::getNewStrokes);
    connect(canvas3d, &Canvas3D::newCubeAnnotated, this, &MainWindow::getNewCube);
    connect(canvas3d, &Canvas3D::newStrokes3DAnnotated, this, &MainWindow::getNewStrokes3D);
    // request from canvas, only for bbox, not segmentation
    connect(canvas2d, &Canvas2D::removeRectRequest, &annoContainer, &AnnotationContainer::remove);
    connect(canvas2d, &Canvas2D::modifySelectedRectRequest, [this](int idx, QRect rect){
        std::shared_ptr<RectAnnotationItem> item =
                std::make_shared<RectAnnotationItem>(rect, annoContainer.getSelectedItem()->label,
                                                     annoContainer.getSelectedItem()->id);
        annoContainer.modify(idx, std::static_pointer_cast<AnnotationItem>(item));
    });
    connect(canvas3d, &Canvas3D::removeCubeRequest, &annoContainer, &AnnotationContainer::remove);
    connect(canvas3d, &Canvas3D::modifySelectedCubeRequest, [this](int idx, Cuboid cube){
        std::shared_ptr<CubeAnnotationItem> item =
                std::make_shared<CubeAnnotationItem>(cube, annoContainer.getSelectedItem()->label,
                                                     annoContainer.getSelectedItem()->id);
        annoContainer.modify(idx, std::static_pointer_cast<AnnotationItem>(item));
    });

    // anno changed: canvas repaint
    connect(&annoContainer, &AnnotationContainer::annoChanged, this, &MainWindow::canvasUpdate);
    connect(&annoContainer, &AnnotationContainer::selectedChanged, this, &MainWindow::canvasUpdate);

    // anno changed: ui list change
    connect(&labelManager, &LabelManager::colorChanged, [this](QString label, QColor color){
        for (int i=0;i<ui->annoListWidget->count();i++){
            auto item = ui->annoListWidget->item(i);
            if (item->text().split(' ')[0]==label)
                ui->annoListWidget->changeIconColorByIdx(i, color);
        }
    });
    connect(&annoContainer, &AnnotationContainer::AnnotationAdded,[this](const AnnoItemPtr &item){
        ui->annoListWidget->addCustomItemUncheckable(item->toStr(), labelManager.getColor(item->label));
    });
    connect(&annoContainer, &AnnotationContainer::AnnotationInserted,[this](const AnnoItemPtr &item, int idx){
        ui->annoListWidget->insertCustomItemUncheckable(item->toStr(), labelManager.getColor(item->label),idx);
    });
    connect(&annoContainer, &AnnotationContainer::AnnotationModified,[this](const AnnoItemPtr &item, int idx){
        ui->annoListWidget->changeTextByIdx(idx, item->toStr());
    });
    connect(&annoContainer, &AnnotationContainer::AnnotationRemoved,[this](int idx){
        ui->annoListWidget->removeCustomItemByIdx(idx);
    });
    connect(&annoContainer, &AnnotationContainer::allCleared,
            ui->annoListWidget, &QListWidget::clear);
    connect(&annoContainer, &AnnotationContainer::AnnotationSwap, [this](int idx){
        int selectedIdx = annoContainer.getSelectedIdx();

        auto item = ui->annoListWidget->takeItem(idx);
        ui->annoListWidget->insertItem(idx+1, item);

        if (selectedIdx == idx)
            ui->annoListWidget->item(idx)->setSelected(true);
        else if (selectedIdx == idx+1)
            ui->annoListWidget->item(idx+1)->setSelected(true);
    });
}

void MainWindow::_setupFileManager()
{
    ui->fileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // set change not saved, to warning when close
    connect(&labelManager, &LabelManager::labelChanged, &fileManager, &FileManager::setChangeNotSaved);
    connect(&annoContainer, &AnnotationContainer::annoChanged, &fileManager, &FileManager::setChangeNotSaved);

    // sync prev/next action enable
    connect(&fileManager, &FileManager::prevEnableChanged, ui->actionPrevious_Image, &QAction::setEnabled);
    connect(&fileManager, &FileManager::nextEnableChanged, ui->actionNext_Image, &QAction::setEnabled);

    // to update ui list
    connect(&fileManager, &FileManager::fileListSetup, [this](){
        ui->fileListWidget->clear();
        if (fileManager.getMode() == Close) return;
        for (const QString &image: fileManager.allImageFiles()){
            ui->fileListWidget->addItem(FileManager::getNameWithExtension(image));
        }
        ui->fileListWidget->item(fileManager.getCurIdx())->setSelected(true);
    });

    // click file item to switch image
    connect(ui->fileListWidget, &QListWidget::itemSelectionChanged, [this](){
        auto items = ui->fileListWidget->selectedItems();
        if (items.length()==1){
            int idx = ui->fileListWidget->row(items[0]);
            if (idx == fileManager.getCurIdx()) return;
            if (fileManager.getMode()==MultiImage){
                if (!switchFile(idx)){
                    ui->fileListWidget->item(fileManager.getCurIdx())->setSelected(true); // redundent?
                }
            }else if (curCanvas==canvas3d){
                switchFile(idx);
            }
        }
        // ui list move responding to selected item
        if (ui->fileListWidget->count()<=1) return;
        if (items.length()==1){
            int row = ui->fileListWidget->row(items[0]);
            auto scroll = ui->fileListWidget->verticalScrollBar();
            scroll ->setValue(scroll->minimum()+
                              (scroll->maximum()-scroll->minimum())*row/(ui->fileListWidget->count()-1));
        }
    });

    // ui list responding to focus move
    connect(canvas3d, &Canvas3D::focus3dMoved, [this](Point3D focus){
        fileManager.selectFile(focus.z);
        ui->fileListWidget->item(focus.z)->setSelected(true);
    });
}

void MainWindow::taskModeChanged()
{
    QString text = taskComboBox->currentText();

    if (is3dTask(text)){ // 3D
        ui->actionOpen_File->setEnabled(false);
        curCanvas = canvas3d;
        canvas2d->setVisible(false); canvas2d->setEnabled(false);
        canvas3d->setVisible(true); canvas3d->setEnabled(true);
    }else{ // 2D
        ui->actionOpen_File->setEnabled(true);
        curCanvas = canvas2d;
        canvas2d->setVisible(true); canvas2d->setEnabled(true);
        canvas3d->setVisible(false); canvas3d->setEnabled(false);
    }
    ui->scrollArea->takeWidget();
    ui->scrollArea->setWidget(curCanvas);

    if (text == taskText.at(DETECTION)) canvas2d->changeTask(DETECTION);
    if (text == taskText.at(SEGMENTATION)) canvas2d->changeTask(SEGMENTATION);
    if (text == taskText.at(DETECTION3D)) canvas3d->changeTask(DETECTION3D);
    if (text == taskText.at(SEGMENTATION3D)) canvas3d->changeTask(SEGMENTATION3D);

    if (isDetectTask(text)){
        drawComboBox->clear();
        drawComboBox->addItem(drawModeText.at(RECTANGLE));
        penWidthBox->setEnabled(false);
    }else if (isSegmentTask(text)){
        drawComboBox->clear();
        drawComboBox->addItem(drawModeText.at(CIRCLEPEN));
        drawComboBox->addItem(drawModeText.at(SQUAREPEN));
        drawComboBox->addItem(drawModeText.at(CONTOUR));
        drawComboBox->addItem(drawModeText.at(POLYGEN));
        penWidthBox->setEnabled(true);
    }
}

void MainWindow::drawModeChanged()
{
    QString text = drawComboBox->currentText();
    if (text.isEmpty() || text.isNull()) return;

    curCanvas->changeDrawMode(getDrawModeFromText(text));

    if (text==drawModeText.at(RECTANGLE) || text==drawModeText.at(CONTOUR) ||text==drawModeText.at(POLYGEN)){
        penWidthBox->setEnabled(false);
        penWidthBox->setValue(1);
    }else { // CIRCLE PEN & SQUARE PEN
        penWidthBox->setEnabled(true);
        penWidthBox->setValue(curCanvas->getLastPenWidth());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->lineEdit_addLabel) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                newLabelRequest(ui->lineEdit_addLabel->text());
                ui->lineEdit_addLabel->setText("");
                return true;
            }
        }
    }
    return false;
}

QString MainWindow::_labelRequest()
{
    QString curLabel = getCurrentLabel();
    if (curLabel==""){
        LabelDialog dialog(labelManager, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString newLabel = dialog.getLabel();
            newLabelRequest(newLabel);
            return newLabel;
            // maybe empty(newLabel==""), that also result in no anno added
        }else {
            return "";
        }
    }else{
        return curLabel;
    }
}

void MainWindow::getNewRect(QRect rect)
{
    QString label = _labelRequest();
    if (label=="") return;
    std::shared_ptr<RectAnnotationItem> item =
            std::make_shared<RectAnnotationItem>(rect, label,
                                                 annoContainer.newInstanceIdForLabel(label));
    annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
}

void MainWindow::getNewCube(Cuboid cube)
{
    QString label = _labelRequest();
    if (label=="") return;
    std::shared_ptr<CubeAnnotationItem> item =
            std::make_shared<CubeAnnotationItem>(cube, label,
                                                 annoContainer.newInstanceIdForLabel(label));
    annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
}

void MainWindow::getNewStrokes3D(const QList<SegStroke3D> &strokes)
{
    QString label = _labelRequest();
    if (label=="") return;
    std::shared_ptr<Seg3DAnnotationItem> item =
            std::make_shared<Seg3DAnnotationItem>(strokes, label,
                                                 annoContainer.newInstanceIdForLabel(label));
    annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
}

void MainWindow::getNewStrokes(const QList<SegStroke> &strokes)
{
    QString label = _labelRequest();
    if (label=="") return;
    std::shared_ptr<SegAnnotationItem> item =
            std::make_shared<SegAnnotationItem>(strokes, label,
                                                 annoContainer.newInstanceIdForLabel(label));
    annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
}

void MainWindow::newLabelRequest(QString newLabel)
{
    if (newLabel.isNull() || newLabel.isEmpty()) return;
    if (!labelManager.hasLabel(newLabel)){
        QColor newColor = randomColor();
        labelManager.addLabel(newLabel, newColor, true);
    }
}

void MainWindow::removeLabelRequest(QString label)
{
    if (annoContainer.hasData(label)){
        QMessageBox::warning(this, "Warning", "This label has existing data! Please remove them first.");
    }else{
        labelManager.removeLabel(label);
    }
}

void MainWindow::provideLabelContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->labelListWidget->mapToGlobal(pos);
    QModelIndex modelIdx = ui->labelListWidget->indexAt(pos);
    if (!modelIdx.isValid()) return;
    int row = modelIdx.row();
    auto item = ui->labelListWidget->item(row);

    QMenu submenu;
    submenu.addAction("Change Color");
    submenu.addAction("Delete");
    QAction* rightClickItem = submenu.exec(globalPos);
    if (rightClickItem){
        if (rightClickItem->text().contains("Delete")){
            removeLabelRequest(item->text());
        }else if (rightClickItem->text().contains("Change Color")){
            QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
            if (color.isValid()){
                labelManager.setColor(item->text(),color);
            }
        }
    }
}

void MainWindow::provideAnnoContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->annoListWidget->mapToGlobal(pos);
    QModelIndex modelIdx = ui->annoListWidget->indexAt(pos);
    if (!modelIdx.isValid()) return;
    int row = modelIdx.row();

    QMenu submenu;
    submenu.addAction("Delete");
    QAction* rightClickItem = submenu.exec(globalPos);
    if (rightClickItem){
        if (rightClickItem->text().contains("Delete")){
            annoContainer.remove(row);
        }
    }
}

QString MainWindow::getCurrentLabel() const
{
    auto selectedLabels = ui->labelListWidget->selectedItems();
    if (selectedLabels.length()==1){
        return selectedLabels[0]->text();
    }else if (selectedLabels.length()==0){
        return QString();
    }else {
        throw "selected mutiple label in the list";
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter){
        if (curCanvas==canvas2d && canvas2d->getTaskMode()==SEGMENTATION && canvas2d->getCanvasMode()==DRAW){
            canvas2d->keyPressEvent(event);
            return;
        }
        if (curCanvas==canvas3d && canvas3d->getTaskMode()==SEGMENTATION3D && canvas3d->getCanvasMode()==DRAW){
            canvas3d->keyPressEvent(event);
            return;
        }
    }
    if (event->key()==Qt::Key_Alt)
        if (curCanvas==canvas3d){
            canvas3d->keyPressEvent(event);
            return;
        }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Alt)
        if (curCanvas==canvas3d){
            canvas3d->keyReleaseEvent(event);
            return;
        }
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::on_actionOpen_File_triggered()
{
    on_actionClose_triggered();
    if (fileManager.getMode()!=Close) return; // cancel is selected when unsaved

    QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
                                                    "Image Files (*.jpg *.png);;JPEG Files (*.jpg);;PNG Files (*.png)");
    if (!fileName.isNull() && !fileName.isEmpty()){
        enableFileActions();

        canvas2d->loadPixmap(fileName);
        adjustFitWindow();

        labelManager.allClear();
        annoContainer.allClear();

        if (canvas2d->getTaskMode()==DETECTION){
            fileManager.setSingleImage(fileName, SUFFIX_DET_LABEL_ANNO);
        }else if (canvas2d->getTaskMode()==SEGMENTATION){
            fileManager.setSingleImage(fileName, SUFFIX_SEG_LABEL_ANNO);
        }

        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();
    }
}

void MainWindow::on_actionOpen_Dir_triggered()
{
    on_actionClose_triggered();
    if (fileManager.getMode()!=Close) return; // cancel is selected when unsaved

    QString dirName = QFileDialog::getExistingDirectory(this, "open a dir", "/");
    if (!dirName.isNull() && !dirName.isEmpty()){
        enableFileActions();

        QDir dir(dirName);
        QStringList images = dir.entryList(QStringList() << "*.jpg" << "*.png", QDir::Files);
        images.sort();
        if (!dirName.endsWith('/')) dirName+="/";
        QStringList tmp;
        for (auto &image: images){
            if (!image.endsWith(SUFFIX_SEG_COLOR) && !image.endsWith(SUFFIX_SEG_LABELID) &&
                    !image.endsWith(SUFFIX_SEG3D_COLOR) && !image.endsWith(SUFFIX_SEG3D_LABELID))
                tmp.push_back(dirName+image);
        }
        images = tmp;

        labelManager.allClear();
        annoContainer.allClear();

        if (curCanvas->getTaskMode() == DETECTION){
            canvas2d->loadPixmap(images[0]);
            fileManager.setMultiImage(images, SUFFIX_DET_ANNO);

        }else if (curCanvas->getTaskMode() == SEGMENTATION){
            canvas2d->loadPixmap(images[0]);
            fileManager.setMultiImage(images, SUFFIX_SEG_ANNO);

        }else if (curCanvas->getTaskMode() == DETECTION3D){
            canvas3d->loadImagesZ(images);
            fileManager.set3DImage(images, FILENAME_DET3D_LABEL_ANNO);

        }else if (curCanvas->getTaskMode() == SEGMENTATION3D){
            canvas3d->loadImagesZ(images);
            fileManager.set3DImage(images, FILENAME_SEG3D_LABEL_ANNO);
        }
        adjustFitWindow();

        _loadJsonFile(fileManager.getLabelFile());
        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();
    }
}

void MainWindow::on_actionLoad_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
                                                     "Json Files (*.json)");
    _loadJsonFile(fileName);
}

void MainWindow::on_actionExit_triggered()
{
    if (ui->actionClose->isEnabled()){
        on_actionClose_triggered();
        if (fileManager.getMode()!=Close) return; // cancel is selected when unsaved
    }
    QApplication::quit();
}

bool MainWindow::switchFile(int idx)
{
    if (curCanvas == canvas2d){
        if (!_checkUnsaved()) return false;

        //! ? : whether clear
        labelManager.allClear();
        annoContainer.allClear();

        fileManager.selectFile(idx);
        _loadJsonFile(fileManager.getLabelFile());
        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();

        canvas2d->close();
        canvas2d->loadPixmap(fileManager.getCurrentImageFile());
        adjustFitWindow();

        ui->fileListWidget->item(idx)->setSelected(true);
        return true;
    }else if (curCanvas == canvas3d){
        fileManager.selectFile(idx);
        ui->fileListWidget->item(idx)->setSelected(true);
        Point3D focusPos = canvas3d->getFocusPos();
        canvas3d->setFocusPos(Point3D(focusPos.x, focusPos.y, idx));
        return true;
    }else{
        throw "abnormal curCanvas";
    }
}

void MainWindow::on_actionClose_triggered()
{
    if (fileManager.getMode() == Close) return;

    if (!_checkUnsaved()) return;

    curCanvas->close();
    labelManager.allClear();
    annoContainer.allClear();
    fileManager.close();
    unableFileActions();
}

void MainWindow::_saveSegmentImageResults()
{
    QString fileName = fileManager.getCurrentImageFile();
    QImage colorImage = drawColorImage(canvas2d->getPixmap().size(), &annoContainer, &labelManager);
    QString colorImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + SUFFIX_SEG_COLOR;
    colorImage.save(colorImagePath);
    QImage labelIdImage = drawLabelIdImage(canvas2d->getPixmap().size(), &annoContainer, &labelManager);
    QString labelIdImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + SUFFIX_SEG_LABELID;
    labelIdImage.save(labelIdImagePath);
}

void MainWindow::_saveSegment3dImageResults()
{
    for (int i=0;i<fileManager.count();i++){
        QString fileName = fileManager.imageFileNameAt(i);
        bool hasColorContent = false;
        QImage colorImage = drawColorImage3d(i, &hasColorContent, canvas3d->imageZSize(), &annoContainer, &labelManager);
        QString colorImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + SUFFIX_SEG3D_COLOR;
        if (hasColorContent) colorImage.save(colorImagePath);
        bool hasLabelContent = false;
        QImage labelIdImage = drawLabelIdImage3d(i, &hasLabelContent, canvas3d->imageZSize(), &annoContainer, &labelManager);
        QString labelIdImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + SUFFIX_SEG3D_LABELID;
        if (hasLabelContent) labelIdImage.save(labelIdImagePath);
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (curCanvas == canvas2d){ // 2D mode
        if (fileManager.getMode()==SingleImage){
            QJsonObject json;
            json.insert("labels", labelManager.toJsonArray());
            json.insert("annotations", annoContainer.toJsonArray());
            FileManager::saveJson(json, fileManager.getCurrentOutputFile());

            if (canvas2d->getTaskMode()==SEGMENTATION)
                _saveSegmentImageResults();
        }else if (fileManager.getMode()==MultiImage){
            QJsonObject labelJson;
            labelJson.insert("labels", labelManager.toJsonArray());
            FileManager::saveJson(labelJson, fileManager.getLabelFile());
            QJsonObject annoJson;
            annoJson.insert("annotations", annoContainer.toJsonArray());
            FileManager::saveJson(annoJson, fileManager.getCurrentOutputFile());

            if (canvas2d->getTaskMode()==SEGMENTATION)
                _saveSegmentImageResults();
        }
    }else if (curCanvas == canvas3d){ // 3D mode
        if (canvas3d->getTaskMode() == DETECTION3D){
            QJsonObject json;
            json.insert("labels", labelManager.toJsonArray());
            json.insert("annotations", annoContainer.toJsonArray());
            FileManager::saveJson(json, fileManager.getCurrentOutputFile());
        }else if (canvas3d->getTaskMode() == SEGMENTATION3D){
            QJsonObject json;
            json.insert("labels", labelManager.toJsonArray());
            json.insert("annotations", annoContainer.toJsonArray());
            FileManager::saveJson(json, fileManager.getCurrentOutputFile());

            if (canvas3d->getTaskMode()==SEGMENTATION3D){
                _saveSegment3dImageResults();
            }
        }
    }
    fileManager.resetChangeNotSaved();
}

void MainWindow::on_actionSave_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "open a file", "/",
                                                    "Json Files (*.json)");
    if (!fileName.endsWith("json"))
        fileName+=".json";
    QJsonObject json;
    json.insert("labels", labelManager.toJsonArray());
    json.insert("annotations", annoContainer.toJsonArray());
    FileManager::saveJson(json, fileName);
}

void MainWindow::enableFileActions()
{
    for (auto action: fileRelatedActions)
        action->setEnabled(true);
    taskComboBox->setEnabled(false);
}

void MainWindow::unableFileActions()
{
    for (auto action: fileRelatedActions)
        action->setEnabled(false);
    taskComboBox->setEnabled(true);
}

qreal MainWindow::scaleFitWindow() const
{
    int w1 = ui->scrollArea->width() - 2; // -2 So that no scrollbars are generated.
    int h1 = ui->scrollArea->height() - 2;
    qreal a1 = static_cast<qreal>(w1)/h1;
    int w2 = curCanvas->sizeUnscaled().width();
    int h2 = curCanvas->sizeUnscaled().height();
    qreal a2 = static_cast<qreal>(w2)/h2;
    return a2>=a1 ? static_cast<qreal>(w1)/w2 : static_cast<qreal>(h1)/h2;
}

void MainWindow::adjustFitWindow()
{
    curCanvas->setScale(scaleFitWindow());
}

void MainWindow::reportMouse2dMoved(QPoint pos)
{
    mousePosLabel->setText("("+ QString::number(pos.x())+","+QString::number(pos.y())+")");
}
void MainWindow::reportMouse3dMoved()
{
    QString text;
    Point3D pos = canvas3d->getCursorPos();
    text = "cursor(" + QString::number(pos.x)+","+QString::number(pos.y)+","+QString::number(pos.z)+")";
    pos = canvas3d->getFocusPos();
    text += ", focus(" + QString::number(pos.x)+","+QString::number(pos.y)+","+QString::number(pos.z)+")";
    mousePosLabel->setText(text);
}
void MainWindow::reportCanvasMode(QString mode){
    ui->statusBar->showMessage(mode, 5000);
}

void MainWindow::_loadJsonFile(QString fileName)
{
    QFileInfo checkFile=QFileInfo(fileName);
    if (checkFile.exists() && checkFile.isFile()){
        try {
            QJsonObject json = FileManager::readJson(fileName);
            labelManager.fromJsonObject(json);
            annoContainer.fromJsonObject(json, curCanvas->getTaskMode());
        } catch (FileException &e) {
            QMessageBox::warning(this, "File Error", e.what());
        } catch (JsonException &e) {
            QString msg;
            msg = QString("The saved json file is broken.\n")
                    +"Error message: "+e.what()+"\n"
                    +"Please check or delete the json file.\n";
            QMessageBox::warning(this, "Json Error", msg);
        }
    }
}

// return false to cancel, true to safety to close
bool MainWindow::_checkUnsaved()
{
    if (fileManager.hasChangeNotSaved()){
        if (ui->actionAuto_Save->isChecked())
            on_actionSave_triggered();
        else{
            int ret = QMessageBox::warning(this, QObject::tr("Warning"),
                                           QObject:: tr("The document has been modified.\n"
                                                        "Do you want to save your changes?\n"
                                                        "Note: you can check AutoSave option in the menu.\n"),
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                           QMessageBox::Save);
            switch (ret) {
            case QMessageBox::Save:
                on_actionSave_triggered();
                break;
            case QMessageBox::Discard:
                break;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
            }
        }
    }
    return true;
}

void MainWindow::canvasUpdate()
{
    if (curCanvas==canvas3d){
        if (canvas3d->getTaskMode() == DETECTION3D){
            canvas3d->updateChildren();
        }else{
            canvas3d->repaintSegAnnotation();
        }
    }else {
        canvas2d->update();
    }
}
