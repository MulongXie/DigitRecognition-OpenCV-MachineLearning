#include <opencv2\opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <iostream>
#include <time.h>
#include "OCR.h"

using namespace std;
using namespace cv;
using namespace cv::ml;

vector<Mat> number;//存储拆分对象(未校正)
vector<int> index;//存储合法轮廓下标
vector<double> angles;//存入转角
vector<int> areas;//存入面积
vector<Mat> rots;//存入旋转矫正后的对象
vector<bool>recs;//存入矩形判别状态
vector<Point> centers;//存入轮廓中心点用来做剪内轮廓

int flag = 0;
int sflag = 0;
String filename = "number.bmp";
char image_name[13];
int wflag = 0;

Digit_INFO * d = new Digit_INFO;
Ptr<SVM> svm1;
Ptr<SVM> svm2;

//拆分轮廓_黑白**
void divide_witblc(Mat& img, Mat& org, vector<vector<Point>> contour, int i) {

	Rect rec = boundingRect(contour[i]);	//计算物体最大外包正轮廓

	float ratio = 1.0 * rec.size().height / rec.size().width;    //宽高比
	if (ratio < 1.1 || ratio > 5)return;	//宽高比不达标则不放入检测 *剪枝1

	Mat show;     //分离各特征物体
	show = img(rec);

	//Scalar s = sum(show);
	//float fillrate = 1.0 * s[0] / (255 * rec.area());   //原始图像黑色区域
	//if (fillrate < 0.2 || fillrate > 0.95) return;       //区域填充大小不达标则不放入检测（可剪去内轮廓） *剪枝2

	if (rec.tl().x < 5 || rec.tl().y < 5 || rec.br().x > org.cols - 5 || rec.br().y > org.rows - 5) return;    //区域过于接近图像边界则不放入检测 *剪枝3

	//Mat mytest;
	//mytest = org(rec);
	//resize(mytest, mytest, Size(70, 96));
	//cvtColor(mytest, mytest, CV_BGR2HSV);
	//Mat  target1, target2;
	//int mycount1 = 0, mycount2 = 0;
	//inRange(mytest, Scalar(0, 0, 0), Scalar(180, 255, 150), target1);
	//inRange(mytest, Scalar(0, 0, 100), Scalar(180, 60, 255), target2);
	//for (int i = 0; i < target1.rows; i++) {
	//	for (int j = 0; j < target1.cols; j++) {
	//		if (target1.at<uchar>(i, j)) {
	//			mycount1++;
	//		}
	//	}
	//}
	//for (int i = 0; i < target2.rows; i++) {
	//	for (int j = 0; j < target2.cols; j++) {
	//		if (target2.at<uchar>(i, j)) {
	//			mycount2++;
	//		}
	//	}
	//}
	//if ((float)mycount1 / (70 * 96) < 0.2 || (float)mycount2 / (70 * 96) < 0.2) return;	   //区域黑白两色填充率不达标不放入检测 *剪枝4
	
	//Mat gradimg = org(rec);
	//cvtColor(gradimg, gradimg, CV_BGR2GRAY);
	//Sobel(gradimg, gradimg, gradimg.depth(), 1, 1);
	//int avgrad = 0;
	//for (int j = 0; j < contour[i].size(); j++)
	//{
	//	avgrad += gradimg.at<uchar>(contour[i][j] - rec.tl());
	//}
	//avgrad /= contour[i].size();
	//if (avgrad < 3) return;	                           //边缘梯度检测不达标不放入检测 *剪枝5

	double a = contourArea(contour[i]);
	float rate = a / rec.area();
	bool r;
	if (rate > 0.85) {
		r = 1;
		if (rec.area() < (org.rows*org.cols) / 350.0 || rec.area() > (org.rows*org.cols) / 3.0) return;   //矩形面积不达标则不放入检测 *剪枝6
		//if (rec.area() > (org.rows*org.cols) / 5.0) return;
	}
	else {
		r = 0;
		if (rec.area() < (org.rows*org.cols) / 500.0 || rec.area() > (org.rows*org.cols) / 3.0) return;   //非矩形面积不达标则不放入检测 *剪枝6
		//if (rec.area() > (org.rows*org.cols) / 50.0) return;//适用于远距离
	}

	int cenx, ceny;
	cenx = (rec.br().x + rec.tl().x) / 2;
	ceny = (rec.br().y + rec.tl().y) / 2;
	Point center = Point((rec.br().x + rec.tl().x) / 2, (rec.br().y + rec.tl().y) / 2);
	centers.push_back(center);

	RotatedRect mr = minAreaRect(contour[i]);
	number.push_back(show);
	recs.push_back(r);
	index.push_back(i);
	angles.push_back(mr.angle);
	areas.push_back(rec.area());
}

