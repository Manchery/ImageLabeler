#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labeldialog.h"
#include "canvas2d.h"
#include "canvas3d.h"
#include "utils.h"
#include "rectannotationitem.h"
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QtDebug>
#include <QColorDialog>
#include <QtGlobal>
#include <QKeyEvent>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QComboBox>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    canvas2d = new Canvas2D(&labelManager, &annoContainer, ui->scrollArea);
    canvas2d->setVisible(true); canvas2d->setEnabled(true);
    curCanvas = canvas2d;

    canvas3d = new Canvas3D(&labelManager, &annoContainer, ui->scrollArea);
    canvas3d->setVisible(false); canvas3d->setEnabled(false);

    ui->scrollArea->setAlignment(Qt::AlignCenter);
    ui->scrollArea->setWidget(canvas2d);
//    ui->scrollArea->setWidgetResizable(true);

    connect(canvas2d, &Canvas2D::modeChanged, this, &MainWindow::reportCanvasMode);
    connect(canvas3d, &Canvas3D::modeChanged, this, &MainWindow::reportCanvasMode);

    _setupToolBarAndStatusBar();

    _setupLabelManager();

    _setupAnnotationContainer();

    _setupFileManager();

    connect(canvas2d, &Canvas2D::mouseMoved, this, &MainWindow::reportMouse2dMoved);
    connect(canvas3d, &Canvas3D::focus3dMoved, this, &MainWindow::reportMouse3dMoved);
    connect(canvas3d, &Canvas3D::cursor3dMoved, this, &MainWindow::reportMouse3dMoved);
//    connect(canvas2d, &Canvas2D::zoomRequest, this, &MainWindow::zoomRequest);
    connect(ui->actionFit_Window, &QAction::triggered, this, &MainWindow::adjustFitWindow);
}

void MainWindow::_setupToolBarAndStatusBar()
{
    taskComboBox = new QComboBox(ui->mainToolBar);
    taskComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    taskComboBox->addItem("Detection ");
    taskComboBox->addItem("Segmentation ");
    taskComboBox->addItem("3D Detection ");
    taskComboBox->addItem("3D Segmentation ");
    ui->mainToolBar->insertWidget(ui->actionOpen_File, taskComboBox);

    drawComboBox = new QComboBox(ui->mainToolBar);
    drawComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    drawComboBox->addItem("Rectangle");
    ui->mainToolBar->insertWidget(ui->actionOpen_File, drawComboBox);

    penWidthBox = new QSpinBox(ui->mainToolBar);
    penWidthBox->setRange(1,100);
    penWidthBox->setSingleStep(1);
    penWidthBox->setValue(1);
    penWidthBox->setWrapping(false);
    penWidthBox->setEnabled(false); // because the defalut mode is detection
    ui->mainToolBar->insertWidget(ui->actionOpen_File, penWidthBox);

    canvas2d->changeTask(DETECTION);

    connect(taskComboBox, &QComboBox::currentTextChanged, this, &MainWindow::taskModeChanged);
    connect(drawComboBox, &QComboBox::currentTextChanged, this, &MainWindow::drawModeChanged);
    connect(penWidthBox, SIGNAL(valueChanged(int)), curCanvas, SLOT(setPenWidth(int)));

    unableFileActions();

    mousePosLabel = new QLabel();
    ui->statusBar->addPermanentWidget(mousePosLabel);
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

    // label changed -> canvas repaint (TODO: 2d or 3d /FINISHED?
    connect(&labelManager, &LabelManager::configChanged, [this](){
        if (curCanvas==canvas3d){
            if (canvas3d->getTaskMode() == DETECTION3D){
                canvas3d->updateChildren();
            }else{
                canvas3d->repaintSegAnnotation();
            }
        }else {
            canvas2d->update();
        }
    });

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
    connect(ui->lineEdit_addLabel, &QLineEdit::returnPressed, [this](){
        newLabelRequest(ui->lineEdit_addLabel->text());
        ui->lineEdit_addLabel->setText("");
    });
}

