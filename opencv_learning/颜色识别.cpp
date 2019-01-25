#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui.h"
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;

int main(){
	int wid = 520, hgt = 310;
//	Mat img = imread("aaa.jpg");
	Mat img(hgt, wid, CV_8UC3, Scalar(255, 0, 0));
	vector<Mat> bgr;
	split(img, bgr);
		
	Mat histb, histg, histr;		//step1 计算直方图
	int channel[] = { 0 };
	int histsize[] = { 256 };
	float h[] = { 0, 256 };
	const float * ranges[] = { h };
	calcHist(&bgr[0], 1, channel, Mat(), histb, 1, histsize, ranges);
	calcHist(&bgr[1], 1, channel, Mat(), histg, 1, histsize, ranges);
	calcHist(&bgr[2], 1, channel, Mat(), histr, 1, histsize, ranges);

	normalize(histb, histb, 0, hgt/2, NORM_MINMAX);
	normalize(histg, histg, 0, hgt/2, NORM_MINMAX);
	normalize(histr, histr, 0, hgt/2, NORM_MINMAX);
	int bin = cvRound((float)wid / (float)histsize[0]);
	Mat draw1(hgt, wid, CV_8UC3, Scalar::all(0));
	for (int i = 1; i < histsize[0]; i++){
		line(draw1, Point(bin*(i - 1), hgt - cvRound(histb.at<float>(i - 1))),
			Point(bin*i, hgt - cvRound(histb.at<float>(i))), Scalar(255, 0, 0), 1);
		line(draw1, Point(bin*(i - 1), hgt - cvRound(histg.at<float>(i - 1))),
			Point(bin*i, hgt - cvRound(histg.at<float>(i))), Scalar(0, 255, 0), 1);
		line(draw1, Point(bin*(i - 1), hgt - cvRound(histr.at<float>(i - 1))),
			Point(bin*i, hgt - cvRound(histr.at<float>(i))), Scalar(0, 0, 255), 1);
	}
	
	int flag = 0;		//step2 根据直方图判断颜色
	for (int i = 1; i < hei; i++){
		for (int j = wid-2; j > 0; j--){
			for (int k = 0; k < 3; k++){
				if (draw1.at<Vec3b>(i, j)[k] != 0){
					if (k == 0){
						cout << i << "," << j << "蓝色" << endl;
						flag = !flag;
						break;
					}
					if (k == 1){
						cout << i << "," << j << "绿色" << endl;
						flag = !flag;
						break;
					}
					if (k == 2){
						cout << i << "," << j << "红色" << endl;
						flag = !flag;
						break;
					}
				}
			}
			if (flag)break;
		}
		if (flag)break;
	}

	
	imshow("result1", draw1);
	imshow("result2", draw2);



	waitKey(0);
	system("pause");
}
