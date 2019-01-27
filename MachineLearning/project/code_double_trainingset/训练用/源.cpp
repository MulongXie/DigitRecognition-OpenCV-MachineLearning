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
vector<double> angles;//�������
vector<Mat> rots;//����������ͼ��
vector<int> border;//�洢�߽�����
vector<int> rborder;
vector<int> topborder;
vector<int> botborder;
int flag = 0;


//�������
void divide(Mat& img, vector<vector<Point>> contour, int i) {
	Point2f v[4];
	Rect mr = boundingRect(Mat(contour[i]));

	if (mr.area() < img.rows*img.cols / 20) {
		cout << "too small************************" << endl; return;
	}

	v[0] = mr.tl();
	v[1].x = mr.tl().x;
	v[1].y = mr.br().y;
	v[2] = mr.br();
	v[3].x = mr.br().x;
	v[3].y = mr.tl().y;

	RotatedRect rmr = minAreaRect(contour[i]);

	Mat show;//�������������
	show = img(mr);
	resize(show, show, Size(70, 96));
	number.push_back(show);
	angles.push_back(rmr.angle);

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
	//	cout << v[0].y << " " << v[0].x << " " << v[1].y - v[0].y << " " << v[3].x - v[0].x << endl;
	Rect rec(v[0].y, v[0].x, v[1].y - v[0].y, v[3].x - v[0].x);
	//	Rect rec(10, 0, 50, 80);
	Mat show = img(rec);
	resize(show, show, Size(70, 96));
	rots.push_back(show);
	return show;
}

////����߽�����
//void borders(Mat img, vector<int>&fea) {
//	int i = 0;
//	border.clear();
//	rborder.clear();
//	topborder.clear();
//	botborder.clear();
//	fea.clear();
//	while (i < img.rows) {	//�ֳ�96��������ұ߽�
//		for (int j = 0; j < img.cols; j++)
//			if (img.at<uchar>(i, j)) {
//				border.push_back(j);
//				break;
//			}
//		for (int j = img.cols - 1; j >= 0; j--)
//			if (img.at<uchar>(i, j)) {
//				rborder.push_back(img.cols - j);
//				break;
//			}
//		i++;
//	}
//
//	/*	int k = 0;
//	while (k < img.cols) {
//	for (int j = 0; j < img.rows; j++)
//	if (img.at<uchar>(j, k)) {
//	topborder.push_back(j);
//	break;
//	}
//	for (int j = img.rows - 1; j >= 0; j--)
//	if (img.at<uchar>(j, k)) {
//	botborder.push_back(img.rows - j);
//	break;
//	}
//	k += 2;
//	}*/
//
//	for (int j = 0; j < rborder.size(); j++) border.push_back(rborder[j]);	//���ұ߽�ƴ��
//																			/*	for (int j = 0; j < topborder.size(); j++)border.push_back(topborder[j]);
//																			for (int j = 0; j < botborder.size(); j++)border.push_back(botborder[j]);*/
//
//	for (int j = 0; j < border.size() - 1;) {	//������������
//		int a = border[j + 1] - border[j];
//		if (abs(a) > 10) {
//			for (int x = 0; x < 3; x++) {
//				fea.push_back(a);
//				j++;
//				if (j >= border.size() - 1)break;
//			}
//		}
//		else
//		{
//			fea.push_back(a); j++;
//		}
//	}
//
//	//	for (int k = 0; k < fea.size(); k++)printf("%d\n", fea[k]);
//}


