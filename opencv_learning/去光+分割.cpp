#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include "highgui.h"  
#include <stdlib.h>  
#include <stdio.h>

using namespace std;
using namespace cv;

static Scalar randomColor(RNG& rng)
{
	int icolor = (unsigned)rng;
	return Scalar(icolor & 255, (icolor >> 8) & 255, (icolor >> 16) & 255);
}

Mat divide(Mat img){
	Mat  label;
	int num = connectedComponents(img, label);
	Mat output = Mat::zeros(img.rows, img.cols, CV_8UC3);
	RNG rng(1234);
	for (int i = 1; i < num; i++){
		Mat mask = label == i;
		output.setTo(randomColor(rng), mask);
	}
	return output;
}


Mat removeLight(Mat img, Mat pattern, int method){
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

int main(){

	
	Mat spike = imread("spike.jpg");
	imshow("spike", spike);
	
	Mat pattern;		//分割操作之前去除背景光效果提升大幅	
	blur(spike, pattern, Size(spike.rows / 3, spike.cols / 3));
	Mat rem1;
	rem1 = removeLight(spike, pattern, 0);
	Mat rem2;
	rem2 = removeLight(spike, pattern, 1);
	imshow("rem1", rem1);

	Mat s1, s2;
	cvtColor(rem2, s1, CV_RGB2GRAY);
	int thresh = 10;
	namedWindow("control");
	createTrackbar("thresh", "control", &thresh, 255);
	while (1){
		threshold(s1, s2, thresh, 255, 0);		//分割操作必须对二值图进行
		Mat result = divide(s2);
		imshow("bin", s2);
		imshow("divide", result);
		waitKey(30);
	}
	system("pause");
	return 0;
	
}