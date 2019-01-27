#include "opencv2\core\core.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include <iostream>
using namespace std;
using namespace cv;

Mat org;
Mat img;
int flag = 0;
vector<Mat>number;//存储拆分物体
vector<Point2f>centers;//存储物体圆心
vector<vector<Point2f>>points;

				  //协议格式
typedef struct tagDigit_POINT
{
	unsigned char ImgStatue;//  图像识别状态  0：没有识别成功  1识别成功 2 识别质量不高
	unsigned char DigitName = 0; // 颜色值  0：识别区  1,2,3：RGB
	short int xPiont_mm = 0;  // 数字中心位置的x坐标值 单位mm  如下图
	short int yPiont_mm = 0;  // 数字中心位置的y坐标值 单位mm
}Digit_POINT;
typedef struct tagDigit_INFO
{
	unsigned char Head = 36; //暂定为 36
	unsigned char FrameID = flag; //帧ID，每发一次，会加一
	unsigned char Key = 1; //帧关键字 ，暂定为常值 	1

	unsigned char Spray_Statue = 0; //禁用：暂定为0
	unsigned char SprayScale = 0; //禁用：暂定为0
	unsigned char ShotPoint_Statue = 0; // 禁用：暂定为0
	short int ShotPoint_x_mm = 0;  // 红外瞄准点 坐标X
	short int ShotPoint_y_mm = 0;  // 红外瞄准点 坐标Y


	Digit_POINT tagDigit_POINT[11]; //tagDigit_POIN[0]：等待区；tagDigit_POIN[1]- tagDigit_POIN[5]：四个靶位区  4*11=44bytes
	uint8_t  tmp1 = 0; // 从head到CheckSum之前的所有数据按照无符号字节加和的低八位，存在CheckSum以便校验
	uint8_t  tmp2 = 0; // 预设值0
}Digit_INFO;  // 一共 29bytes

			  //计算校验和
void unit(int a, int bin[]) {
	if (a == 0) {
		for (int i = 0; i < 16; i++) { bin[i] = 0; }
		return;
	}
	int j = 0;
	int temp = a;
	if (temp<0)temp = 65536 + temp;
	while (temp) {
		bin[j++] = temp % 2;
		temp /= 2;
	}
	for (int i = j; i<16; i++)bin[i] = 0;
	//	cout << "dec:" << a << " bin:";
	//	for (int i = 15; i >= 0; i--)cout << bin[i];
	//	cout << endl;
}
void unitsum(int bin1[], int bin2[], int bin3[]) {
	for (int i = 0; i < 16; i++) {
		if (bin1[i] == bin2[i])bin3[i] = 0;
		else bin3[i] = 1;
	}
	//cout << endl << "add1:";
	//for (int i = 15; i >= 0; i--)cout << bin1[i];
	//cout << " add2:";
	//for (int i = 15; i >= 0; i--)cout << bin2[i];
	//cout << endl << "result:";
	//for (int i = 15; i >= 0; i--)cout << bin3[i];
}
int trandec(int a[]) {
	int m = 1;
	int count = 0;
	for (int i = 0; i < 7; i++) {
		int s = a[i] * m;
		count += s;
		m *= 2;
	}
	return count;
}
void checksum(Digit_INFO&d) {
	int bin1[16], bin2[16], bin3[16], bin4[16];
	unit(d.Head, bin1);
	unit(d.FrameID, bin2);
	unitsum(bin1, bin2, bin4);
	if (bin4[0] == 1)bin4[0] = 0;
	else bin4[0] = 1;

	for (int i = 0; i < 11; i++) {
		unit(d.tagDigit_POINT[i].ImgStatue, bin1);
		unit(d.tagDigit_POINT[i].DigitName, bin2);
		unitsum(bin1, bin2, bin3);
		unit(d.tagDigit_POINT[i].xPiont_mm, bin1);
		unitsum(bin1, bin3, bin2);
		unit(d.tagDigit_POINT[i].yPiont_mm, bin1);
		unitsum(bin1, bin2, bin3);
		unitsum(bin3, bin4, bin4);
	}

	d.tmp1 = trandec(bin4);
}