//拆分轮廓_红黑**
void divide_redblc(Mat& img, Mat& org, vector<vector<Point>> contour, int i) {
	Rect rec = boundingRect(contour[i]);	//计算物体最大外包正轮廓
	if (rec.area() < (org.rows*org.cols) / 40) return;   //面积不达标则不放入检测 *剪枝

	Mat show;     //分离各特征物体
	show = img(rec);

	float ratio = 1.0 * rec.size().height / rec.size().width;    //宽高比
	if (ratio < 1.5 || ratio > 4)return;	//宽高比不达标则不放入检测 *剪枝

	if (rec.tl().x < 5 || rec.tl().y < 5 || rec.br().x > org.cols - 5 || rec.br().y > org.rows - 5) return;    //区域过于接近图像边界则不放入检测 *剪枝

	//Mat gradimg = org(rec);
	//cvtColor(gradimg, gradimg, CV_BGR2GRAY);
	//Sobel(gradimg, gradimg, gradimg.depth(), 1, 1);
	//int avgrad = 0;
	//for (int j = 0; j < contour[i].size(); j++)
	//{
	//	avgrad += gradimg.at<uchar>(contour[i][j] - rec.tl());
	//}
	//avgrad /= contour[i].size();
	//if (avgrad < 4) return;	                           //边缘梯度检测不达标不放入检测 *剪枝

	RotatedRect mr = minAreaRect(contour[i]);
	number.push_back(show);
	index.push_back(i);
	angles.push_back(mr.angle);
	areas.push_back(rec.area());
}

//旋转矫正
Mat rotate(Mat img, int angle) {

	Point center(img.cols / 2, img.rows / 2);
	double degree = angle*CV_PI / 180;
	double a = sin(degree);
	double b = cos(degree);
	int wid = int(img.rows*fabs(a) + img.cols*fabs(b));
	int hgt = int(img.rows*fabs(b) + img.cols*fabs(a));


	float rot[6];
	Mat rot_mat = Mat(2, 3, CV_32F, rot);
	CvMat rot_mat2 = rot_mat;	//新建一个cvmat型的指针以满足之后函数形参需要
	cv2DRotationMatrix(center, angle, 1.0, &rot_mat2);

	rot[2] += (wid - img.cols) / 2;
	rot[5] += (hgt - img.rows) / 2;
	Mat img_rot;
	warpAffine(img, img_rot, rot_mat, Size(wid, hgt));
	return img_rot;
}

//精确锁定
Mat extra(Mat img) {
	int flag = 0;
	Point2f v[4];//用矩形标出物体轮廓
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			if (img.at<uchar>(i, j)) {
				v[0].x = i;
				v[1].x = i;
				flag = 1;
				break;
			}
		}
		if (flag)break;
	}
	flag = 0;
	for (int i = img.rows - 1; i >= 0; i--) {
		for (int j = 0; j < img.cols; j++) {
			if (img.at<uchar>(i, j)) {
				v[2].x = i;
				v[3].x = i;
				flag = 1;
				break;
			}
		}
		if (flag)break;
	}
	flag = 0;
	for (int j = 0; j < img.cols; j++) {
		for (int i = 0; i < img.rows; i++) {
			if (img.at<uchar>(i, j)) {
				v[0].y = j;
				v[3].y = j;
				flag = 1;
				break;
			}
		}
		if (flag)break;
	}
	flag = 0;
	for (int j = img.cols - 1; j >= 0; j--) {
		for (int i = 0; i < img.rows; i++) {
			if (img.at<uchar>(i, j)) {
				v[1].y = j;
				v[2].y = j;
				flag = 1;
				break;
			}
		}
		if (flag)break;
	}
	Rect rec(v[0].y, v[0].x, v[1].y - v[0].y, v[3].x - v[0].x);
	Mat show = img(rec);
	resize(show, show, Size(70, 96));
	rots.push_back(show);
	return show;
}

