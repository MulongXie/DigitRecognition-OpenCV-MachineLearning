#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace cv;

int main(){
	Mat img = imread("bbb.jpg");
	Mat gray;
	Mat edge;
	cvtColor(img, gray, CV_RGB2GRAY);
	Canny(gray, edge, 50, 150);
	
	int thresh = 150;
	int min = 50;
	int max = 10;
	namedWindow("control");
	createTrackbar("threshold", "control", &thresh, 500);
	createTrackbar("min", "control", &min, 500);
	createTrackbar("max", "control", &max, 500);
	while (1)					
	{			
		vector<Vec2f>lines;
		HoughLines(edge, lines, 1, CV_PI / 180, thresh);		//标准霍夫变换
		Mat draw1;
		cvtColor(edge, draw1, CV_GRAY2BGR);
		for (int i = 0; i < lines.size(); i++){
			float rho = lines[i][0], theta = lines[i][1];
			Point p1, p2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			p1.x = cvRound(x0 + 1000 * (-b));
			p1.y = cvRound(y0 + 1000 * a);
			p2.x = cvRound(x0 - 1000 * (-b));
			p2.y = cvRound(y0 - 1000 * a);
			line(draw1, p1, p2, Scalar(0, 0, 255), 2);
		}


		vector<Vec4f> lines2;			//概率霍夫变换	
		HoughLinesP(edge, lines2, 1, CV_PI / 180, thresh, min, max);
		Mat draw2;
		cvtColor(edge, draw2, CV_GRAY2BGR);
		for (int i = 0; i < lines2.size(); i++){
			Vec4f l = lines2[i];
			line(draw2, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255));
		}

		imshow("orign", img);
		imshow("edge", edge);
		imshow("result1", draw1);
		imshow("result2", draw2); 
		waitKey(30);
	}


	std::system("pause");
}
