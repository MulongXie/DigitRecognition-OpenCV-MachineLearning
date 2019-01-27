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

vector<Mat> number;//�洢��ֶ���(δУ��)
vector<int> index;//�洢�Ϸ������±�
vector<double> angles;//����ת��
vector<int> areas;//�������
vector<Mat> rots;//������ת������Ķ���

vector<Point> centers;//�����������ĵ���������������
vector<int> numberareas;//����ʶ��ɹ�����Ŀ��������������������

int flag = 0;
int sflag = 0;
//�������
void divide(Mat& img, Mat& org, vector<vector<Point>> contour, int i) {

	Rect rec = boundingRect(contour[i]);	//��������������������
	if (rec.area() < (org.rows*org.cols) / 150.0 || rec.area() > (org.rows*org.cols) / 3.0) return;  //���������򲻷����� *��֦1

	float ratio = 1.0 * rec.size().height / rec.size().width;//��߱�
	if (ratio < 1 || ratio > 2)return;	//��߱Ȳ�����򲻷����� *��֦2

	Mat show;//�������������
	show = img(rec);

	Scalar s = sum(show);
	float fillrate = 1.0 * s[0] / (255 * rec.area());//ԭʼͼ���ɫ����
	cout << "fillrate:" << fillrate << endl;
	if (fillrate < 0.2)return; //��������С������򲻷����⣨�ɼ�ȥ�������� *��֦3

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

//��ת����
Mat rotate(Mat img, int angle) {

	Point center(img.cols / 2, img.rows / 2);
	double degree = angle*CV_PI / 180;
	double a = sin(degree);
	double b = cos(degree);
	int wid = int(img.rows*fabs(a) + img.cols*fabs(b));
	int hgt = int(img.rows*fabs(b) + img.cols*fabs(a));


	float rot[6];
	Mat rot_mat = Mat(2, 3, CV_32F, rot);
	CvMat rot_mat2 = rot_mat;	//�½�һ��cvmat�͵�ָ��������֮�����β���Ҫ
	cv2DRotationMatrix(center, angle, 1.0, &rot_mat2);

	rot[2] += (wid - img.cols) / 2;
	rot[5] += (hgt - img.rows) / 2;
	Mat img_rot;
	warpAffine(img, img_rot, rot_mat, Size(wid, hgt));
	return img_rot;
}

//��ȷ����
Mat extra(Mat img) {
	int flag = 0;
	Point2f v[4];//�þ��α����������
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

//����߽�����
void borders(Mat img, vector<int>&fea, bool &rec) {
	vector<int> border;//�洢�߽�����
	vector<int> rborder;
	vector<int> tborder;
	vector<int> bborder;
	int i = 0, k = 0;
	int count = 0;	//ȷ��
	fea.clear();

	while (i < img.rows) {	//�ֳ�96��������ұ߽�
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
	for (int j = 0; j < rborder.size(); j++) border.push_back(rborder[j]);	//���ұ߽�ƴ��

	if (count > border.size() / 5.0) {	//�������߽�̫�����߿� *��֦4
		cout << "invilid border" << endl;
		return;
	}


	int rcount = 0;//�ж�����
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


	int subcount = 0;//���������ֵ
	for (int j = 0; j < border.size() - 1;) {	//������������
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

	if (subcount > 8)fea.clear();//������ֵ���������򲻽���Ԥ�� *��֦5


								 //�����ж�
	if (fea.size() != 191) {
		if (rcount >= img.rows * 2 + img.cols - 40)rec = 1;//���δ��ȫ����ά��Ҫ����ſ�������Ϊ����
		else rec = 0;
	}
	else {
		if (rcount >= img.rows * 2 + img.cols - 20)rec = 1;//��Ϊ����
		else rec = 0;
	}

}

//��⼤���
void redpoint(Mat redimg, short &x, short &y, unsigned char &statue)
{

	//ת����hsv�ռ�
	Mat hsv;
	cvtColor(redimg, hsv, CV_BGR2HSV);
	//ɸѡ��ɫ����
	Mat target, target1, target2;
	inRange(hsv, Scalar(156, 43, 46), Scalar(180, 255, 255), target1);
	inRange(hsv, Scalar(0, 43, 46), Scalar(10, 255, 255), target2);
	target = target1 | target2;

	medianBlur(target, target, 5);

	vector<vector<Point>> rcontours; // �������Χ���Һ��
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
		//���ĵ�����
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

//ͨ���������������Ƿ���Ƕ��������
bool check(Point center,int thisarea,int thresh) {
	for (int i = 0; i < centers.size(); i++) {
		if (abs(centers[i].x - center.x) <= thresh / 2 && numberareas[i] < thisarea)return 1;//����������������ɾ��
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
		//1.Ԥ����
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

		//Ѱ�Һ��
		Rect rect(testimg.cols / 4, testimg.rows / 4, testimg.cols / 2, testimg.rows / 2);
		Mat redimg = Mat::zeros(testimg.rows / 2, testimg.cols / 2, CV_8UC3);
		testimg(rect).copyTo(redimg);//ѡȡ����
									 //imshow("aaaa", redimg);
		redpoint(redimg, d->ShotPoint_x_mm, d->ShotPoint_y_mm, d->Spray_Statue);
		//redpoint(redimg);

		//testimg = imread("pic.png");
		cvtColor(testimg, img1, CV_BGR2GRAY);
		Mat thresh_img;
		threshold(img1, thresh_img, 0, 255, CV_THRESH_OTSU);//otsu��ֵ�ָ�
		threshold(thresh_img, thresh_img, 150, 255, THRESH_BINARY_INV);//����
		Mat element = getStructuringElement(MORPH_RECT, Size(10, 10), Point(-1, -1));
		dilate(thresh_img, thresh_img, element);
		erode(thresh_img, thresh_img, element);
		imshow("threshold", thresh_img);
		//2.Ŀ�����
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

		//3.�����Ŀ�������ȡ�б�
		int cflag[10] = { 0,0,0,0,0,0,0,0,0,0 };//ÿ������ֻ��һ������ʶ��
		int maxarea = -1;//����������
		int maxindex;//�����������±�
		for (int i = 0; i < number.size(); i++) {
			int result = -1;
			vector<int> testfea;
			testfea.clear();
			//��ȡ��������
			double an = angles[i];
			bool rec = 0;
			Mat rot;
			if (fabs(an) != 0 && fabs(an) != 90) {	//�����б���Ƚ���
				an = (90 - fabs(an)) < fabs(an) ? (90 - fabs(an)) : an;
				rot = rotate(number[i], an);
				extra(rot);
			}
			else {	//����ֱ������
				extra(number[i]);
			}
			borders(rots[i], testfea, rec);

			cout << "size " << testfea.size() << endl;
			cout << "rectangle " << rec << endl;
			//�ж��Ƿ�Ϊ���β�ѡȡ����������
			if (rec) {
				if (maxarea == -1 || maxarea < areas[i]) { maxarea = areas[i]; maxindex = i; }
				continue;
			}
			//������������γ�����������Ԥ�Ⲣ�����Χ
			if (testfea.size() == 191) {

				Rect mr = boundingRect(Mat(contour[index[i]]));
				Point2f v[4];
				int cenx, ceny;
				cenx = (mr.br().x + mr.tl().x) / 2;
				ceny = (mr.br().y + mr.tl().y) / 2;
				Point center = Point((mr.br().x + mr.tl().x) / 2, (mr.br().y + mr.tl().y) / 2);


				if (centers.size() != 0) {
					if (check(center,mr.area(),mr.size().width))continue;	//�����������������޳���������� *��֦7
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
					d->tagDigit_POINT[result].ImgStatue = 1;	//�ݶ����ŶȾ�Ϊ1
					d->tagDigit_POINT[result].xPiont_mm = cenx - testimg.cols / 2;
					d->tagDigit_POINT[result].yPiont_mm = ceny - testimg.rows / 2;

					//�����ʾ
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

			//������Ϳ��
			Scalar s = sum(thresh_img(rmr));
			float fillrate = 100 * s[0] / (255 * rmr.area());//ԭʼͼ���ɫ����
			d->SprayScale = (unsigned char)(int)fillrate;
			cout << "spray fillrate:" << (int)d->SprayScale << endl;

		}
		cout << "***********************" << endl << endl;
		imshow("img", testimg);
		waitKey(10);
	}
	std::system("pause");
}
/*��̬���ִ���*//*��Э��*/
/*�Ľ���֦Ԥ����*//*�����Ѹ�*//*191γ��*/
/*���뼤��ʶ�����Ϳ��ʶ��*/

/*�Ľ���������֦*/