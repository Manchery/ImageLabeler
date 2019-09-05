#include "filemanager.h"
#include "annotationitem.h"
#include "common.h"
#include <QtDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFile>

FileException::FileException(std::string message):message(message) {}

const char *FileException::what() const noexcept {
    return message.c_str();
}

/*--------------------------FileManager static member-----------------------------*/

QString FileManager::changeExtensionName(QString fileName, QString newExtension){
    QStringList list = fileName.split('.');
    QString oldExtension = list.back();
    QString newFileName = fileName.replace(fileName.length()-oldExtension.length(),
                                           oldExtension.length(), newExtension);
    qDebug()<<newFileName;
    return newFileName;
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
        QMessageBox::warning(nullptr, "File Error", fileName+": file not open");
    }else{
        file.write(document.toJson());
        file.close();
    }
}

QJsonObject FileManager::readJson(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        throw FileException(std::string(QByteArray(fileName.toLocal8Bit()).data())+": file not open");
    }else{
        QString val = file.readAll();
        file.close();
        QJsonDocument document = QJsonDocument::fromJson(val.toUtf8());
        if (!document.isNull()){
            if (document.isObject()){
                return document.object();
            }else{
                throw JsonException("document is not object");
            }
        }else{
            throw JsonException("document read error");
        }
    }
}

/*--------------------------FileManager non-static member-----------------------------*/

FileManager::FileManager(QObject *parent) : QObject(parent)
{
    changeNotSaved=false;
    mode = Close;
}

bool FileManager::hasChangeNotSaved() const { return changeNotSaved; }

QString FileManager::getCurrentImageFile() const { return imageFiles[curIdx]; }

QString FileManager::getCurrentOutputFile() const {
    if (mode==ThirdDImage)
        return outputFiles[0];
    else {
        return outputFiles[curIdx];
    }
}

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

void FileManager::setSingleImage(QString fileName, QString outputSuffix)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = SingleImage;
    curIdx = 0;
    imageFiles<<fileName;
    outputFiles<<getDir(fileName) + getName(fileName) + outputSuffix;
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::setMultiImage(QStringList fileNames, QString outputSuffix)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = MultiImage;
    curIdx = 0;
    fileNames.sort();
    for (auto fileName: fileNames){
        imageFiles<<fileName;
        outputFiles<<getDir(fileName) + getName(fileName) + outputSuffix;
    }
    labelFile = getDir(fileNames[0]) + FILENAME_DIR_LABEL;
    emitPrevNextEnable();
    emit fileListSetup();
}

void FileManager::set3DImage(QStringList fileNames, QString outputSuffix)
{
    imageFiles.clear(); outputFiles.clear(); labelFile = QString();
    changeNotSaved=false;

    mode = ThirdDImage;
    curIdx = 0;
    fileNames.sort();
    imageFiles = fileNames;
    outputFiles << getDir(fileNames[0])+outputSuffix;
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


