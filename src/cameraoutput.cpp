#include <QtCore/QDebug>
#include "cameraoutput.h"

CameraOutput::CameraOutput(QQuickItem *parent) : QQuickPaintedItem(parent){
}

void CameraOutput::paint(QPainter *painter){
    //draw image on CameraOutput
    painter->drawImage(0,0,CVImage);
}

QImage CameraOutput::cameraimage(){
    return CVImage;
}

void CameraOutput::setCameraimage(const QImage &image){
    // recieve new image from camera thread and paint it
    CVImage = image.scaled(320,240,Qt::KeepAspectRatio);
    update();
    emit cameraimageChanged();
}
