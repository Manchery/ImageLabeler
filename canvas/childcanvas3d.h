#ifndef CANVAS2D_H
#define CANVAS2D_H

#include <QWidget>
#include <QPixmap>
#include <QImage>

class ChildCanvas3D : public QWidget
{
    Q_OBJECT
public:
    explicit ChildCanvas3D(QWidget *parent = nullptr);

    QSize getImageSize() const { return image.size(); }
    int getImageHeight() const { return image.height(); }
    int getImageWidth() const { return image.width(); }

    void loadImage(const QImage &newImage);

    bool outOfPixmap(QPoint pos);
    void setScale(qreal new_scale);
signals:
    void mouseMoved(QPoint pos, bool mousePressed);

public slots:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseSetRequest(QPoint pos);

private:
    qreal scale;
    QImage image;

    QPoint mousePos;

    bool mousePressing;

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);
};

#endif // CANVAS2D_H
