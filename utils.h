#ifndef UTILS_H
#define UTILS_H

#include <QList>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <cmath>

extern QList<QColor> randomColors(int count);
extern QColor randomColor();

extern bool onRectTop(QPoint pos, QRect rect);
extern bool onRectBottom(QPoint pos, QRect rect);
extern bool onRectLeft(QPoint pos, QRect rect);
extern bool onRectRight(QPoint pos, QRect rect);

#endif // UTILS_H
