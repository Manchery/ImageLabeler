#ifndef LABELMANAGER_H
#define LABELMANAGER_H

#include <QObject>
#include <QColor>
#include <QList>
#include <QMap>

class LabelProperty
{
public:
    LabelProperty() { }
    LabelProperty(QString label, QColor color=QColor(), bool visible=true) :
        label(label), color(color), visible(visible) { }
    QString label;
    QColor color;
    bool visible;
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

signals:
    void configChanged();
    void labelRemoved(QString label);
    void labelAdded(QString label, QColor color, bool visibile);
    void visibelChanged(QString label, bool visible);
    void colorChanged(QString label, QColor color);
public slots:
    void addLabel(QString label, QColor color=QColor(), bool visible=true);
    void removeLabel(QString label);
    void setColor(QString label, QColor color);
    void setVisible(QString label, bool visible);

private:
    QMap<QString, LabelProperty> labels;
    void checkLabel(QString label) const;
};

#endif // LABELMANAGER_H
