#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labeldialog.h"
#include "canvas.h"
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
    canvas = new Canvas(&labelManager, &annoContainer, ui->scrollArea);
    ui->scrollArea->setWidget(canvas);
    //    ui->scrollArea->setWidgetResizable(true);
    canvas->setEnabled(true);

    connect(canvas, &Canvas::modeChanged, this, &MainWindow::reportCanvasMode);

    // tool bar and status bar
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
    penWidthBox->setValue(DEFAULT_PEN_WIDTH);
    penWidthBox->setWrapping(false);
    penWidthBox->setEnabled(false); // because the defalut mode is detection
    ui->mainToolBar->insertWidget(ui->actionOpen_File, penWidthBox);

    canvas->changeTask(DETECTION);

    connect(taskComboBox, &QComboBox::currentTextChanged, this, &MainWindow::taskModeChanged);
    connect(drawComboBox, &QComboBox::currentTextChanged, this, &MainWindow::drawModeChanged);
//    connect(penWidthBox, &QSpinBox::valueChanged, canvas, &Canvas::setPenWidth);
    connect(penWidthBox, SIGNAL(valueChanged(int)), canvas, SLOT(setPenWidth(int)));

    unableFileActions();

    mousePosLabel = new QLabel();
    ui->statusBar->addPermanentWidget(mousePosLabel);

    //! end tool bar and status bar


    // label manager
    // signal-slot from-to: ui->list => label manager => canvas
    ui->labelListWidget->setSortingEnabled(true);
    ui->labelListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

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

    connect(&labelManager, &LabelManager::configChanged, canvas, qOverload<>(&QWidget::update));
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

    //! maybe label give back
    connect(&annoContainer, &AnnotationContainer::labelGiveBack, this, &MainWindow::newLabelRequest);

    //! end label manager


    // annotations container
    // signal-slot from-to: ui->action/canvas => annotations => canvas
    ui->annoListWidget->setSortingEnabled(false);
    ui->annoListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    //! select
    connect(ui->annoListWidget, &QListWidget::itemSelectionChanged, [this](){
        auto items = ui->annoListWidget->selectedItems();
        if (items.length()==0){
            annoContainer.setSelected(-1);
        }else{
            annoContainer.setSelected(ui->annoListWidget->row(items[0]));
        }
        canvas->changeCanvasModeRequest();
    });

    //! right click menu
    ui->annoListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->annoListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::provideAnnoContextMenu);

    //! undo & redo
    ui->actionUndo->setEnabled(false);
    connect(&annoContainer, &AnnotationContainer::UndoEnableChanged,
            ui->actionUndo, &QAction::setEnabled);
    connect(ui->actionUndo, &QAction::triggered, &annoContainer, &AnnotationContainer::undo);
    ui->actionRedo->setEnabled(false);
    connect(&annoContainer, &AnnotationContainer::RedoEnableChanged,
            ui->actionRedo, &QAction::setEnabled);
    connect(ui->actionRedo, &QAction::triggered, &annoContainer, &AnnotationContainer::redo);

    //! request from canvas
    connect(canvas, &Canvas::newRectangleAnnotated, this, &MainWindow::getNewRect);
    connect(canvas, &Canvas::newStrokesAnnotated, this, &MainWindow::getNewStrokes);

    ///! only for bbox, not segmentation
    connect(canvas, &Canvas::removeRectRequest, &annoContainer, &AnnotationContainer::remove);
    connect(canvas, &Canvas::modifySelectedRectRequest, [this](int idx, QRect rect){
        std::shared_ptr<RectAnnotationItem> item =
                std::make_shared<RectAnnotationItem>(rect, annoContainer.getSelectedItem()->label,
                                                     annoContainer.getSelectedItem()->id);
        annoContainer.modify(idx, std::static_pointer_cast<AnnotationItem>(item));
    });

    //! to repaint canvas
    connect(&annoContainer, &AnnotationContainer::dataChanged, canvas, qOverload<>(&QWidget::update));

    //! to change ui->annolist
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

    //! end annotations

    // file
    ui->fileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(&labelManager, &LabelManager::configChanged, &fileManager, &FileManager::setChangeNotSaved);
    connect(&annoContainer, &AnnotationContainer::dataChanged, &fileManager, &FileManager::setChangeNotSaved);

    connect(&fileManager, &FileManager::prevEnableChanged, ui->actionPrevious_Image, &QAction::setEnabled);
    connect(&fileManager, &FileManager::nextEnableChanged, ui->actionNext_Image, &QAction::setEnabled);

    connect(&fileManager, &FileManager::fileListSetup, [this](){
        if (fileManager.getMode() == Close) return;
        ui->fileListWidget->clear();
        for (const QString &image: fileManager.allImageFiles()){
            ui->fileListWidget->addItem(image);
        }
        ui->fileListWidget->item(fileManager.getCurIdx())->setSelected(true);
    });
    connect(ui->fileListWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        if (fileManager.getMode()==MultiImage){
            int idx = ui->fileListWidget->row(item);
            if (!switchFile(idx)){
                ui->fileListWidget->item(fileManager.getCurIdx())->setSelected(true);
            }
        }
    });
    // end file

    connect(canvas, &Canvas::mouseMoved, this, &MainWindow::reportMouseMoved);
    connect(canvas, &Canvas::zoomRequest, this, &MainWindow::zoomRequest);
    connect(ui->actionFit_Window, &QAction::triggered, this, &MainWindow::adjustFitWindow);

    // labeldialog
    connect(ui->pushButton_addLabel, &QPushButton::clicked, [this](){
        newLabelRequest(ui->lineEdit_addLabel->text());
        ui->lineEdit_addLabel->setText("");
    });
    connect(ui->lineEdit_addLabel, &QLineEdit::returnPressed, [this](){
        newLabelRequest(ui->lineEdit_addLabel->text());
        ui->lineEdit_addLabel->setText("");
    });
    // end labeldialog
}