//判定物体颜色
bool range(int a, int min, int max) {
	if (a >= min && a <= max)return true;
	else return false;
}
int color(Mat num, int x, int&criteria,int mark[]) {
	Mat hsv;
	cvtColor(num, hsv, CV_BGR2HSV);
	//	imshow(to_string(flag), num);
	vector<Mat>hsvs;
	split(hsv, hsvs);
	int c[4] = { 0,0,0,0 };//黄绿蓝红

	int index = 0;
	int max = c[0];
	int flagc = 0;
	int standard = (num.cols*num.rows / 3) * 2;
	for (int i = 0; i < num.rows; i++) {
		for (int j = 0; j < num.cols; j++) {
			if (range(hsvs[0].at<uchar>(i, j), 8, 34) && range(hsvs[1].at<uchar>(i, j), 43, 255) ) {
				c[0]++;
				if (c[0] >= standard) {
					index = 0;
					flagc = 1;
					max = c[0];
					break;
				}
			}
			if (range(hsvs[0].at<uchar>(i, j), 35, 100) && range(hsvs[1].at<uchar>(i, j), 43, 255) ) {
				c[1]++;
				if (c[1] >= standard) {
					index = 1;
					flagc = 1;
					max = c[1];
					break;
				}
			}
			if (range(hsvs[0].at<uchar>(i, j), 100, 124) && range(hsvs[1].at<uchar>(i, j), 43, 255) ) {
				c[2]++;
				if (c[2] >= standard) {
					index = 2;
					flagc = 1;
					max = c[2];
					break;
				}
			}
			if ((range(hsvs[0].at<uchar>(i, j), 0, 10) || range(hsvs[0].at<uchar>(i, j), 156, 180)) && range(hsvs[1].at<uchar>(i, j), 43, 255)) {
				c[3]++;
				if (c[3] >= standard) {
					index = 3;
					flagc = 1;
					max = c[3];
					break;
				}
			}
		}
		if (flagc)break;
	}

	cout << "yellow:" << c[0] << " green:" << c[1] << " blue:" << c[2] << " red:" << c[3] << endl;

	if (!flagc) {
		for (int i = 1; i < 4; i++) {
			if (max < c[i]) {
				max = c[i];
				index = i;
			}
		}
		if (max < num.cols*num.rows / 2) {	//无合标准物体
			cout << "invilid aera" << endl;
			return -1;
		}
		criteria = 2;//低质量
	}
	else criteria = 1;//高质量 

	if (mark[index])return - 1;

	String str2;
	switch (index) {
	case 0:cout << "黄色" << endl; str2 = "Yellow"; break;
	case 1:cout << "绿色" << endl; str2 = "Green"; break;
	case 2:cout << "蓝色" << endl; str2 = "Blue"; break;
	case 3:cout << "红色" << endl; str2 = "Red"; break;
	}

	String str = "(" + to_string((int)(centers[x].x - img.cols / 2)) + "," + to_string((int)(centers[x].y - img.rows / 2)) + ")" + str2;
	putText(img, str, centers[x], CV_FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 2);
	//circle(img, centers[x], radius[x], Scalar(0, 0, 0), 2);

	for (int i = 0; i < 4; i++)line(img, points[x][i], points[x][(i + 1) % 4], Scalar(0, 0, 255), 2);
	
	mark[index] = 1;

	return index + 1;
}
//将数据存储至协议格式
void trans(int mark[]) {
	Digit_INFO d;
	unsigned char col;
	int criteria;
	int flagt[5] = { 0,0,0,0,0 };
	for (int i = 0; i < number.size(); i++) {
		criteria = 0;
		col = color(number[i], i, criteria, mark);
		if (col != -1) {
			if (!flagt[col]) {	//只标记第一次
				d.tagDigit_POINT[col].ImgStatue = criteria;

				d.tagDigit_POINT[col].DigitName = col;

				d.tagDigit_POINT[col].xPiont_mm = centers[i].x - img.cols / 2;
				d.tagDigit_POINT[col].yPiont_mm = centers[i].y - img.rows / 2;

				flagt[col] = 1;
			}
		}
	}
	cout << (int)d.Head << " " << (int)d.FrameID << " " << (int)d.Key << " " << (int)d.Spray_Statue << " " << (int)d.SprayScale << " " << (int)d.ShotPoint_Statue;
	cout << " " << (int)d.ShotPoint_x_mm << " " << (int)d.ShotPoint_y_mm << " ";
	for (int j = 0; j < 11; j++) {
		if (j >= 5 || !flagt[j]) {
			d.tagDigit_POINT[j].ImgStatue = 0;
		}
		cout << (int)d.tagDigit_POINT[j].ImgStatue << " " << (int)d.tagDigit_POINT[j].DigitName << " " << (int)d.tagDigit_POINT[j].xPiont_mm << " " << (int)d.tagDigit_POINT[j].yPiont_mm << " ";
	}
	checksum(d);
	cout << (int)d.tmp1 << " " << (int)d.tmp2 << endl;
}
//物体分离
void draw(Mat & img, vector<vector<Point>> contours, int i) {
	Point2f v[4];
	Rect mr = boundingRect(Mat(contours[i]));
	if (mr.area() < img.cols*img.rows / 45)return;//去除噪声区域

	v[0] = mr.tl();
	v[1].x = mr.tl().x;
	v[1].y = mr.br().y;
	v[2] = mr.br();
	v[3].x = mr.br().x;
	v[3].y = mr.tl().y;
	/*
	for (int j = 0; j < 3; j++) {
		line(img, v[j], v[j + 1], Scalar(0, 0, 255), 2);
	}
	line(img, v[3], v[0], Scalar(0, 0, 255), 2);*/

	Point2f center;
	float r;
	center.x = (mr.tl().x + mr.br().x) / 2;
	center.y = (mr.tl().y + mr.br().y) / 2;
	int hgt, wid;
	hgt = mr.br().y - mr.tl().y;
	wid = mr.br().x - mr.br().x;
	r = (hgt > wid ? hgt : wid) / 2;

	Mat show;//分离各特征物体
	show = img(mr);
	resize(show, show, Size(50, 96));
	number.push_back(show);
	centers.push_back(center);
	vector<Point2f>vx;
	for (int i = 0; i < 4; i++)vx.push_back(v[i]);
	points.push_back(vx);
}


