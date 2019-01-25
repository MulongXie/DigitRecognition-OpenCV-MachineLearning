#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui.h"
#include <iostream>
using namespace std;
using namespace cv;

int main() {
	VideoCapture cap(1);
	Mat img;
	Mat hsv;
	Mat gray;
	Mat result1;
	Mat result2;
	int thresh = 150;

	int lowc = 50;
	int highc = 150;

	namedWindow("control", 0);
	createTrackbar("thresh", "control", &thresh, 1000);
	createTrackbar("lowc", "control", &lowc, 1000);
	createTrackbar("highc", "control", &highc, 1000);

	while (1) {
		cap.read(img);    //Using video
		cvtColor(img, hsv, CV_RGB2HSV);
		cvtColor(img, gray, CV_RGB2GRAY);
		threshold(gray, result1, thresh, 255, THRESH_BINARY_INV);
		Canny(gray, result2, lowc, highc);
		imshow("camera1", result1);
		imshow("canny", result2);
		waitKey(30);
	}
}
/*变光照阈值调节*/