void MainWindow::taskModeChanged()
{
    QString text = taskComboBox->currentText();

    if (text == "Detection ") canvas->changeTask(DETECTION);
    if (text == "Segmentation ") canvas->changeTask(SEGMENTATION);
    if (text == "3D Detection ") canvas->changeTask(DETECTION3D);
    if (text == "3D Segmentation ") canvas->changeTask(SEGMENTATION3D);

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
        ui->annoListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    }

    if (text.startsWith("3D")){
        ui->actionOpen_File->setEnabled(false);
    }else{
        ui->actionOpen_File->setEnabled(true);
    }
}

void MainWindow::drawModeChanged()
{
    QString text = drawComboBox->currentText();

    if (text=="Rectangle") canvas->changeDrawMode(RECTANGLE);
    if (text=="Circle Pen") canvas->changeDrawMode(CIRCLEPEN);
    if (text=="Square Pen") canvas->changeDrawMode(SQUAREPEN);
    if (text=="Contour") canvas->changeDrawMode(CONTOUR);
    if (text=="Polygonal Contour") canvas->changeDrawMode(POLYGEN);

    if (text=="Rectangle"||text=="Contour"||text=="Polygonal Contour"){
        penWidthBox->setEnabled(false);
        penWidthBox->setValue(1);
    }else if (text=="Circle Pen"||text=="Square Pen"){
        penWidthBox->setEnabled(true);
        penWidthBox->setValue(canvas->getLastPenWidth());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getNewRect(QRect rect)
{
    QString curLabel = getCurrentLabel();
    if (curLabel==""){
        LabelDialog dialog(labelManager, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString newLabel = dialog.getLabel();
            newLabelRequest(newLabel);
            curLabel = newLabel;
        }else {
            return;
        }
    }

    std::shared_ptr<RectAnnotationItem> item =
            std::make_shared<RectAnnotationItem>(rect, curLabel,
                                                 annoContainer.newInstanceIdForLabel(curLabel));
    annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
}

void MainWindow::getNewStrokes(const QList<SegStroke> &strokes)
{
    QString curLabel = getCurrentLabel();
    if (curLabel==""){
        LabelDialog dialog(labelManager, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString newLabel = dialog.getLabel();
            newLabelRequest(newLabel);
            curLabel = newLabel;
        }else {
            return;
        }
    }
    std::shared_ptr<SegAnnotationItem> item =
            std::make_shared<SegAnnotationItem>(strokes, curLabel,
                                                 annoContainer.newInstanceIdForLabel(curLabel));
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
    if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter)
        if (canvas->getTaskMode()==SEGMENTATION && canvas->getCanvasMode()==DRAW){
            canvas->keyPressEvent(event);
            return;
        }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::on_actionOpen_File_triggered()
{
    on_actionClose_triggered();
    if (fileManager.getMode()!=Close) return; // cancel is selected when unsaved

    QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
                                                    "Image Files (*.jpg *.png);;JPEG Files (*.jpg);;PNG Files (*.png)");
    if (!fileName.isNull() && !fileName.isEmpty()){
        canvas->loadPixmap(fileName);
        adjustFitWindow();

        labelManager.allClear();
        annoContainer.allClear();

        if (canvas->getTaskMode()==DETECTION){
            fileManager.setAll(fileName, "_detect_labels_annotations.json");
        }else if (canvas->getTaskMode()==SEGMENTATION){
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
        if (!dirName.endsWith('/')) dirName+="/";
        QStringList tmp;
        for (auto &image: images){
            if (!image.endsWith("_segment_color.png") && !image.endsWith("_segment_labelId.png"))
                tmp.push_back(dirName+image);
        }
        images = tmp;

        canvas->loadPixmap(images[0]);
        adjustFitWindow();

        labelManager.allClear();
        annoContainer.allClear();

        if (canvas->getTaskMode()==DETECTION){
            fileManager.setAll(images, "_detect_annotations.json");
        }else if (canvas->getTaskMode()==SEGMENTATION){
            fileManager.setAll(images, "_segment_annotations.json");
        }else if (canvas->getTaskMode()==DETECTION3D){
            //! TODO
        }else if (canvas->getTaskMode()==SEGMENTATION3D){
            //! TODO
        }

        _loadJsonFile(fileManager.getLabelFile());
        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();

        enableFileActions();
    }
}

void MainWindow::on_actionLoad_triggered()
{
    if (fileManager.getMode() == SingleImage || fileManager.getMode() == MultiImage){
        QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
                                                         "Json Files (*.json)");
        _loadJsonFile(fileName);
    }
}

bool MainWindow::switchFile(int idx)
{
    if (!_checkUnsaved()) return false;

    //! TODO: whether clear
    labelManager.allClear();
    annoContainer.allClear();

    fileManager.selectFile(idx);
    _loadJsonFile(fileManager.getLabelFile());
    _loadJsonFile(fileManager.getCurrentOutputFile());
    fileManager.resetChangeNotSaved();

    canvas->loadPixmap(fileManager.getCurrentImageFile());
    adjustFitWindow();

    ui->fileListWidget->item(idx)->setSelected(true);
    return true;
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

    if (fileManager.getMode() == SingleImage || fileManager.getMode() == MultiImage){
        canvas->loadPixmap(QPixmap());
        labelManager.allClear();
        annoContainer.allClear();
    }
    fileManager.close();
    unableFileActions();
}


void MainWindow::_saveSegmentImageResults(QString oldSuffix)
{
    QImage colorImage = drawColorImage(canvas->getPixmap().size(), &annoContainer, &labelManager);
    QString colorImagePath = FileManager::changeFileSuffix(fileManager.getCurrentOutputFile(), oldSuffix, "color.png");
    colorImage.save(colorImagePath);
    QImage labelIdImage = drawLabelIdImage(canvas->getPixmap().size(), &annoContainer, &labelManager);
    QString labelIdImagePath = FileManager::changeFileSuffix(fileManager.getCurrentOutputFile(), oldSuffix, "labelId.png");
    labelIdImage.save(labelIdImagePath);
}

void MainWindow::on_actionSave_triggered()
{
    if (canvas->getTaskMode()==DETECTION || canvas->getTaskMode()==SEGMENTATION){
        if (fileManager.getMode()==SingleImage){
            QJsonObject json;
            json.insert("labels", labelManager.toJsonArray());
            json.insert("annotations", annoContainer.toJsonArray());
            FileManager::saveJson(json, fileManager.getCurrentOutputFile());

            if (canvas->getTaskMode()==SEGMENTATION)
                _saveSegmentImageResults("labels_annotations.json");

            fileManager.resetChangeNotSaved();
        }else if (fileManager.getMode()==MultiImage){
            QJsonObject labelJson;
            labelJson.insert("labels", labelManager.toJsonArray());
            FileManager::saveJson(labelJson, fileManager.getLabelFile());

            QJsonObject annoJson;
            annoJson.insert("annotations", annoContainer.toJsonArray());
            FileManager::saveJson(annoJson, fileManager.getCurrentOutputFile());

            if (canvas->getTaskMode()==SEGMENTATION)
                _saveSegmentImageResults("annotations.json");

            fileManager.resetChangeNotSaved();
        }
    }
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
    canvas->setScale(canvas->getScale()*1.1);
}

void MainWindow::on_actionZoom_out_triggered()
{
    canvas->setScale(canvas->getScale()*0.9);
}

qreal MainWindow::scaleFitWindow()
{
    int w1 = ui->scrollArea->width() - 2; // So that no scrollbars are generated.
    int h1 = ui->scrollArea->height() - 2;
    qreal a1 = static_cast<qreal>(w1)/h1;
    int w2 = canvas->getPixmap().width();
    int h2 = canvas->getPixmap().height();
    qreal a2 = static_cast<qreal>(w2)/h2;
    return a2>=a1 ? static_cast<qreal>(w1)/w2 : static_cast<qreal>(h1)/h2;
}

void MainWindow::enableFileActions()
{
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


void MainWindow::adjustFitWindow()
{
    canvas->setScale(scaleFitWindow());
}

void MainWindow::zoomRequest(qreal delta, QPoint pos)
{
    int oldWidth = canvas->width();
    if (delta>0){
        canvas->setScale(canvas->getScale()*1.1);
    } else {
        canvas->setScale(canvas->getScale()*0.9);
    }
    int newWidth = canvas->width();
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

void MainWindow::reportMouseMoved(QPoint pos)
{
    mousePosLabel->setText("("+ QString::number(pos.x())+","+QString::number(pos.y())+")");
//    ui->statusBar->showMessage("("+ QString::number(pos.x())+","+QString::number(pos.y())+")");
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
        annoContainer.fromJsonObject(json, taskComboBox->currentText());
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

