#include "capturedface.h"

// number of bins for the histogram, for hue and for saturation
const int binsH = 32, binsS = 32,
          histSize[] = {binsH, binsS};

// hue and saturation ranges
const float rangeH[] = {0, 180},
            rangeS[] = {0, 255};
const float* histRanges[] = {rangeH,rangeS};

// channels in the histogram
const int channels[] = {0, 1};

CAMError::CAMError(){

}

CAMError::~CAMError(){

}

const char *CAMError::meaning(){
    return "Wrong SearchWindow position!";
}

CapturedFace::CapturedFace(Mat &frame, Rect face, Point cameraSize, int vminv, int vmaxv, int sminv)
{
    // set CAMShift initial parameters
    cvFrameSize = cameraSize;
    vmin = vminv;
    vmax = vmaxv;
    smin = sminv;
    smax = 250;

    cvisBackProj = false;
    cvKalmanEnabled = false;

    w = frame.size().width;
    h = frame.size().height;

    hue.create(h,w,CV_8UC1);
    sat.create(h,w,CV_8UC1);

    // initial search window is the rectangle detected by Haar, where face is
    searchWindow = face;

    updateImage(frame);
    kalmanInit(face);

}

void CapturedFace::kalmanInit(Rect face){

    kF.init(11,5,0);

    /* transition matrix:
    11 state: X, Y, Width, Height, Angle, dX, dY, ddX, ddY, dAngle, ddAngle
    5 measurement: X, Y, Width, Height, Angle
    */

    kF.transitionMatrix = *(Mat_<float>(11,11) << 1, 0, 0, 0, 0, 1, 0, 0.5,   0, 0,   0,
                                                  0, 1, 0, 0, 0, 0, 1,   0, 0.5, 0,   0,
                                                  0, 0, 1, 0, 0, 0, 0,   0,   0, 0,   0,
                                                  0, 0, 0, 1, 0, 0, 0,   0,   0, 0,   0,
                                                  0, 0, 0, 0, 1, 0, 0,   1,   0, 0, 0.5,
                                                  0, 0, 0, 0, 0, 1, 0,   1,   0, 0,   0,
                                                  0, 0, 0, 0, 0, 0, 1,   0,   1, 0,   0,
                                                  0, 0, 0, 0, 0, 0, 0,   1,   0, 0,   0,
                                                  0, 0, 0, 0, 0, 0, 0,   0,   1, 0,   0,
                                                  0, 0, 0, 0, 0, 0, 0,   0,   0, 1,   1,
                                                  0, 0, 0, 0, 0, 0, 0,   0,   0, 0,   1);
    measurement = Mat_<float>(5,1);
    measurement.setTo(Scalar(0));

    // set initial position of the head: X, Y, W, H, Angle (0, because Haar cascade doesn't work with angles)
    kF.statePre.at<float>(0) = face.x + face.width / 2;
    kF.statePre.at<float>(1) = face.y + face.height / 2;
    kF.statePre.at<float>(2) = face.width;
    kF.statePre.at<float>(3) = face.height;
    kF.statePre.at<float>(4) = 0;

    kF.statePre.at<float>(5) = 0;
    kF.statePre.at<float>(6) = 0;
    kF.statePre.at<float>(7) = 0;
    kF.statePre.at<float>(8) = 0;
    kF.statePre.at<float>(9) = 0;
    kF.statePre.at<float>(10) = 0;

    // initialize Kalman filter matrices
    setIdentity(kF.measurementMatrix);
    setIdentity(kF.processNoiseCov, Scalar::all(1e-5));
    setIdentity(kF.measurementNoiseCov, Scalar::all(1e-2));
    setIdentity(kF.errorCovPost, Scalar::all(.1));

}

void CapturedFace::updateImage(const Mat& frame) {

  // get hue and saturation channels, also limit value and saturation values if image for example too bright
  cvtColor(frame,hsv,CV_BGR2HSV);

  inRange(hsv,Scalar(0, MIN(smin,smax), MIN(vmin, vmax)), //lower bound
          Scalar(180, MAX(smin,smax), MAX(vmin, vmax)), //upper bound
          mask); //destination
  const int ch[] = {0, 0, 1, 0};
  Mat out[] = {hue, sat};

  mixChannels(&hsv,1,out,2,ch,2);

}

void CapturedFace::recalcHist(){
    // recalculate histogram of the tracking head based on hue and saturation channels
    Mat roi_hue(hue,searchWindow),
        roi_sat(sat,searchWindow),
        roi_mask(mask,searchWindow),
        input[] = {roi_hue, roi_sat};

    calcHist(input,2,channels,roi_mask,hist,2,histSize,histRanges);
}

bool CapturedFace::checkSearchWindow(){
    // check if the search window is in the boundaries of the frame
    return searchWindow.x >= 0 && searchWindow.y >= 0 &&
            searchWindow.x + searchWindow.width <= cvFrameSize.x &&
            searchWindow.height <= cvFrameSize.y && searchWindow.width > 0
            && searchWindow.height > 0;
}

RotatedRect CapturedFace::kalmanPrediction(RotatedRect &trackBox){
    // feed data recieved by CAMShift to the Kalman filter
    measurement(0) = trackBox.center.x;
    measurement(1) = trackBox.center.y;
    measurement(2) = trackBox.size.width;
    measurement(3) = trackBox.size.height;
    measurement(4) = trackBox.angle;

    // use the filter, make correction to the head position
    Mat estimated = kF.correct(measurement);

    return RotatedRect(Point(estimated.at<float>(0),estimated.at<float>(1)),
                       Size(estimated.at<float>(2),estimated.at<float>(3)),
                       estimated.at<float>(4));
}

RotatedRect CapturedFace::camTrack(Mat& frame){

    updateImage(frame);
    recalcHist();

    prediction = kF.predict();

    Mat input[] = {hue, sat};

    // calculate backprojection of the frame
    calcBackProject(input,2,channels,hist,backproj,histRanges);
    backproj &= mask;
    threshold(backproj,backproj,smin,255, THRESH_BINARY);

    medianBlur(frame,frame,3);

    // set frame to backproject if we want to see it (user pressed key B)
    frame = (cvisBackProj) ? backproj : frame;

    // get head parameters by CAMShift
    RotatedRect trackBox = CamShift(backproj, searchWindow,
                                    TermCriteria(TermCriteria::COUNT|TermCriteria::EPS, 20, 0.01));

    // if window is out of the frame
    if (!checkSearchWindow())
        throw CAMError();

    // return head parameters based on Kalman filter or on CAMShift only
    return (cvKalmanEnabled) ? kalmanPrediction(trackBox) : trackBox;
}

void CapturedFace::setSmin(int value){
    smin = value;
}

void CapturedFace::setVmin(int value){
    vmin = value;
}

void CapturedFace::setVmax(int value){
    vmax = value;
}

void CapturedFace::setCVBackProj(){
    cvisBackProj = !cvisBackProj;
}

bool CapturedFace::getCVBackProj(){
    return cvisBackProj;
}

void CapturedFace::setKalman(){
    cvKalmanEnabled = !cvKalmanEnabled;
}
