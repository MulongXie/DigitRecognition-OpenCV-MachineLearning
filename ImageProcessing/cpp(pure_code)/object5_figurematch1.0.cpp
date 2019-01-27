#include <opencv2\opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::ml;

vector<Mat> number;//�洢��ֶ���
vector<int> index;//�洢�Ϸ������±�
vector<int> border;//�洢�߽�����
vector<int> rborder;
int flag = 0;


//�������
void divide(Mat& img, Mat&org, vector<vector<Point>> contour, int i) {
	Point2f v[4];
	Rect mr = boundingRect(Mat(contour[i]));
	if (mr.area() < (org.rows*org.cols) / 30.0 || mr.area() >= (org.rows*org.cols) / 3.0) return;  //���������򲻷�����

	v[0] = mr.tl();
	v[1].x = mr.tl().x;
	v[1].y = mr.br().y;
	v[2] = mr.br();
	v[3].x = mr.br().x;
	v[3].y = mr.tl().y;
	/*	for (int i = 0; i < 3; i++) {
	line(org, v[i], v[i + 1], Scalar(0, 0, 255), 2);
	}
	line(org, v[3], v[0], Scalar(0, 0, 255), 2);*/

	Mat show;//�������������
	show = img(mr);
	resize(show, show, Size(50, 96));
	number.push_back(show);
	index.push_back(i);

	/*	string s = to_string(flag);
	flag++;
	imshow(s, show);*/
}

//����߽�����
void borders(Mat img, vector<int>&fea) {
	int i = 0;
	border.clear();
	rborder.clear();
	while (i < img.rows) {	//�ֳ�48��������ұ߽�
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
	border.push_back(0);
	for (int k = 0; k < rborder.size(); k++) border.push_back(rborder[k]);	//���ұ߽�ƴ��

	int count = 0;	//ȷ��
	for (int k = 0; k < border.size(); k++) if (!border[k])count++;
	if (count > border.size() / 5.0) {
		cout << "invilid border" << endl;
		return;
	}

	for (int k = 0; k < border.size() - 1; k++) {	//������������
		int a = border[k + 1] - border[k];
		fea.push_back(a);
	}

	//	for (int k = 0; k < fea.size(); k++)printf("%d\n", fea[k]);
}


int main() {

	Ptr<SVM> svm;
	svm = StatModel::load<SVM>("svmtrain.txt");
	std::cout << "file open successfully" << endl;

	VideoCapture cap(1);
	while (1) {

		//1.Ԥ����
		number.clear();
		index.clear();
		//		Mat testimg = imread("test6.png");
		Mat testimg;
		cap.read(testimg);
		Mat img1;
		medianBlur(testimg, testimg, 7);
		cvtColor(testimg, img1, CV_BGR2GRAY);
		Mat thresh_img;

		threshold(img1, thresh_img, 150, 255, THRESH_BINARY_INV);//����ͼƬΪ��ɫ����
		imshow("threshold", thresh_img);

		//2.Ŀ�����
		vector<Vec4i> hie;
		vector<vector<Point>> contour;
		findContours(thresh_img, contour, hie, RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
		Mat mask(thresh_img.size(), CV_8UC1, Scalar::all(0));
		for (int i = 0; i < contour.size(); i++) {
			drawContours(mask, contour, i, Scalar(255));
			divide(thresh_img, testimg, contour, i);
		}
		imshow("contour", mask);
		cout << "contour size " << contour.size() << endl;
		cout << "number size " << number.size() << endl;

		//3.�����Ŀ�������ȡ�б�
		for (int i = 0; i < number.size(); i++) {

			int result = -1;
			vector<int> testfea;
			testfea.clear();
			borders(number[i], testfea);

			Mat testdata = Mat::zeros(1, testfea.size(), CV_32F);
			for (int j = 0; j < testfea.size(); j++) {
				testdata.at<float>(0, j) = testfea[j];
			}
			cout << "size " << testfea.size() << endl;
			if (testfea.size() == 96) {
				result = svm->predict(testdata);

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


			}
		}
		cout << "***********************" << endl << endl;

		imshow("img", testimg);
		waitKey(10);
	}
	system("pause");
}