#include "mainwindowtest.h"
#include "canvas3d.h"
#include "ui_mainwindowtest.h"

MainWindowTest::MainWindowTest(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowTest)
{
    ui->setupUi(this);
    Canvas3D *canvas = new Canvas3D(nullptr, nullptr);
    ui->scrollArea->setWidget(canvas);

    connect(canvas, &Canvas3D::mouse3DMoved, [this](int x,int y,int z){
       ui->statusbar->showMessage("("+QString::number(x)+","+QString::number(y)+","+QString::number(z)+")");
    });
}

MainWindowTest::~MainWindowTest()
{
    delete ui;
}
