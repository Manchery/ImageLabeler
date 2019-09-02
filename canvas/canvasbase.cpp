#include "canvasbase.h"

CanvasBase::CanvasBase(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent) :
    QWidget(parent),
    pAnnoContainer(pAnnoContainer),
    pLabelManager(pLabelManager)
{
    scale = 1.0;
}

QString CanvasBase::modeString() const{
    QString modeStr("");

    switch(task){
    case DETECTION:  modeStr+="Detection"; break;
    case SEGMENTATION: modeStr+="Segmentation"; break;
    default: modeStr+="Unknown task"; break;
    }

    switch (mode) {
    case DRAW: modeStr+=", Draw"; break;
    case SELECT: modeStr+=", Select & Edit"; break;
    case MOVE: modeStr+=", Move"; break;
//    default: modeStr+="Unknown mode"; break;
    }

    if (mode==DRAW){
        switch (drawMode) {
        case RECTANGLE: modeStr+=", Rectangle"; break;
        case CONTOUR: modeStr+=", Contour"; break;
        case SQUAREPEN: modeStr+=", Square Pen"; break;
        case CIRCLEPEN: modeStr+=", Circle Pen"; break;
        case POLYGEN: modeStr+=", Polygen Contour"; break;
//        default: modeStr+="Unknown mode"; break;
        }
    }
    return modeStr;
}
