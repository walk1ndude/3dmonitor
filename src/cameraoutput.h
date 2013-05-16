#ifndef CAMERAOUTPUT_H
#define CAMERAOUTPUT_H

#include <QtQuick/QQuickPaintedItem>
#include <QtGui/QImage>
#include <QtGui/QPainter>

using namespace std;

class CameraOutput : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QImage cameraImage
               READ cameraImage
               WRITE setCameraImage
               NOTIFY signalCameraImageChanged)

private:
    QImage cvImage;

public:
    explicit CameraOutput(QQuickItem *parent = nullptr);
    QImage cameraImage();

signals:
    void signalCameraImageChanged();

public slots:
    void paint(QPainter *painter);
    void setCameraImage(const QImage &image);
};

#endif // CAMERAOUTPUT_H
