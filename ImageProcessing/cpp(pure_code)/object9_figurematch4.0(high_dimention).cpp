#include <opencv2\opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <iostream>
#include "OCR.h"

using namespace std;
using namespace cv;
using namespace cv::ml;

vector<Mat> number;//存储拆分对象(未校正)
vector<int> index;//存储合法轮廓下标
vector<double> angles;//存入转角
vector<int> areas;//存入面积
vector<Mat> rots;//存入旋转矫正后的对象

vector<Point> centers;//存入轮廓中心点用来做剪内轮廓
vector<int> numberareas;//存入识别成功数字目标的面积用来做剪内轮廓

int flag = 0;
int sflag = 0;
//拆分轮廓
void divide(Mat& img, Mat& org, vector<vector<Point>> contour, int i) {

	Rect rec = boundingRect(contour[i]);	//计算物体最大外包正轮廓
	if (rec.area() < (org.rows*org.cols) / 150.0 || rec.area() > (org.rows*org.cols) / 3.0) return;  //面积不达标则不放入检测 *剪枝1

	float ratio = 1.0 * rec.size().height / rec.size().width;//宽高比
	if (ratio < 1 || ratio > 2)return;	//宽高比不达标则不放入检测 *剪枝2

	Mat show;//分离各特征物体
	show = img(rec);

	Scalar s = sum(show);
	float fillrate = 1.0 * s[0] / (255 * rec.area());//原始图像黑色区域
	cout << "fillrate:" << fillrate << endl;
	if (fillrate < 0.2)return; //区域填充大小不达标则不放入检测（可剪去内轮廓） *剪枝3

	Point2f v2[4];
	v2[0] = rec.tl();
	v2[1].x = rec.tl().x;
	v2[1].y = rec.br().y;
	v2[2] = rec.br();
	v2[3].x = rec.br().x;
	v2[3].y = rec.tl().y;

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
	resize(show, show, Size(50, 96));
	rots.push_back(show);
	return show;
}

//计算边界特征
void borders(Mat img, vector<int>&fea, bool &rec) {
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
				break;
			}
		for (int j = img.cols - 1; j >= 0; j--)
			if (img.at<uchar>(i, j)) {
				rborder.push_back(img.cols - j);
				break;
			}
		i++;
	}
	for (int j = 0; j < rborder.size(); j++) border.push_back(rborder[j]);	//左右边界拼接

	if (count > border.size() / 5.0) {	//如果物体边界太贴近边框 *剪枝4
		cout << "invilid border" << endl;
		return;
	}


	int rcount = 0;//判定矩形
	while (k < img.cols) {
		for (int j = 0; j < img.rows; j++)
			if (img.at<uchar>(j, k)) {
				tborder.push_back(j);
				break;
			}
		for (int j = img.rows - 1; j >= 0; j--)
			if (img.at<uchar>(j, k)) {
				bborder.push_back(j);
				break;
			}
		k += 2;
	}
	for (int j = 0; j < tborder.size() - 1; j++) {
		if (abs(tborder[j + 1] - tborder[j]) <= 1)rcount++;
	}
	for (int j = 1; j < bborder.size() - 1; j++) {
		if (abs(bborder[j + 1] - bborder[j]) <= 1)rcount++;
	}


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
		if (abs(a) <= 1)rcount++;
	}
	cout << "****rcount:" << rcount << " thresh:" << img.rows * 2 + img.cols - 20 << endl;

	if (subcount > 8)fea.clear();//如果大差值数量过多则不进入预测 *剪枝5


								 //矩形判定
	if (fea.size() != 191) {
		if (rcount >= img.rows * 2 + img.cols - 40)rec = 1;//如果未完全符合维度要求则放宽条件判为矩形
		else rec = 0;
	}
	else {
		if (rcount >= img.rows * 2 + img.cols - 20)rec = 1;//判为矩形
		else rec = 0;
	}

}

//检测激光点
void redpoint(Mat redimg, short &x, short &y, unsigned char &statue)
{

	//转化到hsv空间
	Mat hsv;
	cvtColor(redimg, hsv, CV_BGR2HSV);
	//筛选红色区域
	Mat target, target1, target2;
	inRange(hsv, Scalar(156, 43, 46), Scalar(180, 255, 255), target1);
	inRange(hsv, Scalar(0, 43, 46), Scalar(10, 255, 255), target2);
	target = target1 | target2;

	medianBlur(target, target, 5);

	vector<vector<Point>> rcontours; // 利用最大范围查找红点
	findContours(target, rcontours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);
	double mymax = 0;
	vector<Point> max_contours;
	for (int i = 0; i < rcontours.size(); i++) {
		double area = contourArea(rcontours[i]);
		if (area > mymax) {
			mymax = area;
			max_contours = rcontours[i];
		}
	}

	if (rcontours.size() != 0)
	{
		Rect mr = boundingRect(Mat(max_contours));
		Point2f v[4];
		v[0] = mr.tl();
		v[1].x = mr.tl().x;
		v[1].y = mr.br().y;
		v[2] = mr.br();
		v[3].x = mr.br().x;
		v[3].y = mr.tl().y;
		for (int i = 0; i < 3; i++) {
			line(redimg, v[i], v[i + 1], Scalar(0, 0, 255), 2);
		}
		line(redimg, v[3], v[0], Scalar(0, 0, 255), 2);
		//中心点坐标
		x = (mr.tl().x + mr.br().x) / 2;
		y = (mr.tl().y + mr.br().y) / 2;
		statue = 1;
	}
	else
	{
		statue = 0;
		x = 0;
		y = 0;
	}
	imshow("red", redimg);

}

