#include "annotationitem.h"
#include <QImage>
#include <QPainter>
#include <QPainterPath>

AnnotationItem::AnnotationItem():label(), id(-1) {}

AnnotationItem::AnnotationItem(QString label, int id):label(label), id(id) {}

AnnotationItem::~AnnotationItem() {}

