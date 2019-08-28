#include "labelmanager.h"

LabelManager::LabelManager(QObject *parent) : QObject(parent)
{

}

LabelProperty LabelManager::operator[](QString label) const {
    return labels[label];
}

bool LabelManager::hasLabel(QString label) const {
    return labels.find(label)!=labels.end();
}

void LabelManager::checkLabel(QString label) const {
    if (labels.find(label)==labels.end())
        throw "can not find label in labelconfig";
}

QList<LabelProperty> LabelManager::getLabels() const {
    return labels.values();
}

QColor LabelManager::getColor(QString label) const {
    checkLabel(label);
    return labels[label].color;
}

void LabelManager::addLabel(QString label, QColor color, bool visible){
    labels[label] = LabelProperty(label, color, visible);
    emit labelAdded(label, color, visible);
    emit configChanged();
}

void LabelManager::removeLabel(QString label){
    labels.remove(label);
    emit labelRemoved(label);
    emit configChanged();
}

void LabelManager::setColor(QString label, QColor color){
    checkLabel(label);
    if (labels[label].color != color){
        labels[label].color = color;
        emit colorChanged(label, color);
        emit configChanged();
    }
}

void LabelManager::setVisible(QString label, bool visible){
    checkLabel(label);
    if (labels[label].visible != visible){
        labels[label].visible = visible;
        emit visibelChanged(label, visible);
        emit configChanged();
    }
}
