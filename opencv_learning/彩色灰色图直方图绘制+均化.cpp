#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;

int main(){
	Mat img = imread("aaa.jpg");

	int width = 512;
	int height = 300;	//定义输出窗口


//	rectangle(img, Point(100, 100), Point(200, 200), Scalar(255, 0, 0), 2, 8, 0);    //在图上画线框

	vector<Mat>bgr;					//*****彩色图*****
	split(img, bgr);

	const int channel[1] = { 0 };
	const int histsize[] = { 256 };
	float h[] = { 0, 256 };
	const float * ranges = { h };    //参数设置

	Mat b_hist, g_hist, r_hist;
	calcHist(&bgr[0], 1, channel, Mat(), b_hist, 1, histsize, &ranges);
	calcHist(&bgr[1], 1, channel, Mat(), g_hist, 1, histsize, &ranges);
	calcHist(&bgr[2], 1, channel, Mat(), r_hist, 1, histsize, &ranges);

	normalize(b_hist, b_hist, 0, height, NORM_MINMAX);    //归一化（限制y轴范围在0到height）
	normalize(g_hist, g_hist, 0, height, NORM_MINMAX);    
	normalize(r_hist, r_hist, 0, height, NORM_MINMAX);    

	int bin = cvRound((float)width / (float)histsize[0]);	//计算每个单位区间宽度
	
	Mat histimgb(height, width, CV_8UC3, Scalar::all(0));	//画板
	Mat histimgg(height, width, CV_8UC3, Scalar::all(0));
	Mat histimgr(height, width, CV_8UC3, Scalar::all(0));
	for (int i = 1; i < histsize[0]; i++){		
		line(histimgb, Point(bin*(i - 1), height - cvRound(b_hist.at<float>(i - 1))),
			Point(bin*i, height - cvRound(b_hist.at<float>(i))),Scalar(255,0,0));
		line(histimgg, Point(bin*(i - 1), height - cvRound(g_hist.at<float>(i - 1))),
			Point(bin*i, height - cvRound(g_hist.at<float>(i))), Scalar(0, 255, 0));
		line(histimgr, Point(bin*(i - 1), height - cvRound(r_hist.at<float>(i - 1))),
			Point(bin*i, height - cvRound(r_hist.at<float>(i))), Scalar(0, 0, 255));

	}	  //画线

	
	imshow("orign", img);
	imshow("imgb", histimgb);
	imshow("imgg", histimgg);
	imshow("imgr", histimgr);
	
	waitKey(0);

/*	Mat gray;		//*****灰度图******
	cvtColor(img, gray, CV_RGB2GRAY);
	int channel[] = { 0 };
	int histsize[] = { 256 };
	float h[] = { 0, 256 };
	const float *ranges = { h };    //ranges必须要加const

	Mat hist;
	calcHist(&gray, 1, channel, Mat(), hist, 1, histsize, &ranges);
	Mat histimg(height,width,CV_8UC3,Scalar::all(0));
	normalize(hist, hist, 0, 256, NORM_MINMAX);
	int bin = cvRound((float)width / (float)histsize[0]);
	for (int i = 1; i < histsize[0]; i++){
		line(histimg, Point(bin*(i - 1), height - cvRound(hist.at<float>(i - 1))),
			Point(bin*i, height - cvRound(hist.at<float>(i))), Scalar(255, 255, 255));
	}
	namedWindow("show", 0);
	namedWindow("hist", 0);
	imshow("show", gray);
	imshow("hist", histimg);



	equalizeHist(gray, gray);    //*****直方图均化*****
	calcHist(&gray, 1, channel, Mat(), hist, 1, histsize, &ranges);
	Mat histimg2(height, width, CV_8UC3, Scalar::all(0));
	normalize(hist, hist, 0, 256, NORM_MINMAX);
	for (int i = 1; i < histsize[0]; i++){
		line(histimg2, Point(bin*(i - 1), height - cvRound(hist.at<float>(i - 1))),
			Point(bin*i, height - cvRound(hist.at<float>(i))), Scalar(255, 255, 255));
	}
	imshow("show2", gray);
	imshow("hist2", histimg2);
*/

	system("pause");
}