//计算边界特征
void borders1(Mat img, vector<int>&fea) {
	vector<int> border;//存储边界特征
	vector<int> rborder;
	vector<int> tborder;
	vector<int> bborder;
	int i = 0, k = 0;
	int count = 0;	//确认
	fea.clear();

	while (i < img.rows) {	//分成96块计算左右边界
		for (int j = 0; j < img.cols; j++)
			if (img.at<uchar>(i, j)) {
				border.push_back(j);
				if (abs(j) < 1)count++;
				break;
			}
		for (int j = img.cols - 1; j >= 0; j--)
			if (img.at<uchar>(i, j)) {
				rborder.push_back(img.cols - j);
				if (abs(j) < 1)count++;
				break;
			}
		i++;
	}
	for (int j = 0; j < rborder.size(); j++) border.push_back(rborder[j]);	//左右边界拼接

	if (count > border.size() / 5.0) {	//如果物体边界太贴近边框 *剪枝4
		return;
	}


	//int rcount = 0;//判定矩形
	//while (k < img.cols) {
	//	for (int j = 0; j < img.rows; j++)
	//		if (img.at<uchar>(j, k)) {
	//			tborder.push_back(j);
	//			break;
	//		}
	//	for (int j = img.rows - 1; j >= 0; j--)
	//		if (img.at<uchar>(j, k)) {
	//			bborder.push_back(j);
	//			break;
	//		}
	//	k += 2;
	//}
	//for (int j = 0; j < tborder.size() - 1; j++) {
	//	if (abs(tborder[j + 1] - tborder[j]) <= 3)rcount++;
	//}
	//for (int j = 1; j < bborder.size() - 1; j++) {
	//	if (abs(bborder[j + 1] - bborder[j]) <= 3)rcount++;
	//}


	int subcount = 0;//计数过大差值
	for (int j = 0; j < border.size() - 1;) {	//计算拓扑数列
		int a = border[j + 1] - border[j];
		if (abs(a) > 10) {
			subcount++;
			for (int x = 0; x < 3; x++) {
				fea.push_back(a);
				j++;
				if (j >= border.size() - 1)break;
			}
		}
		else
		{
			fea.push_back(a); j++;
		}
		//if (abs(a) <= 3)rcount++;
	}

	if (subcount > 8)fea.clear();//如果大差值数量过多则不进入预测 *剪枝5

}

//计算投影特征
void borders2(Mat img, vector<int>&fea) {
	int i = 0, j = 0;
	int count = 0;
	fea.clear();

	//x方向投影
	for (j = 0; j < img.cols; j++) {
		{
			for (i = 0, count = 0; i < img.rows; i++)
			{
				if (img.at<uchar>(i, j)) {
					count++;
				}
			}
		}
		fea.push_back(count);
	}

	//y方向投影
	for (i = 0; i < img.rows; i++) {
		{
			for (j = 0, count = 0; j < img.cols; j++)
			{
				if (img.at<uchar>(i, j)) {
					count++;
				}
			}
		}
		fea.push_back(count);
	}
}

//通过中心与面积检测是否是嵌套外轮廓
bool checkrec(Point center, int thisarea, int thresh) {
	for (int i = 0; i < centers.size(); i++) {
		if (recs[i]) {
			if (abs(centers[i].x - center.x) <= thresh / 6 && areas[i] < thisarea)return 0;//如果有面积更大的矩形外包则不通过
		}
	}
	return 1;
}

//通过中心检测数字是否有外轮廓矩形
bool checknumber(Point center, int thisarea, int thresh) {
	for (int i = 0; i < centers.size(); i++) {
		if (recs[i]) {
			if (abs(centers[i].x - center.x) <= thresh && areas[i] > 2.5 * thisarea && areas[i] < 10 * thisarea)return 1;//如果有面积大的外包矩形则通过检测
		}
	}
	return 0;
}

//写图像
void writeimg(Mat img)
{
	Mat wrimg = Mat::zeros(Size(img.rows, img.rows), CV_8U);
	Rect rec((img.rows - img.cols) / 2, 0, img.cols, img.rows);
	img.copyTo(wrimg(rec));
	resize(wrimg, wrimg, Size(28, 28));
	sprintf(image_name, "%s%d%s", "image", ++wflag, ".bmp");
	imwrite(image_name, wrimg);
}

