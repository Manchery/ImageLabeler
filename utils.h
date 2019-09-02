#ifndef UTILS_H
#define UTILS_H

#include "cubeannotationitem.h"
#include <QList>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <cmath>

extern QList<QColor> randomColors(int count);
extern QColor randomColor();

enum EditingRectEdge{
    TOP, BOTTOM, LEFT, RIGHT
};

extern bool onRectTop(QPoint pos, QRect rect);
extern bool onRectBottom(QPoint pos, QRect rect);
extern bool onRectLeft(QPoint pos, QRect rect);
extern bool onRectRight(QPoint pos, QRect rect);

// z: min -> max <=> top -> bottom
// x: min -> max <=> left -> right
// y: min -> max <=> back -> front
enum EditingCubeFace{
    TOPf, BOTTOMf, LEFTf, RIGHTf, FRONTf, BACKf
};

extern bool onCubeTop(Point3D pos, Cuboid cube);
extern bool onCubeBottom(Point3D pos, Cuboid cube);
extern bool onCubeLeft(Point3D pos, Cuboid cube);
extern bool onCubeRight(Point3D pos, Cuboid cube);
extern bool onCubeFront(Point3D pos, Cuboid cube);
extern bool onCubeBack(Point3D pos, Cuboid cube);

const int DEFAULT_PEN_WIDTH=15;

#endif // UTILS_H
