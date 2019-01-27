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

vector<Mat> number;//�洢��ֶ���(δУ��)
vector<int> index;//�洢�Ϸ������±�
vector<double> angles;//����ת��
vector<int> areas;//�������
vector<Mat> rots;//������ת������Ķ���
vector<bool>recs;//��������б�״̬
vector<Point> centers;//�����������ĵ���������������
vector<int> numberareas;//����ʶ��ɹ�����Ŀ��������������������

int flag = 0;
int sflag = 0;
String filename = "number.bmp";
char image_name[13];
int i = 0;

//�������
void divide(Mat& img, Mat& org, vector<vector<Point>> contour, int i) {

	Rect rec = boundingRect(contour[i]);	//��������������������
	if (rec.area() < (org.rows*org.cols) / 400.0 || rec.area() > (org.rows*org.cols) / 10.0) return;   //���������򲻷����� *��֦1

	float ratio = 1.0 * rec.size().height / rec.size().width;    //��߱�
	if (ratio < 1.1 || ratio > 3)return;	//��߱Ȳ�����򲻷����� *��֦2

	Mat show;     //�������������
	show = img(rec);

	Scalar s = sum(show);
	float fillrate = 1.0 * s[0] / (255 * rec.area());   //ԭʼͼ���ɫ����
	if (fillrate < 0.2 || fillrate>0.95)return;       //��������С������򲻷����⣨�ɼ�ȥ�������� *��֦3

	if (rec.tl().x < 5 || rec.tl().y < 5 || rec.br().x > org.cols - 5 || rec.br().y > org.rows - 5) return;    //������ڽӽ�ͼ��߽��򲻷����� *��֦4

	Mat mytest;
	mytest = org(rec);
	resize(mytest, mytest, Size(70, 96));
	cvtColor(mytest, mytest, CV_BGR2HSV);
	Mat  target1, target2;
	int mycount1 = 0, mycount2 = 0;
	inRange(mytest, Scalar(0, 0, 0), Scalar(180, 255, 150), target1);
	inRange(mytest, Scalar(0, 0, 160), Scalar(180, 60, 255), target2);
	for (int i = 0; i < target1.rows; i++) {
		for (int j = 0; j < target1.cols; j++) {
			if (target1.at<uchar>(i, j)) {
				mycount1++;
			}
		}
	}
	for (int i = 0; i < target2.rows; i++) {
		for (int j = 0; j < target2.cols; j++) {
			if (target2.at<uchar>(i, j)) {
				mycount2++;
			}
		}
	}
	if ((float)mycount1 / (70 * 96)<0.2 || (float)mycount2 / (70 * 96)<0.15) return;	   //����ڰ���ɫ����ʲ���겻������ *��֦5

	Mat gradimg = org(rec);
	cvtColor(gradimg, gradimg, CV_BGR2GRAY);
	Sobel(gradimg, gradimg, gradimg.depth(), 1, 1);
	int avgrad = 0;
	for (int j = 0; j < contour[i].size(); j++)
	{
		avgrad += gradimg.at<uchar>(contour[i][j] - rec.tl());
	}
	avgrad /= contour[i].size();
	if (avgrad < 9) return;	                           //��Ե�ݶȼ�ⲻ��겻������ *��֦6

	double a = contourArea(contour[i]);
	float rate = a / rec.area();
	bool r;
	if (rate > 0.8)r = 1;           //�������ռ��	*��֦7
	else r = 0;

	RotatedRect mr = minAreaRect(contour[i]);
	number.push_back(show);
	recs.push_back(r);
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
	resize(show, show, Size(70, 96));
	rots.push_back(show);
	return show;
}

//����߽�����
void borders1(Mat img, vector<int>&fea) {
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
	for (int j = 0; j < rborder.size(); j++) border.push_back(rborder[j]);	//���ұ߽�ƴ��

	if (count > border.size() / 5.0) {	//�������߽�̫�����߿� *��֦4
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
		if (abs(tborder[j + 1] - tborder[j]) <= 3)rcount++;
	}
	for (int j = 1; j < bborder.size() - 1; j++) {
		if (abs(bborder[j + 1] - bborder[j]) <= 3)rcount++;
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
		if (abs(a) <= 3)rcount++;
	}

	if (subcount > 8)fea.clear();//������ֵ���������򲻽���Ԥ�� *��֦5

}

