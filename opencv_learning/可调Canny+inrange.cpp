#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui.h"
#include <iostream>
using namespace std;
using namespace cv;

int main(){		
    VideoCapture cap(1);
	Mat img;
	Mat hsv;
	Mat gray;
	Mat result1;
	Mat result2;
	int lowh = 55;
	int lows = 55;
	int lowv = 55;
	int highh = 155; 
	int highs = 155;
	int highv = 155;
	
	int lowc = 50;
	int highc = 150;

	namedWindow("control",0);
	namedWindow("controlc");
	createTrackbar("lowh", "control", &lowh, 180);
	createTrackbar("highh", "control", &highh, 180);
	createTrackbar("lows", "control", &lows, 256);
	createTrackbar("highs", "control", &highs, 256);
	createTrackbar("lowv", "control", &lowv, 256);
	createTrackbar("highv", "control", &highv, 256);

	createTrackbar("lowc", "controlc", &lowc, 255);
	createTrackbar("highc", "controlc",&highc, 255);

	while (1){
		cap.read(img);    //Using video
		cvtColor(img, hsv, CV_RGB2HSV);
		cvtColor(img, gray, CV_RGB2GRAY);
		inRange(hsv, Scalar(lowh, lows, lowv), Scalar(highh, highs, highv), result1);
		Canny(gray, result2, lowc, highc);
		imshow("camera1", result1);
		imshow("canny", result2);
		waitKey(30);
	}
}