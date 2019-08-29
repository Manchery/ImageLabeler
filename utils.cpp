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

bool onRectTop(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.top())<=2;
}

bool onRectBottom(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.bottom())<=2;
}

bool onRectLeft(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.left())<=2;
}

bool onRectRight(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.right())<=2;
}
