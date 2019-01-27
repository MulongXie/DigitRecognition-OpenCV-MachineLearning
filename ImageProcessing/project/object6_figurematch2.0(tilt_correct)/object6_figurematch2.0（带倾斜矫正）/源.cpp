#include <opencv2\opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::ml;

vector<Mat> number;//存储拆分对象(未校正)
vector<int> index;//存储合法轮廓下标
vector<double> angles;//存入转角
vector<Mat> rots;//存入旋转矫正后的对象
vector<int> border;//存储边界特征
vector<int> rborder;
vector<int> topborder;
vector<int> botborder;
int flag = 0;


//拆分轮廓
void divide(Mat& img, Mat& org, vector<vector<Point>> contour, int i) {

	Rect rec = boundingRect(contour[i]);	//计算物体最大外包正轮廓
	if (rec.area() < (org.rows*org.cols) / 30.0 || rec.area() >= (org.rows*org.cols) / 3.0) return;  //面积不达标则不放入检测
	Point2f v2[4];
	v2[0] = rec.tl();
	v2[1].x = rec.tl().x;
	v2[1].y = rec.br().y;
	v2[2] = rec.br();
	v2[3].x = rec.br().x;
	v2[3].y = rec.tl().y;

	RotatedRect mr = minAreaRect(contour[i]);

	Mat show;//分离各特征物体
	show = img(rec);
	number.push_back(show);
	index.push_back(i);
	angles.push_back(mr.angle);


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
void borders(Mat img, vector<int>&fea) {
	int i = 0;
	border.clear();
	rborder.clear();
	topborder.clear();
	botborder.clear();
	fea.clear();
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

	/*	int k = 0;
	while (k < img.cols) {
	for (int j = 0; j < img.rows; j++)
	if (img.at<uchar>(j, k)) {
	topborder.push_back(j);
	break;
	}
	for (int j = img.rows - 1; j >= 0; j--)
	if (img.at<uchar>(j, k)) {
	botborder.push_back(img.rows - j);
	break;
	}
	k += 2;
	}*/

	for (int j = 0; j < rborder.size(); j++) border.push_back(rborder[j]);	//左右边界拼接
	/*	for (int j = 0; j < topborder.size(); j++)border.push_back(topborder[j]);
	for (int j = 0; j < botborder.size(); j++)border.push_back(botborder[j]);*/

	int count = 0;	//确认
	for (int j = 0; j < border.size(); j++) if (!border[j])count++;
	if (count > border.size() / 5.0) {
		cout << "invilid border" << endl;
		return;
	}

	for (int j = 0; j < border.size() - 1; j++) {	//计算拓扑数列
		int a = border[j + 1] - border[j];
		fea.push_back(a);
	}


}


int main() {

	Ptr<SVM> svm;
	svm = StatModel::load<SVM>("svmtrain9.18.txt");
	std::cout << "file open successfully" << endl;

	VideoCapture cap(1);
	while (1) {

		//1.预处理
		number.clear();
		index.clear();
		angles.clear();
		rots.clear();
		Mat testimg;
		cap.read(testimg);
		Mat img1;
		medianBlur(testimg, testimg, 7);
		cvtColor(testimg, img1, CV_BGR2GRAY);
		Mat thresh_img;

		threshold(img1, thresh_img, 100, 255, THRESH_BINARY_INV);//对象图片为白色背景
		imshow("threshold", thresh_img);

		//2.目标分离
		vector<Vec4i> hie;
		vector<vector<Point>> contour;
		findContours(thresh_img, contour, hie, RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
		Mat mask(thresh_img.size(), CV_8UC1, Scalar::all(0));
		for (int i = 0; i < contour.size(); i++) {
			drawContours(mask, contour, i, Scalar(255));
			divide(thresh_img, testimg, contour, i);
		}
		imshow("contour", mask);
		namedWindow("number");
		cout << "contour size " << contour.size() << endl;
		cout << "number size " << number.size() << endl;

		//3.对逐个目标进行提取判别
		for (int i = 0; i < number.size(); i++) {
			int result = -1;
			vector<int> testfea;
			testfea.clear();

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
			borders(rots[i], testfea);
			cout << "size " << testfea.size() << endl;

			//满足特征向量纬度条件则进入预测并框出范围
			if (testfea.size() == 95) {
				Mat testdata = Mat::zeros(1, testfea.size(), CV_32F);
				for (int j = 0; j < testfea.size(); j++) {
					testdata.at<float>(0, j) = testfea[j];
				}
				result = svm->predict(testdata);

				imshow("number", rots[i]);
				std::cout << "the index is " << i << " the result is " << result << endl;
				string str;
				string str2;
				str2 = to_string(result);
				Rect mr = boundingRect(Mat(contour[index[i]]));
				Point2f v[4];
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

				int cenx, ceny;
				cenx = mr.br().x;
				ceny = mr.br().y;
				str = "(" + to_string(cenx) + "," + to_string(ceny) + ")";

				putText(testimg, str, v[0], CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(255, 0, 0), 2);
				putText(testimg, str2, v[3], CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
				break;

			}
		}
		cout << "***********************" << endl << endl;

		imshow("img", testimg);
		waitKey(10);
	}
	std::system("pause");
}
/*特征测试*//*修改版带矫正95纬*/