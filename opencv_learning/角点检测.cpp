#include "opencv2\core\core.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\opencv.hpp"
#include "opencv2\ml\ml.hpp"
#include <iostream>
using namespace std;
using namespace cv;


int main() {
	Mat img = imread("cnum46.png");
	Mat img_gray;
	cvtColor(img, img_gray, CV_BGR2GRAY);
	Mat thresh_img;
	threshold(img_gray, thresh_img, 180, 255, THRESH_BINARY_INV);
	imshow("thresh", thresh_img);

	vector<Point> mask;
	int a=10;	//0.1
	int b = 10;		//10
	namedWindow("control");
	createTrackbar("a", "control", &a, 50);
	createTrackbar("b", "control", &b, 10);

	while (1) {
		goodFeaturesToTrack(img_gray, mask, 200, a/100.0, b);

		for (int i = 0; i < mask.size(); i++) {
			circle(img, mask[i], 5, Scalar(255), 2);
		}
		imshow("pic", img);
		waitKey(30);
	}

	system("pause");
}