#ifndef COMMON_H
#define COMMON_H

#include "canvasbase.h"
#include "cubeannotationitem.h"
#include <QList>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <cmath>
#include <map>

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

const std::map<TaskMode,QString> taskText ={{DETECTION, "Detection "},
                                            {SEGMENTATION, "Segmentation "},
                                            {DETECTION3D, "3D Detection "},
                                            {SEGMENTATION3D, "3d Segmentation "}};
const std::map<DrawMode,QString> drawModeText={{RECTANGLE, "Rectangle "},
                                               {CIRCLEPEN, "Circle Pen "},
                                               {SQUAREPEN, "Square Pen "},
                                               {CONTOUR, "Contour "},
                                               {POLYGEN, "Polygonal Contour "}};

extern inline bool is2dTask(const QString &text) {
    return text == taskText.at(DETECTION) || text == taskText.at(SEGMENTATION);
}
extern inline bool is3dTask(const QString &text) {
    return text == taskText.at(DETECTION3D) || text == taskText.at(SEGMENTATION3D);
}
extern inline bool isDetectTask(const QString &text) {
    return text == taskText.at(DETECTION) || text == taskText.at(DETECTION3D);
}
extern inline bool isSegmentTask(const QString &text) {
    return text == taskText.at(SEGMENTATION) || text == taskText.at(SEGMENTATION3D);
}
extern DrawMode getDrawModeFromText(const QString &text);
extern TaskMode getTaskFromText(const QString &text);

#endif // COMMON_H