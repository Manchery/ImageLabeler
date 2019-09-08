#ifndef COMMON_H
#define COMMON_H

#include "canvasbase.h"
#include "cubeannotationitem.h"

#include <QList>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QString>

#include <cmath>
#include <map>

namespace ColorUtils{
    extern QList<QColor> randomColors(int count);
    extern QColor randomColor();

    extern QIcon iconFromColor(QColor color, QSize size = QSize(16,16));
}

namespace CanvasUtils {
    const int DEFAULT_PEN_WIDTH=15;
    const int LABEL_PIXEL_SIZE = 15;

    const int pixEps=5;

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
}

namespace StringConstants{

    const std::map<TaskMode,QString> taskText ={{DETECTION, "Detection "},
                                                {SEGMENTATION, "Segmentation "},
                                                {DETECTION3D, "3D Detection "},
                                                {SEGMENTATION3D, "3D Segmentation "}};
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

    // for single image
    const QString SUFFIX_DET_LABEL_ANNO("_detect_labels_annotations.json");
    const QString SUFFIX_SEG_LABEL_ANNO("_segment_labels_annotations.json");

    // for multiple image
    const QString FILENAME_DIR_LABEL("labels.json");
    const QString SUFFIX_DET_ANNO("_detect_annotations.json");
    const QString SUFFIX_SEG_ANNO("_segment_annotations.json");

    // for 3d image
    const QString FILENAME_DET3D_LABEL_ANNO("detect3d_labels_annotations.json");
    const QString FILENAME_SEG3D_LABEL_ANNO("segment3d_labels_annotations.json");

    // for image result
    const QString SUFFIX_SEG_COLOR("_segment_color.png");
    const QString SUFFIX_SEG_LABELID("_segment_labelId.png");
    const QString SUFFIX_SEG3D_COLOR("_segment3d_color.png");
    const QString SUFFIX_SEG3D_LABELID("_segment3d_labelId.png");
}



#endif // COMMON_H
