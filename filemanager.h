#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <exception>
#include <string>

enum FileMode{
    Close, SingleImage, MultiImage, ThirdDImage
};

class FileException: public std::exception{
public:
    FileException(std::string message);
    const char * what() const noexcept;
private:
    std::string message;
};

class FileManager : public QObject
{
    Q_OBJECT
public:
    static QString changeExtensionName(QString fileName, QString newExtension);

    static QString getDir(QString fileName);
    static QString getName(QString fileName);
    static QString getNameWithExtension(QString fileName);
    static void saveJson(QJsonObject json, QString fileName);
    static QJsonObject readJson(QString fileName);

    explicit FileManager(QObject *parent = nullptr);

    QString imageFileNameAt(int idx) const { return imageFiles[idx]; }
    bool hasChangeNotSaved() const;
    QString getCurrentImageFile() const;
    QString getCurrentOutputFile() const;
    QString getLabelFile() const;
    FileMode getMode() const;
    QStringList allImageFiles() const { return imageFiles; }
    int getCurIdx() const  { return curIdx; }
    void close();

    int count();
    void setSingleImage(QString fileName, QString outputSuffix);
    void setMultiImage(QStringList fileNames, QString outputSuffix);
    void set3DImage(QStringList fileNames, QString outputSuffix);

signals:
    void prevEnableChanged(bool);
    void nextEnableChanged(bool);
    void fileListSetup();
public slots:
    void setChangeNotSaved();
    void resetChangeNotSaved();

    void prevFile();
    void nextFile();
    void selectFile(int idx);

private:
    QStringList imageFiles;
    QStringList outputFiles;
    QString labelFile;
    int curIdx;
    bool changeNotSaved;
    FileMode mode;

    void emitPrevNextEnable();
};

#endif // FILEMANAGER_H
