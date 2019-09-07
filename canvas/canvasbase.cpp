#include "canvasbase.h"
#include "common.h"

using namespace CanvasUtils;

CanvasBase::CanvasBase(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent) :
    QWidget(parent),
    pAnnoContainer(pAnnoContainer),
    pLabelManager(pLabelManager)
{
    scale = 1.0;
    lastPenWidth=DEFAULT_PEN_WIDTH;
}

QString CanvasBase::modeString() const{
    QString modeStr("");

    switch(task){
    case DETECTION:  modeStr+="Detection"; break;
    case SEGMENTATION: modeStr+="Segmentation"; break;
    case DETECTION3D: modeStr+="Detection 3D"; break;
    case SEGMENTATION3D: modeStr+="Segmentation 3D"; break;
//    default: modeStr+="Unknown task"; break;
    }

    switch (mode) {
    case DRAW: modeStr+=", Draw"; break;
    case SELECT: modeStr+=", Select & Edit"; break;
    case MOVE: modeStr+=", Move"; break;
//    default: modeStr+="Unknown mode"; break;
    }

    if (mode==DRAW){
        switch (drawMode) {
        case RECTANGLE: modeStr+= task==DETECTION3D?", Cuboid":", Rectangle" ; break;
        case CONTOUR: modeStr+=", Contour"; break;
        case SQUAREPEN: modeStr+=", Square Pen"; break;
        case CIRCLEPEN: modeStr+=", Circle Pen"; break;
        case POLYGEN: modeStr+=", Polygen Contour"; break;
//        default: modeStr+="Unknown mode"; break;
        }
    }
    return modeStr;
}