//����߽�����
void borders(Mat img, vector<int>&fea) {
	int i = 0;
	border.clear();
	rborder.clear();
	topborder.clear();
	botborder.clear();
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
																			/*	for (int j = 0; j < topborder.size(); j++)border.push_back(topborder[j]);
																			for (int j = 0; j < botborder.size(); j++)border.push_back(botborder[j]);*/

	for (int j = 0; j < border.size() - 1;) {	//������������
		int a = border[j + 1] - border[j];
		if (abs(a) > 10) {
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
	}
}

//��ͶӰ����Ϊ������
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

//��ȡ����
void feature(Mat img, vector<int>&fea,vector<int>&fea2) {
	//step1.����Ŀ��
	vector<Vec4i> hie;
	vector<vector<Point>> contour;
	findContours(img, contour, hie, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	Mat mask(img.size(), CV_8UC1, Scalar::all(0));
	for (int i = 0; i < contour.size(); i++) {
		drawContours(mask, contour, i, Scalar(255));
		divide(img, contour, i);
	}

	//step2.������ȡ
	printf("num of object: %d \n", number.size());
	if (number.size() > 1) {	//��һ����ͼ��ȷ�ϣ�ֻ����ѵ��
		printf("\n**********invalid**********\n");
		return;
	}

	Mat rot = rotate(img, angles[0]);
	double an = angles[0];
	if (fabs(an) != 0 && fabs(an) != 90) {	//�����б���Ƚ���
		an = (90 - fabs(an)) < fabs(an) ? (90 - fabs(an)) : an;
		rot = rotate(number[0], an);
		extra(rot);
	}
	else {	//����ֱ������
		extra(number[0]);
	}
	borders(rots[0], fea);
	borders2(rots[0], fea2);
	//imshow("rot", rots[0]);

	/*	for (int i = 0; i < number.size(); i++) {
	printf("****************%d*****************\n",i);
	borders(number[i], fea);
	}*/
}

int main() {
	//1.������������
	Ptr<SVM> svm;
	Ptr<SVM> svm2;
	bool train = 1;  //0Ϊ���������ļ���1Ϊ����ѵ��
	if (train) {
		svm = SVM::create();
		svm2 = SVM::create();
		int row = 1;
		vector<string> path;
		vector<int> trainlabel;
		string str;
		ifstream data("D:\\train\\train.txt");
		while (data) {
			if (getline(data, str)) {
				if (row % 2 == 0) trainlabel.push_back(atoi(str.c_str()));
				else path.push_back(str);
				row++;
			}
		}
		data.close();
		printf("path size:%d\n", path.size());
		for (int i = 0; i < path.size(); i++)cout << path[i] << endl;

		//2.����������ȡ�������
		Mat labelmat(trainlabel.size(), 1, CV_32SC1);
		Mat labelmat2(trainlabel.size(), 1, CV_32SC1);
		Mat datamat;
		Mat datamat2;
		int imgnum = path.size();
		for (int i = 0; i < path.size(); i++) {
			number.clear();
			angles.clear();
			rots.clear();
			cout << "path" << path[i] << endl;
			Mat img = imread(path[i]);//����ʼ
			Mat img1;

			cvtColor(img, img1, CV_BGR2GRAY);
			Mat thresh_img;
			threshold(img1, thresh_img, 180, 255, THRESH_BINARY_INV);//����ͼƬΪ��ɫ����

			vector<int> fea;//��ȡ����
			vector<int> fea2;
			feature(thresh_img, fea,fea2);
			printf("********** time:%d  libel:%d ************\n", i, trainlabel[i]);
			printf("feasize:%daaaaaaaaaaaaa%d\n", fea.size(),fea2.size());

			if (i == 0)datamat = Mat::zeros(imgnum, fea.size(), CV_32FC1);//��ʼ����������
			for (int j = 0; j < fea.size(); j++) {
				datamat.at<float>(i, j) = fea[j];
				//				cout << datamat.at<float>(i, j) << endl;
			}
			labelmat.at<int>(i, 0) = trainlabel[i];
			cout << "label:" << labelmat.at<int>(i, 0) << endl << endl;

			if (i == 0)datamat2 = Mat::zeros(imgnum, fea2.size(), CV_32FC1);//��ʼ����������
			for (int j = 0; j < fea2.size(); j++) {
				datamat2.at<float>(i, j) = fea2[j];
				//				cout << datamat.at<float>(i, j) << endl;
			}
			labelmat2.at<int>(i, 0) = trainlabel[i];
			cout << "label:" << labelmat2.at<int>(i, 0) << endl << endl;
		}

		//3.��������
		svm->setType(SVM::C_SVC);
		svm->setKernel(SVM::LINEAR);
		svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

		svm2->setType(SVM::C_SVC);
		svm2->setKernel(SVM::LINEAR);
		svm2->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));

		//4.����ѵ�����洢
		svm->train(datamat, ROW_SAMPLE, labelmat);
		svm->save("svmtrain11.4.1.txt");
		cout << "training accomplish" << endl;

		svm->train(datamat2, ROW_SAMPLE, labelmat2);
		svm->save("svmtrain11.4.2.txt");
		cout << "training accomplish" << endl;
	}
	else {
		svm = StatModel::load<SVM>("svmtrain11.4.1.txt");
		svm2 = StatModel::load<SVM>("svmtrain11.4.2.txt");
		cout << "file open successfully" << endl;
	}


	//5.���������������Ԥ��
	number.clear();
	rots.clear();
	angles.clear();
	Mat testimg = imread("91.png");
	Mat img1;
	cvtColor(testimg, img1, CV_BGR2GRAY);
	Mat thresh_img;
	threshold(img1, thresh_img, 150, 255, THRESH_BINARY_INV);//����ͼƬΪ��ɫ����
	imshow("aaa", thresh_img);

	int result = -1;
	int result2 = -1;
	vector<int> testfea;
	vector<int> testfea2;
	feature(thresh_img, testfea,testfea2);

	Mat testdata = Mat::zeros(1, testfea.size(), CV_32F);
	for (int i = 0; i < testfea.size(); i++) {
		testdata.at<float>(0, i) = testfea[i];
		cout << testdata.at<float>(0, i) << endl;
	}


	Mat testdata2 = Mat::zeros(1, testfea2.size(), CV_32F);
	for (int i = 0; i < testfea2.size(); i++) {
		testdata2.at<float>(0, i) = testfea2[i];
		cout << testdata2.at<float>(0, i) << endl;
	}

	//result = svm->predict(testdata);
	//result2 = svm2->predict(testdata2);

	cout << "fea size " << testfea.size() << "  and   "<< testfea2.size() << endl;
	cout << "the result is " << result<<"  and   "<<result2<< endl;

	waitKey(0);
	system("pause");
}
/*��ת����ѵ��*/
/*�����������*//*���ѵ����ȥ��*//*����γ�ȣ�191��*//*��תѵ����*/