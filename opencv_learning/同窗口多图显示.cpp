// sift_test.cpp : 定义控制台应用程序的入口点。

#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"//因为在属性中已经配置了opencv等目录，所以把其当成了本地目录一样
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace cv;
using namespace std;


int main(int argc, char* argv[])
{
	Mat img1 = imread("1.jpg");
	resize(img1, img1, Size(600, 600));
	Mat img2 = imread("2.jpg");
	resize(img2, img2, Size(600, 600));
	Mat img3 = imread("2017.png");
	resize(img3, img3, Size(600, 600));
	Mat img4 = imread("2018.png");
	resize(img4, img4, Size(600, 600));
	namedWindow("select");
	int flag = 0;
	createTrackbar("control", "select", &flag, 4);
	while (1) {
		switch (flag) {
		case 0:imshow("img", img1); break;
		case 1:imshow("img", img2); break;
		case 2:imshow("img", img3); break;
		case 3:imshow("img", img4); break;
		}
		waitKey(30);
	}
}