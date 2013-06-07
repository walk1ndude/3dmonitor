#include <QtCore/QDebug>

#include "cameracapture.h"

#ifdef Q_OS_WIN32
    #define FACE_CASCADE_PATH "C:\opencv\data\haarcascades\haarcascade_frontalface_alt.xml"
#endif

#ifdef Q_OS_LINUX
    #define FACE_CASCADE_PATH "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml"
#endif

CameraCapture::CameraCapture(QObject *parent) : QObject(parent){

    cvisHSV = false;

    cvMode = DETECTION;

    cvAverageFacePos = Point(0,0);
    cvRadius = 7;

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
    cvCapture.release();
}

bool CameraCapture::initializeCV(Size cvFrameSize){

    if (!cvCapture.open(CV_CAP_ANY)){
            emit signalError("Can't open camera device");
            qDebug() << "Can't open camera device";
            return false;
        }

    this->cvFrameSize = cvFrameSize;

    // set camera resolution
    cvCapture.set(CV_CAP_PROP_FRAME_WIDTH, cvFrameSize.width);
    cvCapture.set(CV_CAP_PROP_FRAME_HEIGHT, cvFrameSize.height);

    if (!cvCascadeFace.load(FACE_CASCADE_PATH)){
            emit signalError("No Face Cascade Found");
            qDebug() << "No Face Cascade Found";
            return false;
        }
        else
            return true;
}

void CameraCapture::recapture(){
    // get new frame
    cvCapture >> cvFrame;
    // flip image by 180
    flip(cvFrame,cvFrame,1);
    // send frame to camera output
    emit signalRecapture(getFace());
}

void CameraCapture::deleteFaceCam(){
    // when we lost it, stop tracking it, switch to detection
    delete cvCapturedFace;
    cvMode = DETECTION;
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
    ellipse(cvFrame,trackH, Scalar(0,0,255), 3, CV_AA);
    rectangle(cvFrame,trackHRect, Scalar(0,0,255), 3, CV_AA);
}

void CameraCapture::doCamshift(){

    trackingNow.lock();

    RotatedRect trackH;

    // try to use camshift, if it fails - we get CAMError, so we lost our object
    try{
        trackH = cvCapturedFace->camTrack(cvFrame);
    }
    catch (CAMError c){
        //qDebug() << c.meaning();
        deleteFaceCam();
    }

    Rect trackHRect = getApproxPosition(trackH);

    //get the center of the face, calculated by getApproxPosition
    Point face;
    face.x = trackHRect.x + trackHRect.width / 2,
    face.y = trackHRect.y + trackHRect.height / 2;

    // if center is far enough, change average center
    if (abs(face.x - cvAverageFacePos.x) + abs(face.y - cvAverageFacePos.y) >= cvRadius)
        cvAverageFacePos = face;

    // send coordinates to the monitor
    emit signalRedraw(QVariant(cvAverageFacePos.x),QVariant(cvAverageFacePos.y));

    // mark face position on the frame
    paintImage(trackHRect,trackH);

    trackingNow.unlock();
}

void CameraCapture::doHaar(){
    vector<Rect>faces,profile;
    Mat cvGray;
    cvtColor(cvFrame,cvGray,CV_BGR2GRAY);

    /* find face by cascade; only one object can be found because of flag CV_HAAR_FIND_BIGGEST_OBJECT
     CV_HAAR_DO_CANNY_PRUNING for improvement capture */
    cvCascadeFace.detectMultiScale(cvGray,faces, 1.1, 10,CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_DO_CANNY_PRUNING);

    // if we capture a face (or something, that resembles it)

    if (!faces.empty())
    {
        Rect face = faces.at(0);

        // now do some modification, so rect (where the face is) is in the boundaries of the frame
        face.x = min(max(face.x,0),cvFrameSize.width),
        face.y = min(max(face.y,0),cvFrameSize.height),
        face.width = min(face.width,cvFrameSize.width),
        face.height = min(face.height,cvFrameSize.height);

        if (face.x + face.width > cvFrameSize.width)
            face.width = cvFrameSize.width - face.x;

        if (face.x + face.height > cvFrameSize.height)
            face.height = cvFrameSize.height - face.y;

        // put this modified rectangle on the frame
        rectangle(cvFrame, Point(face.x,face.y), Point(face.x + face.width,face.y + face.height),
                  Scalar(255, 0, 0), 2, 8, 0 );

        // now we can track the face with CAMShift

        cvCapturedFace = new CapturedFace(cvFrame,face,vmin,vmax,smin);
        cvMode = TRACKING;
    }

}

QImage CameraCapture::getFace(){

    // if we have face for tracking - use CAMShift, otherwise use HaarCascades for detection
    if (cvMode == DETECTION)
        doHaar();
    else
       doCamshift();

    // if true, return image in HSV palette
    if (cvisHSV)
        cvtColor(cvFrame,cvFrame,CV_BGR2HSV);

    // convert from Mat (OpenCV) to Qt (QImage)
    return mat2QImage(cvFrame);
}

QImage CameraCapture::mat2QImage(Mat image){
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
    trackingNow.lock();

    if (cvMode != DETECTION)
        deleteFaceCam();

    trackingNow.unlock();
}

void CameraCapture::keyBpressed(){
    // get backprojection on/off
    if (cvMode != DETECTION && cvisHSV && !cvCapturedFace->getCVBackProj())
            cvisHSV = false;
    cvCapturedFace->setCVBackProj();
}

void CameraCapture::keyKpressed(){
    // turn kalman on/off
    if (cvMode != DETECTION) {
        cvCapturedFace->setKalman();
        cvMode = (cvMode == TRACKING) ? TRACKINGWITHKALMAN : TRACKING;
    }
}

void CameraCapture::keyHpressed(){
    // get image in HSV on/off
    if (cvMode != DETECTION && !cvisHSV && cvCapturedFace->getCVBackProj())
        cvCapturedFace->setCVBackProj();
    cvisHSV = !cvisHSV;
}

void CameraCapture::sminchanged(int value){
    // set new min saturation value
    smin = value;
    if (cvMode != DETECTION)
        cvCapturedFace->setSmin(value);
}

void CameraCapture::vminchanged(int value){
    // set new min value value
    vmin = value;
    if (cvMode != DETECTION)
        cvCapturedFace->setVmin(value);
}

void CameraCapture::vmaxchanged(int value){
    // set new max value value
    vmax = value;
    if (cvMode != DETECTION)
        cvCapturedFace->setVmax(value);
}
