#include <QtCore/QDebug>

#include "cameracapture.h"

#define FACE_CASCADE_PATH "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt2.xml"
#define EYE_CASCADE_PATH "/usr/share/opencv/haarcascades/haarcascade_eye.xml"
#define NOSE_CASCADE_PATH "/usr/share/opencv/haarcascades/haarcascade_mcs_nose.xml"


CameraCapture::CameraCapture(QObject *parent) : QObject(parent){

    CVFrameSize = Point(640,480);

    CVisHSV = false;

    CVMode = DETECTION;

    CVAverageHeadPos = Point(0,0);
    CVRadius = 3;

    avcountMax = 7;

    // min value
    vmin = 55;
    // max value
    vmax = 130;
    // min saturation
    smin = 65;
}

CameraCapture::~CameraCapture(){
    // release camera
    CVCapture.release();
}

bool CameraCapture::initializeCV(){

    if (!CVCapture.open(CV_CAP_ANY)){
            emit signalError("Can't open camera device");
            qDebug() << "Can't open camera device";
            return false;
        }

    // set camera resolution
    CVCapture.set(CV_CAP_PROP_FRAME_WIDTH, CVFrameSize.width);
    CVCapture.set(CV_CAP_PROP_FRAME_HEIGHT, CVFrameSize.height);

    if (!CVCascadeFace.load(FACE_CASCADE_PATH)){
            emit signalError("No Face Cascade Found");
            qDebug() << "No Face Cascade Found";
            return false;
        }
        else /*
        if (!CVCascadeNose.load(NOSE_CASCADE_PATH)){
                emit signalError("No Nose Cascade Found");
                qDebug() << "No Nose Cascade Found";
                return false;
            }
            else
            if (!CVCascadeEye.load(EYE_CASCADE_PATH)){
                    emit signalError("No Eye Cascade Found");
                    qDebug() << "No Eye Cascade Found";
                    return false;
                }
                else */ //for some good use ;)
                    return true;
}

void CameraCapture::recapture(){
    // get new frame
    CVCapture >> CVFrame;
    // flip image by 180
    flip(CVFrame,CVFrame,1);
    // send frame to camera output
    emit signalRecapture(getFace());
}

void CameraCapture::deleteFaceCam(){
    // when we lost it, stop tracking it, switch to detection
    delete CVCapturedFace;
    CVMode = DETECTION;
}

Rect CameraCapture::newBoundingRect(RotatedRect &trackH){

    // find average rectangle in buffer
    quint16 rx = 0;
    quint16 ry = 0;
    quint16 rwidth = 0;
    quint16 rheight = 0;
    qint16  rangle = 0;
    quint8  rsize = averagePosition.size();

    foreach (RotatedRect r, averagePosition){
        rx += r.center.x;
        ry += r.center.y;
        rheight += r.size.height;
        rwidth += r.size.width;
        rangle += r.angle;
    }

    trackH.center.x = rx / rsize;
    trackH.center.y = ry / rsize;
    trackH.size.width = rwidth / rsize;
    trackH.size.height = rheight / rsize;
    trackH.angle = rangle / rsize;

    return trackH.boundingRect();
}

Rect CameraCapture::getApproxPosition(RotatedRect &trackH){
    // insert new rectangle in buffer or delete the oldest
    if (averagePosition.size() < avcountMax)
        averagePosition.push_back(trackH);
    else
        averagePosition.pop_front();

    return newBoundingRect(trackH);
}

void CameraCapture::paintImage(Rect &trackHRect, RotatedRect &trackH){

    // put ellipse and its bounding rect: where face is on the frame
    ellipse(CVFrame,trackH, Scalar(0,0,255), 3, CV_AA);
    rectangle(CVFrame,trackHRect, Scalar(0,0,255), 3, CV_AA);

   // putText(CVFrame, (CVMode == TRACKINGWITHKALMAN) ? "Kalman" : "CAMShift",
     //       Point(trackHRect.x + trackHRect.width, trackHRect.y), 1.5, 2,Scalar(0,0,255),1,CV_AA);
}

void CameraCapture::doCamshift(){

    TrackingNow.lock();

    RotatedRect trackH;

    // try to use camshift, if it fails - we get CAMError, so we lost our object
    try{
        trackH = CVCapturedFace->camTrack(CVFrame);
    }
    catch (CAMError c){
        //qDebug() << c.meaning();
        deleteFaceCam();
    }

    Rect trackHRect = getApproxPosition(trackH);

    //get the center of the face, calculated by getApproxPosition
    Point head;
    head.x = trackHRect.x + trackHRect.width / 2,
    head.y = trackHRect.y + trackHRect.height / 2;

    // if center is far enough, change average center
    if (abs(head.x - CVAverageHeadPos.x) + abs(head.y - CVAverageHeadPos.y) >= CVRadius)
        CVAverageHeadPos = head;

    // send coordinates to the monitor
    emit signalRedraw(QVariant(CVAverageHeadPos.x),QVariant(CVAverageHeadPos.y));

    // mark face position on the frame
    paintImage(trackHRect,trackH);

    TrackingNow.unlock();
}

