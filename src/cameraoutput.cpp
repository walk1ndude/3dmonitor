#include <QtCore/QDebug>
#include "cameraoutput.h"

CameraOutput::CameraOutput(QQuickItem *parent) : QQuickPaintedItem(parent){
}

void CameraOutput::paint(QPainter * painter){
    //draw image on CameraOutput
    painter->drawImage(0,0,cvImage);
}

QImage CameraOutput::cameraImage(){
    return cvImage;
}

QSize CameraOutput::size(){
    return cvFrameSize;
}

void CameraOutput::setCameraImage(const QImage & image){
    // recieve new image from camera thread and paint it
    cvImage = image.scaled(cvFrameSize.width(),cvFrameSize.height(),Qt::KeepAspectRatio);
    update();
    emit signalCameraImageChanged();
}

void CameraOutput::setSize(const QSize & size){
    cvFrameSize = size;
    this->setWidth(size.width());
    this->setHeight(size.height());
    emit signalSizeChanged();
}
