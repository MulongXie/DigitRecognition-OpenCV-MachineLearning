#include "opencv2\core\core.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\opencv.hpp"
#include "opencv2\ml\ml.hpp"
#include <iostream>
using namespace std;
using namespace cv;

//typedef int ints[20];

vector<Mat>number;//存储各分离数字
vector<float>aeras;//存储区域大小
vector<int>wholes;//存储洞的个数
vector<vector<float>> pro;//存储x方向的投影(可用）

vector<int> border;//储存边界特征
vector<int> rborder;
vector<int> sub;

FILE*f1 = fopen("result.txt", "w");
FILE*f2 = fopen("result2.txt", "w");

int flag = 0;

//物体分离
void divide(Mat & img, vector<vector<Point>> contours, int i) {

	Point2f v[4];//用矩形标出物体轮廓线
	Rect mr = boundingRect(Mat(contours[i]));
	v[0] = mr.tl();
	v[1].x = mr.tl().x;
	v[1].y = mr.br().y;
	v[2] = mr.br();
	v[3].x = mr.br().x;
	v[3].y = mr.tl().y;

	/*	for (int j = 0; j < 4; j++)
	line(img, v[j], v[(j + 1) % 4], Scalar(255), 2);*/

	Mat show;//分离各特征物体
	show = img(mr);
	resize(show, show, Size(50, 96));
	number.push_back(show);
	string s = to_string(i);

	//	imshow(s, show);
}

//投影特征
void projection(int x) {
	//x-direction
	Mat img = number[x];
	int mark[20];
	int k = 0;
	for (float i = 1; i < img.rows; i += img.rows / 20.0) {
		printf("%d ", cvRound(i));
		int row = cvRound(i);
		int count = 0;
		for (int j = 1; j < img.cols; j++) {
			if (img.at<uchar>(row, j) != 0) count++;
		}

		float c = count * 10.0 / img.cols;

		pro[x].push_back(c);
	}
	printf("\n");
	for (int i = 0; i < 20; i++)printf("%.3f\n", pro[x][i]);
}

//画出直方图
Mat hist(vector<int> x) {
	Mat draw(150, 405, CV_8UC1, Scalar(0));
	int j = 1;
	int bin;
	bin = cvRound(500.0 / x.size());
	printf("%d\n", bin);
	for (int i = bin; i < 400; i += bin) {
		line(draw, Point(i - bin, 150 - 5 * cvRound(x[j - 1])), Point(i, 150 - 5 * cvRound(x[j])), Scalar(255));
		printf("%d %d\n", i, j);
		j++;
	}

	string s = to_string(flag);
	imshow("hist" + s, draw);
	flag++;
	return draw;
}

//边界特征
void borders(Mat img) {
	int i = 0;
	border.clear();
	rborder.clear();
	sub.clear();
	while (i < img.rows) {	//分成48块计算左右边界
		for (int j = 0; j < img.cols; j++)
			if (img.at<uchar>(i, j)) {
				border.push_back(j);
				break;
			}
		for (int j = img.cols - 1; j >= 0; j--)
			if (img.at<uchar>(i, j)) {
				rborder.push_back(img.cols - j);
				break;
			}
		i += img.rows / 48;
	}

	/*	border.push_back(-1000);
	rborder.push_back(-1000);*/

	for (int k = 0; k < rborder.size(); k++) border.push_back(rborder[k]);	//左右边界拼接

	for (int k = 0; k < border.size() - 1; k++) {	//计算拓扑数列
		int a = border[k + 1] - border[k];
		sub.push_back(a);
	}

	hist(sub);	//直方图展示
}

int main() {
	Mat img = imread("cnum1.png");
	Mat img_gray;
	cvtColor(img, img_gray, CV_BGR2GRAY);
	Mat thresh_img;
	threshold(img_gray, thresh_img, 180, 255, THRESH_BINARY_INV);
	imshow("thresh", thresh_img);

	/****************object divide******************/
	vector<Vec4i>hier;
	vector<vector<Point>> contours;
	findContours(thresh_img, contours, hier, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	Mat mask(img.size(), CV_8UC1, Scalar::all(0));
	for (int i = 0; i < contours.size(); i++) {
		drawContours(mask, contours, i, Scalar(255));
		divide(thresh_img, contours, i);
	}


	/***************feature extract*******************/

	//1.面积孔洞特征
	/*	int whole = 0;
	for (int i = 0; i < number.size(); i++) {
	Scalar area_s = sum(mask);
	aeras.push_back(area_s[0]);

	if (hier[i][3] != -1) {	//洞的个数
	whole++;
	wholes.push_back(hier[i][3]);
	}
	}
	*/

	//2.角点计算
	/*	for (int i = 0; i < number.size(); i++) {
	vector<Point> mask;
	goodFeaturesToTrack(number[i], mask, 10, 0.2, 10);
	for (int x = 0; x < mask.size(); x++)circle(number[i], mask[x], 6, Scalar(255), 2);
	string s = to_string(i);
	imshow(s, number[i]);
	}
	*/

	//3.hog特征
	/*	for (int i = 0; i < number.size(); i++) {
	HOGDescriptor * hog = new HOGDescriptor(Size(3, 3), Size(3, 3), Size(5, 10), Size(3, 3), 9);
	vector<float>descript;
	hog->compute(number[i], descript, Size(1, 1), Size(0, 0));
	}*/

	//4.边界投影
	borders(number[1]);
	for (int i = 0; i < sub.size(); i++)fprintf(f1, "%d\n", sub[i]);
	fclose(f1);
	printf("%d\n*************************\n", sub.size());
	borders(number[0]);
	for (int i = 0; i < sub.size(); i++)fprintf(f2, "%d\n", sub[i]);
	fclose(f2);


	imshow("test1", number[0]);
	imshow("test2", number[1]);
	imshow("mask", mask);
	imshow("pic", img);
	waitKey(0);
	system("pause");
}