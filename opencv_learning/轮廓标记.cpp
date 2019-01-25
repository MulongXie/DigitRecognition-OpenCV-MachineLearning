#include "opencv2\core\core.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include <iostream>
using namespace std;
using namespace cv;

void draw(Mat & img, vector<vector<Point>> contours, int i) {
	Point2f v[4];
	RotatedRect mr = minAreaRect(Mat(contours[i]));
	mr.points(v);
	for (int j = 0; j < 4; j++)
		line(img, v[j], v[(j + 1) % 4], Scalar(0, 255, 0), 2);//用矩形标出物体轮廓线
}

int main() {
	Mat img = imread("contour.png");
	Mat img_gray;
	cvtColor(img, img_gray, CV_BGR2GRAY);
	Mat thresh_img;
	threshold(img_gray, thresh_img, 180, 255, THRESH_BINARY_INV);//二值化
	imshow("thresh", thresh_img);

	vector<Vec4i>hier;
	vector<vector<Point>> contours;
	findContours(thresh_img, contours, hier, RETR_TREE, CV_CHAINTREE_APPROX_NONE);


	Mat mask = Mat::zeros(img.rows, img.cols, CV_8UC1);
	for (int i = 0; i < contours.size() - 1; i++) {
		draw(img, contours, i);//用矩形标出物体轮廓线

		drawContours(mask, contours, i, Scalar(255));//画出物体边缘轮廓（类似边缘检测）
		Scalar area_s = sum(mask);
		float area = area_s[0];

		//		printf("area:%f  x:%f y:%f\n", area, mr.center.x, mr.center.y);
	}

	imshow("mask", mask);
	imshow("hie", hier);
	printf("number:%d\n size:%d\n", contours.size(), hier.size());
	for (int i = 0; i < hier.size(); i++)printf("index:%d  %d %d %d %d\n", i, hier[i][0], hier[i][1], hier[i][2], hier[i][3]);

//	draw(img, contours, 1);//画出某个contour的矩形标记

	imshow("pic", img);
	waitKey(0);
	system("pause");
}