void CameraCapture::doHaar(){
    vector<Rect>faces,profile;
    Mat CVGray;
    cvtColor(CVFrame,CVGray,CV_BGR2GRAY);

    /* find face by cascade; only one object can be found because of flag CV_HAAR_FIND_BIGGEST_OBJECT
     CV_HAAR_DO_CANNY_PRUNING for improvement capture */
    CVCascadeFace.detectMultiScale(CVGray,faces, 1.1, 10,CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_DO_CANNY_PRUNING);

  /*  foreach (Rect r, faces) {

        int x = min(max(r.x,0),CVCameraSize.x),
            y = min(max(r.y,0),CVCameraSize.y),
            w = min(r.width,CVCameraSize.x),
            h = min(r.height,CVCameraSize.y);

        if (x + w > CVCameraSize.x)
            w = CVCameraSize.x - x;
        if (y + h > CVCameraSize.y)
            h = CVCameraSize.y - y;

        rectangle(CVFrame, Point(x,y), Point(x + w,y + h),
                  Scalar(255, 0, 0), 2, 8, 0 );

        Mat faceNROI(CVFrame, Rect(x,y + h / 4,w,3 * h / 4)),   // let's assume that this is a straight face ;)
            faceEROI(CVFrame, Rect(x,y,w,h / 2));

        cvtColor(faceNROI,CVGray,CV_BGR2GRAY);
        cvtColor(faceEROI,CVGray,CV_BGR2GRAY);

        vector<Rect>nose,eye;

        CVCascadeEye.detectMultiScale(faceEROI,eye, 1.1, 5, CV_HAAR_SCALE_IMAGE
                                       |CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_DO_CANNY_PRUNING,
                                       Size(4,4));

        foreach (Rect re, eye) {

            rectangle(CVFrame, Point(x + re.x,y + re.y), Point(x + re.x + re.width,
                      y + re.y + re.height),Scalar(255, 255, 0), 2, 8, 0 );

        }

        CVCascadeNose.detectMultiScale(faceNROI,nose, 1.1, 3, CV_HAAR_SCALE_IMAGE
                                       |CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_DO_CANNY_PRUNING,
                                       Size(4,4));

        foreach (Rect rn, nose) {

            rectangle(CVFrame, Point(x + rn.x,y + rn.y), Point(x + rn.x + rn.width,
                      y + rn.y + rn.height),Scalar(0, 0, 255), 2, 8, 0 );

        }

    }*/
    // if we capture a face (or something, that resembles it)

    if (!faces.empty())
    {
        Rect face = faces.at(0);

        // now do some modification, so rect (where the face is) is in the boundaries of the frame
        face.x = min(max(face.x,0),CVFrameSize.width),
        face.y = min(max(face.y,0),CVFrameSize.height),
        face.width = min(face.width,CVFrameSize.width),
        face.height = min(face.height,CVFrameSize.height);

        if (face.x + face.width > CVFrameSize.width)
            face.width = CVFrameSize.width - face.x;

        if (face.x + face.height > CVFrameSize.height)
            face.height = CVFrameSize.height - face.y;

        // put this modified rectangle on the frame
        rectangle(CVFrame, Point(face.x,face.y), Point(face.x + face.width,face.y + face.height),
                  Scalar(255, 0, 0), 2, 8, 0 );

        // now we can track the face with CAMShift
        CVCapturedFace = new CapturedFace(CVFrame,face,CVFrameSize,vmin,vmax,smin);
        CVMode = TRACKING;
        doCamshift();
    }

}

QImage CameraCapture::getFace(){

    // if we have face for tracking - use CAMShift, otherwise use HaarCascades for detection
    if (CVMode == DETECTION)
        doHaar();
    else
       doCamshift();

    // if true, return image in HSV palette
    if (CVisHSV)
        cvtColor(CVFrame,CVFrame,CV_BGR2HSV);

    // convert from Mat (OpenCV) to Qt (QImage)
    return Mat2QImage(CVFrame);
}

QImage CameraCapture::Mat2QImage(Mat image){
    // 1 channel, for binary image
    if (image.type() == CV_8UC1) {
        QVector<QRgb> colorTable;
        for(quint16 i = 0; i < 256; i ++)
            colorTable.push_back(qRgb(i,i,i));
        const uchar *qImageBuffer = (const uchar*)image.data;
        QImage img(qImageBuffer,image.cols,image.rows,image.step,QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        return img;
    }

    // 3 channels, for BGR and HSV
    else {
        const uchar *qImageBuffer = (const uchar*)image.data;
        QImage img(qImageBuffer,image.cols,image.rows,image.step,QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    /* don't have other variants, only 1 or 3 channels per image */
}

void CameraCapture::keyRpressed(){
    // restart tracking
    TrackingNow.lock();

    if (CVMode != DETECTION)
        deleteFaceCam();

    TrackingNow.unlock();
}

void CameraCapture::keyBpressed(){
    // get backprojection on/off
    if (CVMode != DETECTION && CVisHSV && !CVCapturedFace->getCVBackProj())
            CVisHSV = false;
    CVCapturedFace->setCVBackProj();
}

void CameraCapture::keyKpressed(){
    // turn kalman on/off
    if (CVMode != DETECTION) {
        CVCapturedFace->setKalman();
        CVMode = (CVMode == TRACKING) ? TRACKINGWITHKALMAN : TRACKING;
    }
}

void CameraCapture::keyHpressed(){
    // get image in HSV on/off
    if (CVMode != DETECTION && !CVisHSV && CVCapturedFace->getCVBackProj())
        CVCapturedFace->setCVBackProj();
    CVisHSV = !CVisHSV;
}

void CameraCapture::sminchanged(int value){
    // set new min saturation value
    smin = value;
    if (CVMode != DETECTION)
        CVCapturedFace->setSmin(value);
}

void CameraCapture::vminchanged(int value){
    // set new min value value
    vmin = value;
    if (CVMode != DETECTION)
        CVCapturedFace->setVmin(value);
}

void CameraCapture::vmaxchanged(int value){
    // set new max value value
    vmax = value;
    if (CVMode != DETECTION)
        CVCapturedFace->setVmax(value);
}
