#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QVariant>
#include <QtGui/QImage>

#include "capturedface.h"

#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace cv;
using namespace std;

enum Mode {
    DETECTION,
    TRACKING,
    TRACKINGWITHKALMAN
};

class CameraCapture : public QObject
{
    Q_OBJECT
public:
    explicit CameraCapture(QObject *parent = nullptr);
    ~CameraCapture();
    bool initializeCV();

private:
    int vmin;
    int vmax;
    int smin;

    unsigned int avcountMax;

    bool CVisHSV;

    QMutex TrackingNow;

    qreal CVRadius;

    Mode CVMode;

    Size CVFrameSize;

    Point CVAverageHeadPos;

    VideoCapture CVCapture;

    CascadeClassifier CVCascadeFace;
    CascadeClassifier CVCascadeNose;
    CascadeClassifier CVCascadeEye;

    Mat CVFrame;

    deque<RotatedRect>averagePosition;
    CapturedFace* CVCapturedFace;

    QImage Mat2QImage(Mat image);

    QImage getFace();

    void doHaar();

    void doCamshift();
    Rect newBoundingRect(RotatedRect &trackH);
    Rect getApproxPosition(RotatedRect &trackH);
    void paintImage(Rect &object, RotatedRect &trackH);
    void deleteFaceCam();

signals:
    void signalRedraw(QVariant HeadX, QVariant HeadY);
    void signalError(QString);
    void signalRecapture(QImage);

public slots:
    void sminchanged(int);
    void vminchanged(int);
    void vmaxchanged(int);

    void recapture();

    void keyBpressed();
    void keyKpressed();
    void keyHpressed();
    void keyRpressed();

};

#endif // CAMERACAPTURE_H
