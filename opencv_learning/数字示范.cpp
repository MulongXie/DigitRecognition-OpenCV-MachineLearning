#include<iostream>
#include<fstream>
#include<opencv2\opencv.hpp>
#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include <opencv2\objdetect\objdetect.hpp>
#include<opencv2\ml\ml.hpp>
#include<vector>
using namespace std;
using namespace cv;

#define TRAIN false   //是否进行训练,true表示重新训练，false表示读取xml文件中的SVM模型
void drawContours(Mat srcImg,Mat ROI)
{
	Mat gray, img;
	cvtColor(srcImg, gray, CV_BGR2GRAY);
	medianBlur(gray, img, 3);
	threshold(img, img, 0, 255, CV_THRESH_OTSU);
	//找出数字边缘
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//使用迭代器
	//画出包围数字的最小矩形
	int i;
	vector<vector<Point>>::iterator it;
	for (it = contours.begin(); it < contours.end(); it++)
	{
		Point2f vertex[4];
		Rect rect = boundingRect(*it);
		//左上角的点
		vertex[0] = rect.tl();
		//左下方的点
		vertex[1].x = (float)rect.tl().x;
		vertex[1].y = (float)rect.br().y;
		//右下角的点
		vertex[2] = rect.br();
		//右上方的点
		vertex[3].x = (float)rect.br().x;
		vertex[3].y = (float)rect.tl().y;
		for (i = 0; i < 4; i++)
			line(srcImg, vertex[i], vertex[(i + 1) % 4], Scalar(0, 255, 255), 1);
		int width = vertex[3].x - vertex[0].x;
		int height = vertex[1].y - vertex[0].y;
		Mat temp = gray(Rect(vertex[0].x, vertex[0].y, width, height));
		resize(temp, ROI, Size(28, 28), 0, 0, CV_INTER_LINEAR);
	}
	imshow("结果", srcImg);
	waitKey();
}
int main()
{
	Mat srcImg, grayImg;
	CvSVM svm;//创建一个SVM
	if (TRAIN)   //需要重新训练数据
	{
		/*****1.批量读取文件夹中的训练图片**********/
		vector<string> img_path;
		vector<int> img_label;
		int lines = 0;//用来标记图片路径的行数
		string buf;
		ifstream data("F:\\C++学习之路\\NumberReg\\NumberReg\\train.txt");
		while (data)
		{
			if (getline(data, buf))
			{
				lines++;
				if (lines % 2 == 0)//偶数行为数字标签，奇数行为图像路径
					img_label.push_back(atoi(buf.c_str()));
				else
				{
					img_path.push_back(buf);
				}
			}
		}
		data.close();//关闭文档
		cout << lines << endl;
	
		/*****2.提取图片中HOG特征**********/
		Mat featureMat, labelMat;
		int ImgNum = lines / 2;//图片训练样本数量
		labelMat = Mat::zeros(ImgNum, 1, CV_32FC1);
		for (string::size_type i = 0; i < img_path.size(); i++)
		{
			srcImg = imread(img_path[i].c_str());
			cvtColor(srcImg, grayImg, CV_BGR2GRAY);
			HOGDescriptor *hog = new HOGDescriptor(cvSize(28, 28), cvSize(14, 14), cvSize(7, 7), cvSize(7, 7), 9);
			vector<float> descriptors;//存储结果
			hog->compute(grayImg, descriptors, Size(1, 1), Size(0, 0));//计算HOG特征
			
			if (i == 0)
				featureMat = Mat::zeros(ImgNum, descriptors.size(), CV_32FC1);
			//cout << "HOG dims" << descriptors.size() << endl;
			unsigned long n = 0;
			for (vector<float>::iterator iter = descriptors.begin(); iter != descriptors.end(); iter++)
			{
				featureMat.at<float>(i, n) = *iter;
				n++;
			}
			labelMat.at<float>(i, 0) = img_label[i];

		}
		/*****3.用HOG特征开始SVM训练*********************/
		CvSVMParams param;//存储训练的相关参数
		CvTermCriteria criteria;
		criteria = cvTermCriteria(CV_TERMCRIT_EPS, 1000, FLT_EPSILON);
		param = CvSVMParams(CvSVM::C_SVC, CvSVM::RBF, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria);
		cout << "Start Training" << endl;
		svm.train(featureMat, labelMat, Mat(), Mat(), param);
		svm.save("SVM_TRAIN.xml"); //将训练结果保存
		cout << "Training is Over!" << endl;
	}
	else
	{
		svm.load("SVM_TRAIN.xml");
	}

	/*****4.用测试图片检测该训练数据的准确性**********/
	//读取test文本里面的测试图片路径

	vector<string> test_path;
	vector<int> test_label;
	int testlines = 0;//用来标记图片路径的行数
	string testbuf;
	ifstream testdata("F:\\C++学习之路\\NumberReg\\NumberReg\\test.txt");
	while (testdata)
	{
		if (getline(testdata, testbuf))
		{
			testlines++;
			if (testlines % 2 == 0)//偶数行为数字标签，奇数行为图像路径
				test_label.push_back(atoi(testbuf.c_str()));
			else
			{
			test_path.push_back(testbuf);
			}
		}
	}
	testdata.close();//关闭文档
	//用SVM训练样本预测结果
	ofstream predict("predict.txt");
	Mat testFeature;
	int testNum = testlines / 2, rightNum = 0;
	for (string::size_type j = 0; j != test_path.size(); j++)
	{
		srcImg = imread(test_path[j].c_str());
		cvtColor(srcImg, grayImg, CV_BGR2GRAY);
		HOGDescriptor *hog = new HOGDescriptor(cvSize(28, 28), cvSize(14, 14), cvSize(7, 7), cvSize(7, 7), 9);
		vector<float> descriptors;//存储结果
		hog->compute(grayImg, descriptors, Size(1, 1), Size(0, 0));//计算HOG特征
		
		testFeature = Mat::zeros(1, descriptors.size(), CV_32FC1);
		//cout << "HOG dims" << descriptors.size() << endl;
		unsigned long n = 0;
		for (vector<float>::iterator iter = descriptors.begin(); iter != descriptors.end(); iter++)
		{
			testFeature.at<float>(0, n) = *iter;
			n++;
		}
		int temp = svm.predict(testFeature);
		char result[100];
		sprintf(result, "%s %d\n", test_path[j].c_str(), temp);
		predict << result;
		if (temp == test_label[j])
			rightNum++;
	}
	cout << "正确率为:" << 1.0*rightNum / testNum << endl;
	predict.close();
	
/*****5.开启摄像头读入图片进行数字识别**********/
	Mat frame = imread("F:\\C++学习之路\\NumberReg\\NumberReg\\1.jpg");
	Mat ROIImg(28,28,CV_8UC1);
	drawContours(frame, ROIImg);
	imshow("ROI", ROIImg);
	waitKey();
	HOGDescriptor *hog = new HOGDescriptor(cvSize(28, 28), cvSize(14, 14), cvSize(7, 7), cvSize(7, 7), 9);
	vector<float> descriptors;//存储结果
	hog->compute(ROIImg, descriptors, Size(1, 1), Size(0, 0));//计算HOG特征
	Mat resultFeature = Mat::zeros(1, descriptors.size(), CV_32FC1);
	for (int i = 0; i < descriptors.size(); i++)
	resultFeature.at<float>(0, i) = descriptors[i];
	int x = svm.predict(resultFeature);
	cout << "该数字为：" << x << endl;
	
	system("pause");
	return 0;
}