//黑白数字处理**
void witblc(Mat&testimg) {

	/**************1.预处理*******************/
	number.clear();
	index.clear();
	angles.clear();
	areas.clear();
	rots.clear();
	recs.clear();
	centers.clear();

	Mat img1;
	medianBlur(testimg, testimg, 3);		//中值滤波
	cvtColor(testimg, img1, CV_BGR2GRAY);
	Mat thresh_img;
	//Mat mean, sdv;
	//meanStdDev(img1,mean, sdv);
	//adaptiveThreshold(img1, thresh_img, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 75, sdv.at<double>(0, 0));
	threshold(img1, thresh_img, 0, 255, CV_THRESH_OTSU + THRESH_BINARY_INV);//otsu阈值分割+反相

	Mat element = getStructuringElement(MORPH_RECT, Size(7, 7), Point(-1, -1));
	dilate(thresh_img, thresh_img, element);	//膨胀	
	erode(thresh_img, thresh_img, element);		//腐蚀

	imshow("threshold", thresh_img);

	/**************2.目标分离*****************/
	vector<Vec4i> hie;
	vector<vector<Point>> contour;
	findContours(thresh_img, contour, hie, RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	Mat mask(thresh_img.size(), CV_8UC1, Scalar::all(0));
	for (int i = 0; i < contour.size(); i++) {
		drawContours(mask, contour, i, Scalar(255), 1);
		divide_witblc(thresh_img, testimg, contour, i);
	}
	namedWindow("number");
	cout << "contour size " << contour.size() << endl;
	cout << "number size " << number.size() << endl;

	/********3.对逐个目标进行提取判别*********/
	int cflag[10] = { 0,0,0,0,0,0,0,0,0,0 };//每个数字只做一次数字识别
	int maxarea = -1;//最大面积矩形
	int maxindex;//最大面积矩形下标
	for (int i = 0; i < number.size(); i++) {

		int result1 = -1;
		int result2 = -1;
		int result = -1;
		vector<int> testfea1;
		vector<int> testfea2;
		testfea1.clear();
		testfea2.clear();

		//提取特征矩阵
		double an = angles[i];
		Mat rot;
		if (fabs(an) != 0 && fabs(an) != 90) {	//如果倾斜则先矫正
			an = (90 - fabs(an)) < fabs(an) ? (90 - fabs(an)) : an;
			rot = rotate(number[i], an);
			extra(rot);
		}
		else {	//否则直接锁定
			extra(number[i]);
		}
		if (recs[i])
		{
			Rect rmr = boundingRect(Mat(contour[index[i]]));
			Point2f v[4];
			v[0] = rmr.tl();
			v[1].x = rmr.tl().x;
			v[1].y = rmr.br().y;
			v[2] = rmr.br();
			v[3].x = rmr.br().x;
			v[3].y = rmr.tl().y;
			Point center = Point((rmr.br().x + rmr.tl().x) / 2, (rmr.br().y + rmr.tl().y) / 2);//中心点

			if (!checkrec(center, rmr.area(), rmr.size().width))continue; //检查嵌套矩形

			for (int i = 0; i < 4; i++) {
				line(testimg, v[i], v[(i + 1) % 4], Scalar(255, 0, 0), 2);
			}
			if (maxarea == -1 || maxarea < areas[i]) { maxarea = areas[i]; maxindex = i; }
			continue;
		}
		else {

			borders1(rots[i], testfea1);
			borders2(rots[i], testfea2);
		}


		//满足特征向量纬度条件则进入预测并框出范围
		if (testfea1.size() == 191 && testfea2.size() == 166) {

			Rect mr = boundingRect(Mat(contour[index[i]]));
			Point2f v[4];
			int cenx, ceny;
			cenx = (mr.br().x + mr.tl().x) / 2;
			ceny = (mr.br().y + mr.tl().y) / 2;
			Point center = Point((mr.br().x + mr.tl().x) / 2, (mr.br().y + mr.tl().y) / 2);

			if (!checknumber(center, mr.area(), mr.size().width))continue;//没有外包轮廓的黑白数字不进入检测 *剪枝8 **关键剪枝

			Mat testdata1 = Mat::zeros(1, testfea1.size(), CV_32F);
			Mat testdata2 = Mat::zeros(1, testfea2.size(), CV_32F);
			for (int j = 0; j < testfea1.size(); j++) {
				testdata1.at<float>(0, j) = testfea1[j];
			}
			for (int j = 0; j < testfea2.size(); j++) {
				testdata2.at<float>(0, j) = testfea2[j];
			}
			cout << testdata1.size() << endl;
			cout << testdata2.size() << endl;
			result1 = svm1->predict(testdata1);
			result2 = svm2->predict(testdata2);
			if (result2 == 2 || result2 == 6 || result2 == 9 || result2 == 4) result = result2;
			else {
				if (result1 == result2) result = result2;
				//else continue;
			}
			if (cflag[result] == 0) {

				d->Spray_Statue = 1;
				d->tagDigit_POINT[result].DigitName = result;
				d->tagDigit_POINT[result].ImgStatue = 1;	//暂定可信度均为1
				d->tagDigit_POINT[result].xPiont_mm = cenx - testimg.cols / 2;
				d->tagDigit_POINT[result].yPiont_mm = ceny - testimg.rows / 2;

				//输出显示
				imshow("number", rots[i]);
				std::cout << "the number index is " << i << " the result is " << result << endl;
				
				cout << endl;
				string str;
				string str2;
				str2 = to_string(result);
				v[0] = mr.tl();
				v[1].x = mr.tl().x;
				v[1].y = mr.br().y;
				v[2] = mr.br();
				v[3].x = mr.br().x;
				v[3].y = mr.tl().y;
				for (int i = 0; i < 3; i++) {
					line(testimg, v[i], v[i + 1], Scalar(0, 0, 255), 2);
				}
				line(testimg, v[3], v[0], Scalar(0, 0, 255), 2);
				str = "(" + to_string(cenx - testimg.cols / 2) + "," + to_string(ceny - testimg.rows / 2) + ")";
				putText(testimg, str, center, CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(255, 0, 0), 2);
				putText(testimg, str2, v[3], CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

				cflag[result] = 1;
			}
		}
	}
	if (maxarea != -1) {
		Rect rmr = boundingRect(Mat(contour[index[maxindex]]));
		Point2f v[4];
		v[0] = rmr.tl();
		v[1].x = rmr.tl().x;
		v[1].y = rmr.br().y;
		v[2] = rmr.br();
		v[3].x = rmr.br().x;
		v[3].y = rmr.tl().y;
		for (int i = 0; i < 4; i++) {
			line(testimg, v[i], v[(i + 1) % 4], Scalar(255, 0, 0), 2);
		}

		//计算喷涂率
		Scalar s = sum(thresh_img(rmr));
		float fillrate = 100 * s[0] / (255 * rmr.area());//原始图像黑色区域
		d->SprayScale = (unsigned char)(int)fillrate;
		cout << "spray fillrate:" << (int)d->SprayScale << endl;

	}
	cout << "*********** binary finish *************" << endl;

}

//红黑数字处理**
void redblc(Mat&testimg) {

	/**************1.预处理*******************/
	number.clear();
	index.clear();
	angles.clear();
	areas.clear();
	rots.clear();
	recs.clear();
	centers.clear();

	Mat hsv;
	//medianBlur(testimg, testimg, 3);		//中值滤波
	cvtColor(testimg, hsv, CV_BGR2HSV);
	Mat thresh_img;
	inRange(hsv, Scalar(-20, 100, 0), Scalar(40, 256, 256), thresh_img);

	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5), Point(-1, -1));
	erode(thresh_img, thresh_img, element);		//腐蚀
	dilate(thresh_img, thresh_img, element);	//膨胀	

	imshow("thresh_red", thresh_img);

	vector<Vec4i> hie;
	vector<vector<Point>> contour;
	findContours(thresh_img, contour, hie, RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	Mat mask(thresh_img.size(), CV_8UC1, Scalar::all(0));
	for (int i = 0; i < contour.size(); i++) {
		drawContours(mask, contour, i, Scalar(255), 1);
		divide_redblc(thresh_img, testimg, contour, i);
	}
	namedWindow("number_red");
	cout << "***** red contour size " << contour.size() << endl;
	cout << "***** red number size " << number.size() << endl;

	/********3.对逐个目标进行提取判别*********/
	int cflag = 0;//每个数字只做一次数字识别
	
	for (int i = 0; i < number.size(); i++) {

		int result1 = -1;
		int result2 = -1;
		int result = -1;
		vector<int> testfea1;
		vector<int> testfea2;
		testfea1.clear();
		testfea2.clear();

		//提取特征矩阵
		double an = angles[i];
		Mat rot;
		if (fabs(an) != 0 && fabs(an) != 90) {	//如果倾斜则先矫正
			an = (90 - fabs(an)) < fabs(an) ? (90 - fabs(an)) : an;
			rot = rotate(number[i], an);
			extra(rot);
		}
		else {	//否则直接锁定
			extra(number[i]);
		}

		borders1(rots[i], testfea1);
		borders2(rots[i], testfea2);


		//满足特征向量纬度条件则进入预测并框出范围
		if (testfea1.size() == 191 && testfea2.size() == 166) {

			Rect mr = boundingRect(Mat(contour[index[i]]));
			Point2f v[4];
			int cenx, ceny;
			cenx = (mr.br().x + mr.tl().x) / 2;
			ceny = (mr.br().y + mr.tl().y) / 2;
			Point center = Point((mr.br().x + mr.tl().x) / 2, (mr.br().y + mr.tl().y) / 2);

			//if (centers.size() != 0) {
			//	if (check(center, mr.area(), mr.size().width))continue;	//中心相近但面积大者剔除针对内轮廓 *剪枝7
			//}

			Mat testdata1 = Mat::zeros(1, testfea1.size(), CV_32F);
			Mat testdata2 = Mat::zeros(1, testfea2.size(), CV_32F);
			for (int j = 0; j < testfea1.size(); j++) {
				testdata1.at<float>(0, j) = testfea1[j];
			}
			for (int j = 0; j < testfea2.size(); j++) {
				testdata2.at<float>(0, j) = testfea2[j];
			}
			cout << testdata1.size() << endl;
			cout << testdata2.size() << endl;
			result1 = svm1->predict(testdata1);
			result2 = svm2->predict(testdata2);
			if (result2 == 2) result = result2;
			else {
				if (result1 == result2) result = result2;
				else continue;
			}


			d->Spray_Statue = 1;
			d->tagDigit_POINT[result].DigitName = result;
			d->tagDigit_POINT[result].ImgStatue = 1;	//暂定可信度均为1
			d->tagDigit_POINT[result].xPiont_mm = cenx - testimg.cols / 2;
			d->tagDigit_POINT[result].yPiont_mm = ceny - testimg.rows / 2;


			cout << endl;
			string str;
			string str2;
			str2 = to_string(result);
			v[0] = mr.tl();
			v[1].x = mr.tl().x;
			v[1].y = mr.br().y;
			v[2] = mr.br();
			v[3].x = mr.br().x;
			v[3].y = mr.tl().y;
			for (int i = 0; i < 3; i++) {
				line(testimg, v[i], v[i + 1], Scalar(0, 255, 0), 2);
			}
			line(testimg, v[3], v[0], Scalar(0, 255, 0), 2);
			str = "(" + to_string(cenx - testimg.cols / 2) + "," + to_string(ceny - testimg.rows / 2) + ")";
			putText(testimg, str, center, CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(255, 0, 0), 2);
			putText(testimg, str2, v[3], CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

			break;
		
		}
	}

	d->SprayScale = 0;

	cout << "*********** redblc finish *************" << endl;
}

int main() {

	
	svm1 = StatModel::load<SVM>("svmtrain11.4.1.txt");
	svm2 = StatModel::load<SVM>("svmtrain11.4.2.txt");
	std::cout << "file open successfully" << endl;

	/*VideoCapture cap(1);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1600);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1200);*/
	VideoCapture cap("F:\\测试视频\\11.9.4.avi");
	long frameToStart = 100;
	cap.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	while (1) {

		float tstart = clock();
		Mat testimg;
		cap.read(testimg);
		//testimg = imread("rb2.png");
		resize(testimg, testimg, Size(800, 600));
		
		witblc(testimg);
		redblc(testimg);
		float tstop = clock();

		cout << "time:" << (tstop - tstart) / CLOCKS_PER_SEC << endl;
		cout << "************************" << endl << endl << endl;
		
		imshow("img", testimg);
		waitKey(10);
	}

	std::system("pause");
}
