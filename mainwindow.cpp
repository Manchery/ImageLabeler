#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labeldialog.h"
#include "canvas.h"
#include "utils.h"
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

//    QComboBox *comboBox = new QComboBox(ui->mainToolBar);
//    comboBox->addItem("Detection");
//    comboBox->addItem("Segmentation");
//    ui->mainToolBar->insertWidget(ui->actionOpen_File, comboBox);

    mousePosLabel = new QLabel();
    ui->statusBar->addPermanentWidget(mousePosLabel);

    canvas = new Canvas(&labelManager, &annoContainer, ui->scrollArea);
    ui->scrollArea->setWidget(canvas);
//    ui->scrollArea->setWidgetResizable(true);

    canvas->setEnabled(true);

    connect(canvas, &Canvas::modeChanged, this, &MainWindow::reportCanvasMode);

    // label config
    // signal-slot from-to: ui->list => labelconfig => canvas
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

    //! end label config


    // label data
    // signal-slot from-to: ui->action/canvas => labeldata => canvas
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

    //! end label data

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

MainWindow::~MainWindow()
{
    delete ui;
    delete canvas;
}

void MainWindow::getNewRect(QRect rect)
{
    QString curLabel = getCurrentLabel();
    if (curLabel==""){
        LabelDialog dialog(labelManager, this);
        if(dialog.exec() == QDialog::Accepted) {
            QString newLabel = dialog.getLabel();
            newLabelRequest(newLabel);
            std::shared_ptr<RectAnnotationItem> item =
                    std::make_shared<RectAnnotationItem>(rect, newLabel,
                                                         annoContainer.newInstanceIdForLabel(newLabel));
            annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
        }
    }else{
        std::shared_ptr<RectAnnotationItem> item =
                std::make_shared<RectAnnotationItem>(rect, curLabel,
                                                     annoContainer.newInstanceIdForLabel(curLabel));
        annoContainer.push_back(std::static_pointer_cast<AnnotationItem>(item));
    }
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

        fileManager.setAll(fileName, "json");

        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();

        ui->actionSave->setEnabled(true);
        ui->actionSave_As->setEnabled(true);
        ui->actionLoad->setEnabled(true);
        ui->actionClose->setEnabled(true);
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
        for (auto &image: images)
            image = dirName+image;

        canvas->loadPixmap(images[0]);
        adjustFitWindow();

        labelManager.allClear();
        annoContainer.allClear();

        fileManager.setAll(images, "json");

        _loadJsonFile(fileManager.getLabelFile());
        _loadJsonFile(fileManager.getCurrentOutputFile());
        fileManager.resetChangeNotSaved();

        ui->actionSave->setEnabled(true);
        ui->actionSave_As->setEnabled(true);
        ui->actionLoad->setEnabled(true);
        ui->actionClose->setEnabled(true);
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
    ui->actionSave->setEnabled(false);
    ui->actionSave_As->setEnabled(false);
    ui->actionLoad->setEnabled(false);
    ui->actionClose->setEnabled(false);
}


void MainWindow::on_actionSave_triggered()
{
    if (fileManager.getMode() == SingleImage){
        QJsonObject json;
        json.insert("labels", labelManager.toJsonArray());
        json.insert("annotations", annoContainer.toJsonArray());
        FileManager::saveJson(json, fileManager.getCurrentOutputFile());

        fileManager.resetChangeNotSaved();
    } else if (fileManager.getMode() == MultiImage){
        QJsonObject labelJson;
        labelJson.insert("labels", labelManager.toJsonArray());
        FileManager::saveJson(labelJson, fileManager.getLabelFile());

        QJsonObject annoJson;
        annoJson.insert("annotations", annoContainer.toJsonArray());
        FileManager::saveJson(annoJson, fileManager.getCurrentOutputFile());

        fileManager.resetChangeNotSaved();
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
        annoContainer.fromJsonObject(json, "Rect2D");
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

