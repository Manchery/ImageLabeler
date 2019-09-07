#include "common.h"
#include <QIcon>
#include <cmath>
#include <ctime>
#include <cstdlib>

//! Reference: https://gist.github.com/ialhashim/b39a68cf48a0d2e66621
QList<QColor> randomColors(int count){
    static qreal currentHue = static_cast<qreal>(qrand())/RAND_MAX;
    QList<QColor> colors;
    for (int i = 0; i < count; i++){
        currentHue += 0.618033988749895;
        currentHue = std::fmod(currentHue, 1.0);
        colors.push_back( QColor::fromHslF(currentHue, 1.0, 0.5) );
    }
    return colors;
}

QColor randomColor(){
    static qreal currentHue = static_cast<qreal>(qrand())/RAND_MAX;
    currentHue += 0.618033988749895;
    currentHue = std::fmod(currentHue, 1.0);
    return QColor::fromHslF(currentHue, 1.0, 0.5);
}

bool onRectTop(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.top())<=pixEps;
}

bool onRectBottom(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.bottom())<=pixEps;
}

bool onRectLeft(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.left())<=pixEps;
}

bool onRectRight(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.right())<=pixEps;
}

bool onCubeTop(Point3D pos, Cuboid cube)
{
    return cube.minX()<=pos.x && pos.x<=cube.maxX() &&
            cube.minY()<=pos.y && pos.y<=cube.maxY() &&
            abs(pos.z - cube.minZ())<pixEps;
}

bool onCubeBottom(Point3D pos, Cuboid cube)
{
    return cube.minX()<=pos.x && pos.x<=cube.maxX() &&
            cube.minY()<=pos.y && pos.y<=cube.maxY() &&
            abs(pos.z - cube.maxZ())<pixEps;
}

bool onCubeLeft(Point3D pos, Cuboid cube)
{
    return cube.minZ()<=pos.z && pos.z<=cube.maxZ() &&
            cube.minY()<=pos.y && pos.y<=cube.maxY() &&
            abs(pos.x - cube.minX())<pixEps;
}

bool onCubeRight(Point3D pos, Cuboid cube)
{
    return cube.minZ()<=pos.z && pos.z<=cube.maxZ() &&
            cube.minY()<=pos.y && pos.y<=cube.maxY() &&
            abs(pos.x - cube.maxX())<pixEps;
}

bool onCubeFront(Point3D pos, Cuboid cube)
{
    return cube.minZ()<=pos.z && pos.z<=cube.maxZ() &&
            cube.minX()<=pos.x && pos.x<=cube.maxX() &&
            abs(pos.y - cube.maxY())<pixEps;
}

bool onCubeBack(Point3D pos, Cuboid cube)
{
    return cube.minZ()<=pos.z && pos.z<=cube.maxZ() &&
            cube.minX()<=pos.x && pos.x<=cube.maxX() &&
            abs(pos.y - cube.minY())<pixEps;
}

DrawMode getDrawModeFromText(const QString &text){
    for (auto item: drawModeText)
        if (item.second==text)
            return item.first;
    throw "can not find text "+text+" for map drawModeText";
}

TaskMode getTaskFromText(const QString &text){
    for (auto item: taskText)
        if (item.second==text)
            return item.first;
    throw "can not find text "+text+" for map taskText";
}

QIcon iconFromColor(QColor color, QSize size)
{
    QPixmap pixmap(size);
    if (color.isValid()){
        pixmap.fill(color);
    }else{
        pixmap.fill(Qt::white);
    }
    return QIcon(pixmap);
}
