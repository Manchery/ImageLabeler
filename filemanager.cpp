#include "filemanager.h"
#include <QtDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFile>

QString FileManager::changeExtensionName(QString fileName, QString newExtension){
    QStringList list = fileName.split('.');
    QString oldExtension = list.back();
    QString newFileName = fileName.replace(fileName.length()-oldExtension.length(),
                                           oldExtension.length(), newExtension);
    qDebug()<<newFileName;
    return newFileName;
}

QString FileManager::changeFileSuffix(QString fileName, QString oldSuffix, QString newSuffix)
{
    //! TODO: check oldSuffix exists
    return fileName.replace(fileName.length()-oldSuffix.length(), oldSuffix.length(), newSuffix);
}

//example: "../../../abc.d" => "../../../"
QString FileManager::getDir(QString fileName)
{
    QStringList list = fileName.split('/');
    list.pop_back();
    return list.join("/")+"/";
}

// example: "../../../abc.d" => abc
QString FileManager::getName(QString fileName)
{
    QStringList list = fileName.split('/');
    fileName = list.back();
    list = fileName.split('.');
    list.pop_back();
    return list.join('.');
}

// example: "../../../abc.d" => abc.d
QString FileManager::getNameWithExtension(QString fileName)
{
    QStringList list = fileName.split('/');
    return list.back();
}

void FileManager::saveJson(QJsonObject json, QString fileName)
{
    QJsonDocument document;
    document.setObject(json);
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(nullptr, "Warning", fileName+": file not open");
    }else{
        file.write(document.toJson());
        file.close();
    }
}

QJsonObject FileManager::readJson(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(nullptr, "Warning", fileName+": file not open");
    }else{
        QString val = file.readAll();
        file.close();
        QJsonDocument document = QJsonDocument::fromJson(val.toUtf8());
        if (!document.isNull()){
            if (document.isObject()){
                return document.object();
            }else{
                throw "document is not object";
            }
        }else{
            throw "document read error";
        }
    }
}

/*--------------------------FileManager-----------------------------*/

FileManager::FileManager(QObject *parent) : QObject(parent)
{
    changeNotSaved=false;
    mode = Close;
}

FileManager::FileManager(QString fileName, QString outputExtension, QObject *parent) : QObject(parent)
{
    setAll(fileName, outputExtension);
}

// assume fileNames are not empty
FileManager::FileManager(QStringList fileNames, QString outputExtension, QObject *parent) : QObject(parent)
{
    setAll(fileNames, outputExtension);
}

bool FileManager::hasChangeNotSaved() const { return changeNotSaved; }

QString FileManager::getCurrentImageFile() const { return imageFiles[curIdx]; }

QString FileManager::getCurrentOutputFile() const { return outputFiles[curIdx]; }

QString FileManager::getLabelFile() const { return labelFile; }

FileMode FileManager::getMode() const { return mode; }

void FileManager::close()
{
    mode = Close;
    imageFiles.clear();
    outputFiles.clear();
    labelFile = QString();
    curIdx=0;
    changeNotSaved=false;
    emit prevEnableChanged(false);
    emit nextEnableChanged(false);
    emit fileListSetup();
}

int FileManager::count(){ return imageFiles.length(); }

void FileManager::setAll(QString fileName, QString outputExtension)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = SingleImage;
    curIdx = 0;
    imageFiles<<fileName;
    outputFiles<<getDir(fileName) + getName(fileName) + outputExtension;
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::setAll(QStringList fileNames, QString outputExtension)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = MultiImage;
    curIdx = 0;
    fileNames.sort();
    for (auto fileName: fileNames){
        imageFiles<<fileName;
        outputFiles<<getDir(fileName) + getName(fileName) + outputExtension;
    }
    labelFile = getDir(fileNames[0]) + "labels.json";
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::setAllDetection3D(QStringList fileNames, QString outputExtension)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = ThirdDImage;
    curIdx = 0;
    fileNames.sort();
    imageFiles = fileNames;
    outputFiles << getDir(fileNames[0])+outputExtension;
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::setChangeNotSaved() { changeNotSaved=true; }

void FileManager::resetChangeNotSaved(){ changeNotSaved=false; }

void FileManager::prevFile(){
    if (curIdx>0) selectFile(curIdx-1);
}

void FileManager::nextFile(){
    if (curIdx<count()-1) selectFile(curIdx+1);
}

void FileManager::selectFile(int idx){
    if (curIdx==idx) return;
    curIdx=idx;
    emitPrevNextEnable();
}

void FileManager::emitPrevNextEnable(){
    emit prevEnableChanged(curIdx>0);
    emit nextEnableChanged(curIdx<count()-1);
}
