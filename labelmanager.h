#ifndef LABELMANAGER_H
#define LABELMANAGER_H

#include <QObject>
#include <QColor>
#include <QList>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>

class LabelProperty
{
public:
    LabelProperty();
    LabelProperty(QString label, QColor color=QColor(), bool visible=true);
    QString label;
    QColor color;
    bool visible;

    QJsonObject toJsonObject();
    void fromJsonObject(QJsonObject json);
};

class LabelManager : public QObject
{
    Q_OBJECT
public:
    explicit LabelManager(QObject *parent = nullptr);

    LabelProperty operator[](QString label) const;
    bool hasLabel(QString label) const;
    QList<LabelProperty> getLabels() const;
    QColor getColor(QString label) const;

    QJsonArray toJsonArray();
    void fromJsonArray(QJsonArray json);
    void fromJsonObject(QJsonObject json);

signals:
    void configChanged();
    void labelRemoved(QString label);
    void labelAdded(QString label, QColor color, bool visibile);
    void visibelChanged(QString label, bool visible);
    void colorChanged(QString label, QColor color);
    void allCleared();
public slots:
    void addLabel(QString label, QColor color=QColor(), bool visible=true);
    void removeLabel(QString label);
    void setColor(QString label, QColor color);
    void setVisible(QString label, bool visible);
    void allClear();

private:
    QMap<QString, LabelProperty> labels;
    void checkLabel(QString label) const;
};

#endif // LABELMANAGER_H