//����ͶӰ����
void borders2(Mat img, vector<int>&fea) {
	int i = 0, j = 0;
	int count = 0;
	fea.clear();

	//x����ͶӰ
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

	//y����ͶӰ
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

//ͨ���������������Ƿ���Ƕ��������
bool check(Point center, int thisarea, int thresh) {
	for (int i = 0; i < centers.size(); i++) {
		if (abs(centers[i].x - center.x) <= thresh / 2 && numberareas[i] < thisarea)return 1;//����������������ɾ��
	}
	return 0;
}

//дͼ��
void writeimg(Mat img)
{
	Mat wrimg = Mat::zeros(Size(img.rows, img.rows), CV_8U);
	Rect rec((img.rows - img.cols) / 2, 0, img.cols, img.rows);
	img.copyTo(wrimg(rec));
	resize(wrimg, wrimg, Size(28, 28));
	sprintf(image_name, "%s%d%s", "image", ++i, ".bmp");
	imwrite(image_name, wrimg);
}


int main() {

	Digit_INFO * d = new Digit_INFO;
	Ptr<SVM> svm1;
	Ptr<SVM> svm2;
	svm1 = StatModel::load<SVM>("svmtrain11.4.1.txt");
	svm2 = StatModel::load<SVM>("svmtrain11.4.2.txt");
	std::cout << "file open successfully" << endl;

	//VideoCapture cap(1);
	VideoCapture cap("F:\\������Ƶ\\11.5.16.1.5.avi");
	//long frameToStart = 350;
	//cap.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	while (1) {
		float tstart = clock();

		Mat testimg;
		cap.read(testimg);
		resize(testimg, testimg, Size(800, 600));
		/**************1.Ԥ����*******************/
		number.clear();
		index.clear();
		angles.clear();
		areas.clear();
		rots.clear();
		recs.clear();
		centers.clear();
		numberareas.clear();

		Mat img1;
		//medianBlur(testimg, testimg, 3);		//��ֵ�˲�
		cvtColor(testimg, img1, CV_BGR2GRAY);
		//equalizeHist(img1, img1);			//ֱ��ͼ���⻯

		Mat thresh_img;
		//Mat mean, sdv;
		//meanStdDev(img1,mean, sdv);
		//adaptiveThreshold(img1, thresh_img, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 75, sdv.at<double>(0, 0));
		threshold(img1, thresh_img, 0, 255, CV_THRESH_OTSU + THRESH_BINARY_INV);//otsu��ֵ�ָ�+����
																				//threshold(img1, thresh_img, 170, 255,THRESH_BINARY_INV);

		Mat element = getStructuringElement(MORPH_RECT, Size(7, 7), Point(-1, -1));
		dilate(thresh_img, thresh_img, element);	//����	
		erode(thresh_img, thresh_img, element);		//��ʴ

		imshow("threshold", thresh_img);

		/**************2.Ŀ�����*****************/
		vector<Vec4i> hie;
		vector<vector<Point>> contour;
		findContours(thresh_img, contour, hie, RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
		Mat mask(thresh_img.size(), CV_8UC1, Scalar::all(0));
		for (int i = 0; i < contour.size(); i++) {
			drawContours(mask, contour, i, Scalar(255), 1);
			divide(thresh_img, testimg, contour, i);
		}
		namedWindow("number");
		cout << "contour size " << contour.size() << endl;
		cout << "number size " << number.size() << endl;

		/********3.�����Ŀ�������ȡ�б�*********/
		int cflag[10] = { 0,0,0,0,0,0,0,0,0,0 };//ÿ������ֻ��һ������ʶ��
		int maxarea = -1;//����������
		int maxindex;//�����������±�
		for (int i = 0; i < number.size(); i++) {

			int result1 = -1;
			int result2 = -1;
			int result = -1;
			vector<int> testfea1;
			vector<int> testfea2;
			testfea1.clear();
			testfea2.clear();

			//��ȡ��������
			double an = angles[i];
			Mat rot;
			if (fabs(an) != 0 && fabs(an) != 90) {	//�����б���Ƚ���
				an = (90 - fabs(an)) < fabs(an) ? (90 - fabs(an)) : an;
				rot = rotate(number[i], an);
				extra(rot);
			}
			else {	//����ֱ������
				extra(number[i]);
			}
			if (recs[i])
			{
				cout << "the rec index is " << i << " hiac: ";
				for (int p = 0; p < 4; p++) {
					cout << hie[i][p] << " ";
				}
				cout << endl;

				Rect rmr = boundingRect(Mat(contour[index[i]]));
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
				if (maxarea == -1 || maxarea < areas[i]) { maxarea = areas[i]; maxindex = i; }
				continue;
			}
			else {
				borders1(rots[i], testfea1);
				borders2(rots[i], testfea2);
			}


			//������������γ�����������Ԥ�Ⲣ�����Χ
			if (testfea1.size() == 191 && testfea2.size() == 166) {

				Rect mr = boundingRect(Mat(contour[index[i]]));
				Point2f v[4];
				int cenx, ceny;
				cenx = (mr.br().x + mr.tl().x) / 2;
				ceny = (mr.br().y + mr.tl().y) / 2;
				Point center = Point((mr.br().x + mr.tl().x) / 2, (mr.br().y + mr.tl().y) / 2);


				if (centers.size() != 0) {
					if (check(center, mr.area(), mr.size().width))continue;	//�����������������޳���������� *��֦7
				}

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
				}
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

					///****************************************/
					//writeimg(rots[i]);
					///****************************************/

					std::cout << "the index is " << i << " the result is " << result << endl;
					cout << "number hia: ";
					for (int p = 0; p < 4; p++) {
						cout << hie[i][p] << " ";
					}
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

			//������Ϳ��
			Scalar s = sum(thresh_img(rmr));
			float fillrate = 100 * s[0] / (255 * rmr.area());//ԭʼͼ���ɫ����
			d->SprayScale = (unsigned char)(int)fillrate;
			cout << "spray fillrate:" << (int)d->SprayScale << endl;

		}
		cout << "***********************" << endl << endl;
		imshow("img", testimg);
		waitKey(10);
		float tstop = clock();
		cout << "time:" << (tstop - tstart) / CLOCKS_PER_SEC << endl;
	}
	std::system("pause");
}
