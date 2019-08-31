#ifndef CANVAS2D_H
#define CANVAS2D_H

#include <QWidget>
#include <QPixmap>
#include <QImage>

class Canvas2D : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas2D(QWidget *parent = nullptr);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QSize size() const { return image.size(); }
    int height() const { return image.height(); }
    int width() const { return image.width(); }

//    void loadPixmap(QPixmap);
    void loadImage(const QImage &newImage);

    bool outOfPixmap(QPoint pos);
signals:
    void mouseMoved(QPoint pos);

public slots:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseSetRequest(QPoint pos);

private:
//    QPixmap pixmap;
    QImage image;

    QPoint mousePos;


    QPoint boundedPixelPos(QPoint pos);
};

#endif // CANVAS2D_H