int main() {

	VideoCapture cap(1);
	while (1) {
		cap.read(img);

		number.clear();
		centers.clear();
		points.clear();
		int mark[5] = { 0,0,0,0,0 };
		
		int thresh = 0;//运行阀门

		//img = imread(".png");
		resize(img, img, Size(400, 300));

		Mat blur;
		medianBlur(img, blur, 5);
		Mat img_gray;
		cvtColor(blur, img_gray, CV_BGR2GRAY);
		//Mat thresh_img;
		//threshold(img_gray, thresh_img, 120, 255, THRESH_BINARY);//二值化
		//imshow("thresh", thresh_img);
		Mat edge;
		Canny(img_gray, edge, 100, 150);
		vector<Vec4i>hier;
		vector<vector<Point>> contours;
		findContours(edge, contours, hier, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		cout << "contour:" << contours.size() << endl;
		if (contours.size() > 60) thresh = 1;
		
		if (!thresh) {
			
			for (int i = 0; i < contours.size(); i++) {

				draw(img, contours, i);
			}
			trans(mark);
			cout << "number:" << number.size() << endl;
		}
		flag++;
		line(img, Point(img.cols / 2 + 5, img.rows / 2), Point(img.cols / 2 - 5, img.rows / 2), Scalar(0, 0, 255), 1);
		line(img, Point(img.cols / 2, img.rows / 2 + 5), Point(img.cols / 2, img.rows / 2 - 5), Scalar(0, 0, 255), 1);

		cout << "***************************************" << endl << endl;

		imshow("mask", edge);

		imshow("imgs", img);
		imshow("pic", blur);
		waitKey(10);
	}
	std::system("pause");
}
/*飞机颜色识别*//*带协议转换*//*校验和修改版*//*已存*/