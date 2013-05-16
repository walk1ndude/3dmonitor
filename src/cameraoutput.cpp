#include <QtCore/QDebug>
#include "cameraoutput.h"

CameraOutput::CameraOutput(QQuickItem *parent) : QQuickPaintedItem(parent){
}

void CameraOutput::paint(QPainter *painter){
    //draw image on CameraOutput
    painter->drawImage(0,0,cvImage);
}

QImage CameraOutput::cameraImage(){
    return cvImage;
}

void CameraOutput::setCameraImage(const QImage &image){
    // recieve new image from camera thread and paint it
    cvImage = image.scaled(320,240,Qt::KeepAspectRatio);
    update();
    emit signalCameraImageChanged();
}
