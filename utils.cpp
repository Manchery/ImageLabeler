#include "utils.h"
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

static const int eps=5;

bool onRectTop(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.top())<=eps;
}

bool onRectBottom(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.bottom())<=eps;
}

bool onRectLeft(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.left())<=eps;
}

bool onRectRight(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.right())<=eps;
}

bool onCubeTop(Point3D pos, Cuboid cube)
{
    return cube.minX()-eps<=pos.x && pos.x<=cube.maxX()+eps &&
            cube.minY()-eps<=pos.y && pos.y<=cube.maxY()+eps &&
            abs(pos.z - cube.minZ())<eps;
}

bool onCubeBottom(Point3D pos, Cuboid cube)
{
    return cube.minX()-eps<=pos.x && pos.x<=cube.maxX()+eps &&
            cube.minY()-eps<=pos.y && pos.y<=cube.maxY()+eps &&
            abs(pos.z - cube.maxZ())<eps;
}

bool onCubeLeft(Point3D pos, Cuboid cube)
{
    return cube.minZ()-eps<=pos.z && pos.z<=cube.maxZ()+eps &&
            cube.minY()-eps<=pos.y && pos.y<=cube.maxY()+eps &&
            abs(pos.x - cube.minX())<eps;
}

bool onCubeRight(Point3D pos, Cuboid cube)
{
    return cube.minZ()-eps<=pos.z && pos.z<=cube.maxZ()+eps &&
            cube.minY()-eps<=pos.y && pos.y<=cube.maxY()+eps &&
            abs(pos.x - cube.maxX())<eps;
}

bool onCubeFront(Point3D pos, Cuboid cube)
{
    return cube.minZ()-eps<=pos.z && pos.z<=cube.maxZ()+eps &&
            cube.minX()-eps<=pos.x && pos.x<=cube.maxX()+eps &&
            abs(pos.y - cube.maxY())<eps;
}

bool onCubeBack(Point3D pos, Cuboid cube)
{
    return cube.minZ()-eps<=pos.z && pos.z<=cube.maxZ()+eps &&
            cube.minX()-eps<=pos.x && pos.x<=cube.maxX()+eps &&
            abs(pos.y - cube.minY())<eps;
}
