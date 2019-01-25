#include"opencv2/highgui/highgui.hpp"
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include<iostream>
#include<vector>
using namespace cv;
using namespace std;

int main() {
	VideoCapture cap(1);
	int lowc = 50;
	int highc = 150;
	int lowh = 55;
	int lows = 55;
	int highh = 155;
	namedWindow("control");
	createTrackbar("lowc", "control", &lowc, 255);
	createTrackbar("highc", "control", &highc, 255);

	createTrackbar("lowh", "control", &lowh, 180);
	createTrackbar("highh", "control", &highh, 180);
	createTrackbar("lows", "control", &lows, 256);
	while (1) {
		Mat img;
		cap.read(img);
		Mat can1;
		Canny(img, can1, 100, 150);
		imshow("canny1", can1);
		Mat hsv;
		cvtColor(img, hsv, CV_RGB2HSV);
		vector<Mat> hsvs;
		split(hsv, hsvs);
		for (int i = 1; i < img.rows; i++)
			for (int j = 1; j < img.cols; j++) {
				if (hsvs[0].at<uchar>(i, j) < lowh || hsvs[0].at<uchar>(i, j) > highh  || hsvs[1].at<uchar>(i,j)<lows) {
					img.at<Vec3b>(i, j)[0] = 255;
					img.at<Vec3b>(i, j)[1] = 255;
					img.at<Vec3b>(i, j)[2] = 255;
				}
			}
		Mat bin;
		Mat can;
		cvtColor(img, bin, CV_RGB2GRAY);
		Canny(bin, can, lowc, highc);

		imshow("result", img);
		imshow("canny", can);
		imshow("hsv", hsv);
		waitKey(10);
	}
	system("pause");
}