//通过中心与面积检测是否是嵌套外轮廓
bool check(Point center,int thisarea,int thresh) {
	for (int i = 0; i < centers.size(); i++) {
		if (abs(centers[i].x - center.x) <= thresh / 2 && numberareas[i] < thisarea)return 1;//中心相近面积大者判删除
	}
	return 0;
}

int main() {

	Digit_INFO * d = new Digit_INFO;

	Ptr<SVM> svm;
	svm = StatModel::load<SVM>("svmtrain10.25.txt");
	std::cout << "file open successfully" << endl;

	VideoCapture cap(1);
	while (1) {
		//1.预处理
		number.clear();
		index.clear();
		angles.clear();
		areas.clear();
		rots.clear();
		centers.clear();
		numberareas.clear();

		Mat testimg;
		cap.read(testimg);
		Mat img1;
		medianBlur(testimg, testimg, 3);

		//寻找红点
		Rect rect(testimg.cols / 4, testimg.rows / 4, testimg.cols / 2, testimg.rows / 2);
		Mat redimg = Mat::zeros(testimg.rows / 2, testimg.cols / 2, CV_8UC3);
		testimg(rect).copyTo(redimg);//选取区域
									 //imshow("aaaa", redimg);
		redpoint(redimg, d->ShotPoint_x_mm, d->ShotPoint_y_mm, d->Spray_Statue);
		//redpoint(redimg);

		//testimg = imread("pic.png");
		cvtColor(testimg, img1, CV_BGR2GRAY);
		Mat thresh_img;
		threshold(img1, thresh_img, 0, 255, CV_THRESH_OTSU);//otsu阈值分割
		threshold(thresh_img, thresh_img, 150, 255, THRESH_BINARY_INV);//反相
		Mat element = getStructuringElement(MORPH_RECT, Size(10, 10), Point(-1, -1));
		dilate(thresh_img, thresh_img, element);
		erode(thresh_img, thresh_img, element);
		imshow("threshold", thresh_img);
		//2.目标分离
		vector<Vec4i> hie;
		vector<vector<Point>> contour;
		findContours(thresh_img, contour, hie, RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
		Mat mask(thresh_img.size(), CV_8UC1, Scalar::all(0));
		for (int i = 0; i < contour.size(); i++) {
			drawContours(mask, contour, i, Scalar(255), 1);
			divide(thresh_img, testimg, contour, i);
		}
		imshow("contour", mask);
		namedWindow("number");
		cout << "contour size " << contour.size() << endl;
		cout << "number size " << number.size() << endl;

		//3.对逐个目标进行提取判别
		int cflag[10] = { 0,0,0,0,0,0,0,0,0,0 };//每个数字只做一次数字识别
		int maxarea = -1;//最大面积矩形
		int maxindex;//最大面积矩形下标
		for (int i = 0; i < number.size(); i++) {
			int result = -1;
			vector<int> testfea;
			testfea.clear();
			//提取特征矩阵
			double an = angles[i];
			bool rec = 0;
			Mat rot;
			if (fabs(an) != 0 && fabs(an) != 90) {	//如果倾斜则先矫正
				an = (90 - fabs(an)) < fabs(an) ? (90 - fabs(an)) : an;
				rot = rotate(number[i], an);
				extra(rot);
			}
			else {	//否则直接锁定
				extra(number[i]);
			}
			borders(rots[i], testfea, rec);

			cout << "size " << testfea.size() << endl;
			cout << "rectangle " << rec << endl;
			//判定是否为矩形并选取最大面积矩形
			if (rec) {
				if (maxarea == -1 || maxarea < areas[i]) { maxarea = areas[i]; maxindex = i; }
				continue;
			}
			//满足特征向量纬度条件则进入预测并框出范围
			if (testfea.size() == 191) {

				Rect mr = boundingRect(Mat(contour[index[i]]));
				Point2f v[4];
				int cenx, ceny;
				cenx = (mr.br().x + mr.tl().x) / 2;
				ceny = (mr.br().y + mr.tl().y) / 2;
				Point center = Point((mr.br().x + mr.tl().x) / 2, (mr.br().y + mr.tl().y) / 2);


				if (centers.size() != 0) {
					if (check(center,mr.area(),mr.size().width))continue;	//中心相近但面积大者剔除针对内轮廓 *剪枝7
				}

				Mat testdata = Mat::zeros(1, testfea.size(), CV_32F);
				for (int j = 0; j < testfea.size(); j++) {
					testdata.at<float>(0, j) = testfea[j];
				}
				result = svm->predict(testdata);
				if (cflag[result] == 0) {

					centers.push_back(center);
					numberareas.push_back(areas[i]);

					d->Spray_Statue = 1;
					d->tagDigit_POINT[result].DigitName = result;
					d->tagDigit_POINT[result].ImgStatue = 1;	//暂定可信度均为1
					d->tagDigit_POINT[result].xPiont_mm = cenx - testimg.cols / 2;
					d->tagDigit_POINT[result].yPiont_mm = ceny - testimg.rows / 2;

					//输出显示
					imshow("number", rots[i]);
					std::cout << "the index is " << i << " the result is " << result << endl;
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
		cout << "***********************" << endl << endl;
		imshow("img", testimg);
		waitKey(10);
	}
	std::system("pause");
}
/*动态数字处理*//*带协议*/
/*改进剪枝预处理*//*特征已改*//*191纬版*/
/*加入激光识别和喷涂率识别*/

/*改进内轮廓剪枝*/