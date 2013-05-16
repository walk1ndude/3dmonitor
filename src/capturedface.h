#ifndef CAPTUREDFACE_H
#define CAPTUREDFACE_H

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

class CAMError
{
public:
    CAMError();
    ~CAMError();
    const char *meaning();
};

class CapturedFace
{
public:
   CapturedFace(Mat& frame, Rect face, Point cameraSize, int vminv, int vmaxv, int sminv);
   RotatedRect camTrack(Mat& frame);

   void setSmin(int);
   void setVmin(int);
   void setVmax(int);
   void setCVBackProj();
   bool getCVBackProj();
   void setKalman();

private:
   int vmin;
   int vmax;
   int smin;
   int smax;
   int binsH;
   int binsS;
   int w;
   int h;
   bool cvisBackProj;
   bool cvKalmanEnabled;
   Point cvFrameSize;
   Mat frame;
   Mat hsv;
   Mat hue;
   Mat sat;
   Mat mask;
   Mat prob;
   Mat prediction;
   Mat state;
   Mat processNoise;
   Mat_<float> measurement;
   MatND hist, backproj;
   Rect searchWindow;
   KalmanFilter kF;

   bool checkSearchWindow();
   void recalcHist();

   void kalmanInit(Rect Face);
   RotatedRect kalmanPrediction(RotatedRect &trackBox);

   void updateImage(const Mat& image);

};

#endif // CAPTUREDFACE_H
