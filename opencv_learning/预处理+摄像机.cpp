#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui.h"
#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace cv;

Mat removeLight(Mat img, Mat pattern,int method){    //去光
	Mat aux;
	if (method == 1){
		Mat img32, pattern32;
		img.convertTo(img32, CV_32F);
		pattern.convertTo(pattern32, CV_32F);
		aux = 1 - (img32 / pattern32);
		aux = aux * 255;
		aux.convertTo(aux, CV_8U);
	}
	else{
		aux = pattern - img;
	}
	return aux;
}


static Scalar randomColor(RNG& rng)
{
	int icolor = (unsigned)rng;
	return Scalar(icolor & 255, (icolor >> 8) & 255, (icolor >> 16) & 255);
}
Mat connected(Mat img){    //分割
	Mat label;
	int num = connectedComponents(img, label);
	std::cout << num;
	Mat output = Mat::zeros(img.rows, img.cols, CV_8UC3);
	RNG rng(1234);
	for (int i = 1; i < num; i++){
		Mat mask = label == i;
		output.setTo(randomColor(rng), mask);
	}
	return output;
}


Mat contours(Mat img){			//轮廓处理
	vector<vector<Point>>contours;
	vector<Vec4i>hie;
	findContours(img, contours, hie, RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	Mat draw = Mat::zeros(img.size(), CV_8UC3);
	RNG rng(1234);
	for (int i = 0; i < contours.size(); i++){
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(draw, contours, i, color);
	}
	imshow("out", draw);
	return draw;
}


int main(){
	Mat spike = imread("aaa.jpg");
	cvtColor(spike, spike, CV_RGB2GRAY);
	
	Mat pattern;
	blur(spike, pattern, Size(spike.cols / 3, spike.cols / 3));  
	Mat rml;
	rml=removeLight(spike, pattern,1);
	imshow("removelight", rml);  //去光

	medianBlur(rml, rml, 3);              
	GaussianBlur(rml, rml, Size(3,3), 0, 0);  //滤波

	Mat divide;
	divide = connected(rml);
	imshow("divide", divide);  //分割 处理图像必须是二值图

	Mat bin;
	Canny(spike,bin,10,20);
	Mat contour=contours(bin);  //绘制轮廓 处理图必须是二值图

	int low = 1;
	int high = 50;
	namedWindow("controll");
	cvCreateTrackbar("low", "controll", &low, 255);
	cvCreateTrackbar("high", "controll", &high, 255);  //外部手动控制
	
	Mat canny;
	while (1){
		Canny(rml, canny, low, high);
		imshow("controll", canny);
		waitKey(30);
	}   //边缘

/*
    VideoCapture cap(0);
    if(!cap.isOpened()){
        return -1;
    }
    Mat frame;
    while(1){
        cap.read(frame);
        imshow("video",frame);
        if(waitKey(30)==0)
            break;
    }   //video
*/
	waitKey(0);
	return 0;
}