void MainWindow::_setupAnnotationContainer()
{
    // signal-slot from-to: ui->action/canvas => annotations => canvas
    ui->annoListWidget->setSortingEnabled(false);
    ui->annoListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // select a anno to edit it
    connect(ui->annoListWidget, &QListWidget::itemSelectionChanged, [this](){
        auto items = ui->annoListWidget->selectedItems();
        if (items.length()==0){
            annoContainer.setSelected(-1);
            curCanvas->changeCanvasMode(DRAW);
        }else{
            annoContainer.setSelected(ui->annoListWidget->row(items[0]));
            curCanvas->changeCanvasMode(SELECT);
        }
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
    connect(canvas3d, &Canvas3D::newStrokes3DAnnotated, this, &MainWindow::getNewStrokes3D);

    connect(canvas3d, &Canvas3D::newCubeAnnotated, this, &MainWindow::getNewCube);

    // request from canvas only for bbox, not segmentation
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

    // anno changed: canvas repaint (TODO: 2d or 3d /FINISHED?
    connect(&annoContainer, &AnnotationContainer::dataChanged, [this](){
        if (curCanvas==canvas3d){
            if (canvas3d->getTaskMode() == DETECTION3D){
                canvas3d->updateChildren();
            }else{
                canvas3d->repaintSegAnnotation();
            }
        }else {
            canvas2d->update();
        }
    });

    // anno changed: ui list changed
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
}

void MainWindow::_setupFileManager()
{
    ui->fileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // set change not saved, to warning when close
    connect(&labelManager, &LabelManager::configChanged, &fileManager, &FileManager::setChangeNotSaved);
    connect(&annoContainer, &AnnotationContainer::dataChanged, &fileManager, &FileManager::setChangeNotSaved);

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
    connect(ui->fileListWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        int idx = ui->fileListWidget->row(item);
        if (fileManager.getMode()==MultiImage){
            if (!switchFile(idx)){
                ui->fileListWidget->item(fileManager.getCurIdx())->setSelected(true); // redundent?
            }
        }else if (curCanvas==canvas3d){
            switchFile(idx);
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

    if (text.startsWith("3D")){
        ui->actionOpen_File->setEnabled(false);
        curCanvas = canvas3d;
        canvas2d->setVisible(false); canvas2d->setEnabled(false);
        canvas3d->setVisible(true); canvas3d->setEnabled(true);
    }else{
        ui->actionOpen_File->setEnabled(true);
        curCanvas = canvas2d;
        canvas2d->setVisible(true); canvas2d->setEnabled(true);
        canvas3d->setVisible(false); canvas3d->setEnabled(false);
    }
    ui->scrollArea->takeWidget();
    ui->scrollArea->setWidget(curCanvas);

    if (text == "Detection ") canvas2d->changeTask(DETECTION);
    if (text == "Segmentation ") canvas2d->changeTask(SEGMENTATION);
    if (text == "3D Detection ") canvas3d->changeTask(DETECTION3D);
    if (text == "3D Segmentation ") canvas3d->changeTask(SEGMENTATION3D);

    if (text == "Detection " || text == "3D Detection "){
        drawComboBox->clear();
        drawComboBox->addItem("Rectangle");
        ui->annoListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        penWidthBox->setEnabled(false);
    }else if (text == "Segmentation " || text == "3D Segmentation "){
        drawComboBox->clear();
        drawComboBox->addItem("Circle Pen");
        drawComboBox->addItem("Square Pen");
        drawComboBox->addItem("Contour");
        drawComboBox->addItem("Polygonal Contour");
        penWidthBox->setEnabled(true);
        //! TODO: add segmentation select mode
        ui->annoListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    }
}

void MainWindow::drawModeChanged()
{
    QString text = drawComboBox->currentText();

    if (text=="Rectangle") curCanvas->changeDrawMode(RECTANGLE);
    else if (text=="Circle Pen") curCanvas->changeDrawMode(CIRCLEPEN);
    else if (text=="Square Pen") curCanvas->changeDrawMode(SQUAREPEN);
    else if (text=="Contour") curCanvas->changeDrawMode(CONTOUR);
    else if (text=="Polygonal Contour") curCanvas->changeDrawMode(POLYGEN);

    if (text=="Rectangle"||text=="Contour"||text=="Polygonal Contour"){
        penWidthBox->setEnabled(false);
        penWidthBox->setValue(1);
    }else if (text=="Circle Pen"||text=="Square Pen"){
        penWidthBox->setEnabled(true);
        penWidthBox->setValue(curCanvas->getLastPenWidth());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


QString MainWindow::_labelRequest()
{
    QString curLabel = getCurrentLabel();
    if (curLabel==""){
        LabelDialog dialog(labelManager, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString newLabel = dialog.getLabel();
            newLabelRequest(newLabel);
            return newLabel; // maybe empty?
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
//    auto item = ui->annoListWidget->item(row);

    QMenu submenu;
    submenu.addAction("Delete");
    QAction* rightClickItem = submenu.exec(globalPos);
    if (rightClickItem){
        if (rightClickItem->text().contains("Delete")){
            annoContainer.remove(row);
        }
    }
}

QString MainWindow::getCurrentLabel()
{
    auto selected = ui->labelListWidget->selectedItems();
    if (selected.length()==1){
        return selected[0]->text();
    }else if (selected.length()==0){
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
    if (event->key()==Qt::Key_Control)
        if (curCanvas==canvas3d){
            canvas3d->keyPressEvent(event);
            return;
        }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Control)
        if (curCanvas==canvas3d){
            canvas3d->keyReleaseEvent(event);
            return;
        }
}

void MainWindow::on_actionOpen_File_triggered()
{
    on_actionClose_triggered();
    if (fileManager.getMode()!=Close) return; // cancel is selected when unsaved

    QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
                                                    "Image Files (*.jpg *.png);;JPEG Files (*.jpg);;PNG Files (*.png)");
    if (!fileName.isNull() && !fileName.isEmpty()){
        canvas2d->loadPixmap(fileName);
        adjustFitWindow();

        labelManager.allClear();
        annoContainer.allClear();

        if (canvas2d->getTaskMode()==DETECTION){
            fileManager.setAll(fileName, "_detect_labels_annotations.json");
        }else if (canvas2d->getTaskMode()==SEGMENTATION){
            fileManager.setAll(fileName, "_segment_labels_annotations.json");
        }

        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();

        enableFileActions();
    }
}

void MainWindow::on_actionOpen_Dir_triggered()
{
    on_actionClose_triggered();
    if (fileManager.getMode()!=Close) return; // cancel is selected when unsaved

    QString dirName = QFileDialog::getExistingDirectory(this, "open a dir", "/");
    if (!dirName.isNull() && !dirName.isEmpty()){
        QDir dir(dirName);
        QStringList images = dir.entryList(QStringList() << "*.jpg" << "*.png", QDir::Files);
        images.sort();
        if (!dirName.endsWith('/')) dirName+="/";
        QStringList tmp;
        for (auto &image: images){
            if (!image.endsWith("_segment_color.png") && !image.endsWith("_segment_labelId.png") &&
                    !image.endsWith("_segment3d_color.png") && !image.endsWith("_segment3d_labelId.png"))
                tmp.push_back(dirName+image);
        }
        images = tmp;

        labelManager.allClear();
        annoContainer.allClear();

        if (taskComboBox->currentText()=="Detection "){
            canvas2d->loadPixmap(images[0]);
            adjustFitWindow();
            fileManager.setAll(images, "_detect_annotations.json");

        }else if (taskComboBox->currentText()=="Segmentation "){
            canvas2d->loadPixmap(images[0]);
            adjustFitWindow();
            fileManager.setAll(images, "_segment_annotations.json");

        }else if (taskComboBox->currentText()=="3D Detection "){
            canvas3d->loadImagesZ(images);
            adjustFitWindow();
            fileManager.setAllDetection3D(images, "detect3d_labels_annotations.json");

        }else if (taskComboBox->currentText()=="3D Segmentation "){
            canvas3d->loadImagesZ(images);
            adjustFitWindow();
            fileManager.setAllDetection3D(images, "segment3d_labels_annotations.json");
        }

        _loadJsonFile(fileManager.getLabelFile());
        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();

        enableFileActions();
    }
}

void MainWindow::on_actionLoad_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
                                                     "Json Files (*.json)");
    _loadJsonFile(fileName);
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

void MainWindow::on_actionPrevious_Image_triggered()
{
    switchFile(fileManager.getCurIdx()-1);
}

void MainWindow::on_actionNext_Image_triggered()
{
    switchFile(fileManager.getCurIdx()+1);
}

void MainWindow::on_actionClose_triggered()
{
    if (fileManager.getMode() == Close) return;

    if (!_checkUnsaved()) return;

    if (curCanvas==canvas2d){
        canvas2d->loadPixmap(QPixmap());
    }else if (curCanvas == canvas3d){
        canvas3d->close();
    }
    labelManager.allClear();
    annoContainer.allClear();
    fileManager.close();
    unableFileActions();
}


void MainWindow::_saveSegmentImageResults(QString oldSuffix)
{
    QImage colorImage = drawColorImage(canvas2d->getPixmap().size(), &annoContainer, &labelManager);
    QString colorImagePath = FileManager::changeFileSuffix(fileManager.getCurrentOutputFile(), oldSuffix, "color.png");
    colorImage.save(colorImagePath);
    QImage labelIdImage = drawLabelIdImage(canvas2d->getPixmap().size(), &annoContainer, &labelManager);
    QString labelIdImagePath = FileManager::changeFileSuffix(fileManager.getCurrentOutputFile(), oldSuffix, "labelId.png");
    labelIdImage.save(labelIdImagePath);
}

void MainWindow::_saveSegment3dImageResults()
{
    for (int i=0;i<fileManager.count();i++){
        QString fileName = fileManager.imageFileNameAt(i);
        bool hasColorContent = false;
        QImage colorImage = drawColorImage3d(i, &hasColorContent, canvas3d->imageZSize(), &annoContainer, &labelManager);
        QString colorImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + "_segment3d_color.png";
        if (hasColorContent) colorImage.save(colorImagePath);
        bool hasLabelContent = false;
        QImage labelIdImage = drawLabelIdImage3d(i, &hasLabelContent, canvas3d->imageZSize(), &annoContainer, &labelManager);
        QString labelIdImagePath = FileManager::getDir(fileName) + FileManager::getName(fileName) + "_segment3d_labelId.png";
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
                _saveSegmentImageResults("labels_annotations.json");
        }else if (fileManager.getMode()==MultiImage){
            QJsonObject labelJson;
            labelJson.insert("labels", labelManager.toJsonArray());
            FileManager::saveJson(labelJson, fileManager.getLabelFile());

            QJsonObject annoJson;
            annoJson.insert("annotations", annoContainer.toJsonArray());
            FileManager::saveJson(annoJson, fileManager.getCurrentOutputFile());

            if (canvas2d->getTaskMode()==SEGMENTATION)
                _saveSegmentImageResults("annotations.json");
        }
    }else if (curCanvas == canvas3d){
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
                                                    "Json Files (*json)");
    if (!fileName.endsWith("json"))
        fileName+=".json";
    QJsonObject json;
    json.insert("labels", labelManager.toJsonArray());
    json.insert("annotations", annoContainer.toJsonArray());
    FileManager::saveJson(json, fileName);
}

void MainWindow::on_actionZoom_in_triggered()
{
    curCanvas->setScale(curCanvas->getScale()*1.1);
}

void MainWindow::on_actionZoom_out_triggered()
{
    curCanvas->setScale(curCanvas->getScale()*0.9);
}

void MainWindow::enableFileActions()
{
    //! TODO: add more action
    ui->actionSave->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionLoad->setEnabled(true);
    ui->actionClose->setEnabled(true);
    taskComboBox->setEnabled(false);
}

void MainWindow::unableFileActions()
{
    ui->actionSave->setEnabled(false);
    ui->actionSave_As->setEnabled(false);
    ui->actionLoad->setEnabled(false);
    ui->actionClose->setEnabled(false);
    taskComboBox->setEnabled(true);
}

qreal MainWindow::scaleFitWindow()
{
    int w1 = ui->scrollArea->width() - 2; // So that no scrollbars are generated.
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

void MainWindow::zoomRequest(qreal delta, QPoint pos)
{
    int oldWidth = canvas2d->width();
    if (delta>0){
        canvas2d->setScale(canvas2d->getScale()*1.1);
    } else {
        canvas2d->setScale(canvas2d->getScale()*0.9);
    }
    int newWidth = canvas2d->width();
    if (newWidth!=oldWidth){
        qreal scale = static_cast<qreal>(newWidth) / oldWidth;
        int xShift = static_cast<int>(round(pos.x() * scale)) - pos.x();
        int yShift = static_cast<int>(round(pos.y() * scale)) - pos.y();
        auto verticalBar = ui->scrollArea->verticalScrollBar();
        verticalBar->setValue(verticalBar->value()+xShift);
        auto horizontalBar = ui->scrollArea->horizontalScrollBar();
        horizontalBar->setValue(horizontalBar->value()+yShift);
    }
}

void MainWindow::reportMouse2dMoved(QPoint pos)
{
    mousePosLabel->setText("("+ QString::number(pos.x())+","+QString::number(pos.y())+")");
    //    ui->statusBar->showMessage("("+ QString::number(pos.x())+","+QString::number(pos.y())+")");
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
        QJsonObject json = FileManager::readJson(fileName);
        labelManager.fromJsonObject(json);
        annoContainer.fromJsonObject(json, curCanvas->getTaskMode());
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

