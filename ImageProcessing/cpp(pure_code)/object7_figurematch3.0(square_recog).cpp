#include <opencv2\opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::ml;

vector<Mat> number;//�洢��ֶ���(δУ��)
vector<int> index;//�洢�Ϸ������±�
vector<double> angles;//����ת��
vector<int> areas;//�������
vector<Mat> rots;//������ת������Ķ���

int flag = 0;


//�������
void divide(Mat& img, Mat& org, vector<vector<Point>> contour, int i) {

	Rect rec = boundingRect(contour[i]);	//��������������������
	if (rec.area() < (org.rows*org.cols) / 30.0) return;  //���������򲻷�����
	Point2f v2[4];
	v2[0] = rec.tl();
	v2[1].x = rec.tl().x;
	v2[1].y = rec.br().y;
	v2[2] = rec.br();
	v2[3].x = rec.br().x;
	v2[3].y = rec.tl().y;

	RotatedRect mr = minAreaRect(contour[i]);

	Mat show;//�������������
	show = img(rec);
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
void borders(Mat img, vector<int>&fea) {
	vector<int> border;//�洢�߽�����
	vector<int> rborder;
	int i = 0;
	border.clear();
	rborder.clear();
	fea.clear();
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

	for (int j = 0; j < rborder.size(); j++) border.push_back(rborder[j]);	//���ұ߽�ƴ��

	int count = 0;	//ȷ��
	for (int j = 0; j < border.size(); j++) if (!border[j])count++;
	if (count > border.size() / 5.0) {
		cout << "invilid border" << endl;
		return;
	}

	for (int j = 0; j < border.size() - 1; j++) {	//������������
		int a = border[j + 1] - border[j];
		fea.push_back(a);
	}

}

//����ʶ��
bool rectan(vector<int> fea) {
	int count = 0;
	for (int i = 0; i < fea.size(); i++) {
		if (abs(fea[i] <= 1))count++;
	}
	if (count >= fea.size() - 5)return 1;
	return 0;
}

int main() {

	Ptr<SVM> svm;
	svm = StatModel::load<SVM>("svmtrain9.18.txt");
	std::cout << "file open successfully" << endl;

	VideoCapture cap(1);
	while (1) {

		//1.Ԥ����
		number.clear();
		index.clear();
		angles.clear();
		areas.clear();
		rots.clear();
		Mat testimg;
		cap.read(testimg);
		Mat img1;
		medianBlur(testimg, testimg, 7);
		cvtColor(testimg, img1, CV_BGR2GRAY);
		Mat thresh_img;

		threshold(img1, thresh_img, 0, 255, CV_THRESH_OTSU);//otsu��ֵ�ָ�
		threshold(thresh_img, thresh_img, 150, 255, THRESH_BINARY_INV);//����
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
		namedWindow("number");
		cout << "contour size " << contour.size() << endl;
		cout << "number size " << number.size() << endl;

		//3.�����Ŀ�������ȡ�б�
		int cflag = 1;//ÿֻ֡��һ������ʶ��
		int maxarea = -1;//����������
		int maxindex;//�����������±�
		for (int i = 0; i < number.size(); i++) {
			int result = -1;
			vector<int> testfea;
			testfea.clear();

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
			borders(rots[i], testfea);

			//�ж��Ƿ�Ϊ���β�ѡȡ����������
			if (rectan(testfea)) {
				if (maxarea == -1 || maxarea < areas[i]) { maxarea = areas[i]; maxindex = i; }
			}
			cout << "size " << testfea.size() << endl;

			//������������γ�����������Ԥ�Ⲣ�����Χ
			if (testfea.size() == 95 && cflag) {
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
				cflag = 0;

			}
		}
		if (maxarea != -1) {
			RotatedRect rmr = minAreaRect(contour[index[maxindex]]);
			Point2f rv[4];
			rmr.points(rv);
			for (int u = 0; u < 4; u++)
				line(testimg, rv[u], rv[(u + 1) % 4], Scalar(255, 0, 0), 2);
		}

		cout << "***********************" << endl << endl;

		imshow("img", testimg);
		waitKey(10);
	}
	std::system("pause");
}
/*��Ƶ����+������rentangle�궨*/