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

    Q_PROPERTY(QSize size
               READ size
               WRITE setSize
               NOTIFY signalSizeChanged)

private:
    QImage cvImage;
    QSize cvFrameSize;

public:
    explicit CameraOutput(QQuickItem *parent = nullptr);
    QImage cameraImage();
    QSize size();

signals:
    void signalCameraImageChanged();
    void signalSizeChanged();

public slots:
    void paint(QPainter *painter);
    void setCameraImage(const QImage &image);
    void setSize(const QSize &size);
};

#endif // CAMERAOUTPUT_H
