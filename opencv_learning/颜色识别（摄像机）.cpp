#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui.h"
#include <vector>
#include <iostream>
using namespace std;
using namespace cv;

int main() {
	VideoCapture cap(1);
	Mat img;
	int wid = 520;
	int hei = 310;
	while (1) {
		cap.read(img);
		vector<Mat> bgr;
//		cvtColor(img, img, CV_RGB2HSV);
		blur(img, img, Size(50, 50));
		split(img, bgr);

		Mat histb, histg, histr;
		int channel[] = { 0 };
		int histsize[] = { 256 };
		float h[] = { 0,256 };
		const float * ranges[] = { h };
		calcHist(&bgr[0], 1, channel, Mat(), histb, 1, histsize, ranges);
		calcHist(&bgr[1], 1, channel, Mat(), histg, 1, histsize, ranges);
		calcHist(&bgr[2], 1, channel, Mat(), histr, 1, histsize, ranges);

		normalize(histb, histb, 0, hei / 2, NORM_MINMAX);
		normalize(histg, histg, 0, hei / 2, NORM_MINMAX);
		normalize(histr, histr, 0, hei / 2, NORM_MINMAX);
		int bin = cvRound(wid / histsize[0]);
		Mat draw(hei, wid, CV_8UC3, Scalar::all(0));
		for (int i = 1; i < histsize[0]; i++) {
			line(draw, Point(bin*(i - 1), hei - cvRound(histb.at<float>(i - 1))),
				Point(bin*i, hei - cvRound(histb.at<float>(i))), Scalar(255, 0, 0));
			line(draw, Point(bin*(i - 1), hei - cvRound(histg.at<float>(i - 1))),
				Point(bin*i, hei - cvRound(histg.at<float>(i))), Scalar(0, 255, 0));
			line(draw, Point(bin*(i - 1), hei - cvRound(histr.at<float>(i - 1))),
				Point(bin*i, hei - cvRound(histr.at<float>(i))), Scalar(0, 0, 255));
		}

		vector<Mat> hists;
		split(draw, hists);
		

		int flag = 0;
		for (int i = 1; i < hei; i++) {
			for (int j = wid - 1; j > 0; j--) {
				for (int k = 0; k < 3; k++)
					if (draw.at<Vec3b>(i, j)[k] != 0) {
						if (k == 0) {
							std::cout << i << "," << j << " 蓝色" << endl;
							flag = 1;
							break;
						}
						if (k == 1) {
							std::cout << i << "," << j << " 绿色" << endl;
							flag = 1;
							break;
						}
						if (k == 2) {
							std::cout << i << "," << j << " 红色" << endl;
							flag = 1;
							break;
						}
					}
				if (flag)break;
			}
			if (flag)break;
		}
		imshow("hist", draw);
		imshow("蓝色", hists[0]);
		imshow("绿色", hists[1]);
		imshow("红色", hists[2]);
		imshow("camera", img);
		waitKey(30);
	}
	system("pause");
}