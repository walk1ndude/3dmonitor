#ifndef CAMERAOUTPUT_H
#define CAMERAOUTPUT_H

#include <QtQuick/QQuickPaintedItem>
#include <QtGui/QImage>
#include <QtGui/QPainter>

using namespace std;

class CameraOutput : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QImage cameraimage
               READ cameraimage
               WRITE setCameraimage
               NOTIFY cameraimageChanged)

private:
    QImage CVImage;

public:
    explicit CameraOutput(QQuickItem *parent = nullptr);
    QImage cameraimage();

signals:
    void cameraimageChanged();

public slots:
    void paint(QPainter *painter);
    void setCameraimage(const QImage &image);
};

#endif // CAMERAOUTPUT_H
