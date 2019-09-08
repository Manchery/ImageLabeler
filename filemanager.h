#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <exception>
#include <string>

// 表示文件打开模式的枚举类型
//  Close 表示没有文件打开
//  SingleImage 表示打开单个文件，适用于2D的检测或分割
//  MultiImage 表示打开一个文件夹下的文件，适用于2D的检测或分割
//  ThirdDImage 表示打开一个文件下的3D图片，适用于3D的检测和分割
enum FileMode{
    Close, SingleImage, MultiImage, ThirdDImage
};

/* FileException
 * 简介：异常类，用于抛出由于文件读取等造成的异常
 */
class FileException: public std::exception{
public:
    FileException(std::string message);
    // 返回 message
    const char * what() const noexcept;
private:
    // 用于存储异常的提示信息
    std::string message;
};

/* FileManager
 * 简介：管理与文件相关的状态的类型
 * 功能：a. 实现静态函数作为更便捷的处理文件名、读写文件的接口
 *      b. 打开文件或文件夹，生成默认的输出文件名（包括label和annotation的输出文件）
 *      c. 记录距离上次保存后是否有未保存的修改
 *      d. 可选中文件，切换显示的图像，并与 ui->fileListWidget 同步
 */

class FileManager : public QObject
{
    Q_OBJECT
public:
    // 从完整的文件路径中获取文件所在文件夹的路径 example: "../../../abc.d" => "../../../"
    static QString getDir(QString fileName);
    // 从完整的文件路径中获取文件名（不含扩展名） example: "../../../abc.d" => abc
    static QString getName(QString fileName);
    // 从完整的文件路径中获取文件名（含扩展名）   example: "../../../abc.d" => abc.d
    static QString getNameWithExtension(QString fileName);
    // 将 $json$ 写入文件 $fileName$
    static void saveJson(QJsonObject json, QString fileName);
    // 从文件 $fileName$ 中读取json，可能抛出 FileException 和 JsonException
    static QJsonObject readJson(QString fileName);

    explicit FileManager(QObject *parent = nullptr);

    QString imageFileNameAt(int idx) const { return imageFiles[idx]; }
    bool hasChangeNotSaved() const { return changeNotSaved; }
    QString getCurrentImageFile() const { return imageFiles[curIdx]; }
    QString getCurrentOutputFile() const { return mode==ThirdDImage ? outputFiles[0] : outputFiles[curIdx]; }
    QString getLabelFile() const { return labelFile; }
    FileMode getMode() const { return mode; }
    const QStringList &allImageFiles() const { return imageFiles; }
    int getCurIdx() const  { return curIdx; }
    int count() const { return imageFiles.length(); }

    // 关闭打开的文件
    void close();

    // 载入文件列表，outputSuffix表示输出文件的名称的后缀，详见 common.h
    void setSingleImage(QString fileName, QString outputSuffix);
    void setMultiImage(QStringList fileNames, QString outputSuffix);
    void set3DImage(QStringList fileNames, QString outputSuffix);

signals:
    void prevEnableChanged(bool);   // 是否可以切换至上一张图片，依据是否已经是第一张
    void nextEnableChanged(bool);   // 是否可以切换至下一张图片，依据是否已经是最后一张
    void fileListSetup();           // 文件列表载入完毕
public slots:
    void setChangeNotSaved() { changeNotSaved=true; }
    void resetChangeNotSaved(){ changeNotSaved=false; }
    void prevFile();            // 切换到上一张
    void nextFile();            // 切换到下一张
    void selectFile(int idx);   // 切换到指定的一张图片

private:
    QStringList imageFiles;
    QStringList outputFiles;
    QString labelFile;

    int curIdx;
    bool changeNotSaved;
    FileMode mode;

    // 根据当前curIdx重新发出相应的prevEnableChanged和nextEnableChanged信号
    void emitPrevNextEnable();
};

#endif // FILEMANAGER_H
