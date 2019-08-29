#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

enum FileMode{
    Close, SingleImage, MultiImage, ThirdDImage
};

class FileManager : public QObject
{
    Q_OBJECT
public:
    static QString changeExtensionName(QString fileName, QString newExtension);
    static QString getDir(QString fileName);
    static QString getName(QString fileName);
    static void saveJson(QJsonObject json, QString fileName);
    static QJsonObject readJson(QString fileName);

    explicit FileManager(QObject *parent = nullptr);
    explicit FileManager(QString fileName, QString outputExtension = "json", QObject *parent = nullptr);
    explicit FileManager(QStringList fileNames, QString outputExtension = "json", QObject *parent = nullptr);

    bool hasChangeNotSaved() const;
    QString getCurrentImageFile() const;
    QString getCurrentOutputFile() const;
    QString getLabelFile() const;
    FileMode getMode() const;
    void close();

    int count();
    void setAll(QString fileName, QString outputExtension = "json");
    void setAll(QStringList fileNames, QString outputExtension = "json");

signals:
    void prevEnableChanged(bool);
    void nextEnableChanged(bool);
public slots:
    void setChangeNotSaved();
    void resetChangeNotSaved();
    void setOutputFile(QString output, int idx);

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
