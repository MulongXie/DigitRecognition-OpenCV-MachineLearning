#pragma once
#pragma once
#include "opencv2\core\core.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\imgproc.hpp"
#include <iostream>

using namespace std;
using namespace cv;

typedef unsigned char uint8_t;
#ifndef uint8
#define uint8  unsigned char
#endif
#ifndef s16
#define s16 short
#endif
#ifndef BYTE
#define BYTE unsigned char
#endif

extern int flag;

typedef struct tagUav2Img_INFO
{
	uint8 Head; //	uint8_t FrameID; //֡ID��ÿ��һ�Σ����һ
	uint8 Key; //֡�ؼ��� ���ݶ�Ϊ��ֵ 	2
	uint8 DigitName; //������Ϳ������ID��
	s16 DisImg_mm;  // ��������ֵ����
	s16 AngleImg_mrad;	// �������Ļ�ļн� ��λ ������ 
	s16 TargSprayPoint_x_mm;  // ��Ϳ���������ʶ������X
	s16 TargSprayPoint_y_mm;  // ��Ϳ���������ʶ������Y
	uint8 Spraygun_status;   //0,3-close 1-ready 2-running 3-finish
	uint8 tmp1; // Ԥ��
	uint8 tmp2; // Ԥ��
}Uav2Img_INFO;

typedef struct tagDigit_POINT
{
	unsigned char ImgStatue;//  ͼ��ʶ��״̬  0��û��ʶ��ɹ�  1ʶ��ɹ� 2 ʶ����������
	unsigned char DigitName = 0; // ��ɫֵ  0��ʶ����  1,2,3��RGB
	short int xPiont_mm = 0;  // ��������λ�õ�x����ֵ ��λmm  ����ͼ
	short int yPiont_mm = 0;  // ��������λ�õ�y����ֵ ��λmm
}Digit_POINT;

typedef struct tagDigit_INFO
{
	unsigned char Head = 36; //�ݶ�Ϊ 36
	unsigned char FrameID = flag; //֡ID��ÿ��һ�Σ����һ
	unsigned char Key = 1; //֡�ؼ��� ���ݶ�Ϊ��ֵ 	1

	unsigned char Spray_Statue = 0; //����Ϳ��ʶ��״̬ 0��ʶ��ʧ�� 1��ʶ��ɹ�
	unsigned char SprayScale = 0; //��ǰ��Ϳ������С  0-100%
	unsigned char ShotPoint_Statue = 0; //������׼��ʶ��״̬ 0��ʶ��ʶ�� 1��ʶ��ɹ� 
	short int ShotPoint_x_mm = 0;  // ������׼�� ����X
	short int ShotPoint_y_mm = 0;  // ������׼�� ����Y


	Digit_POINT tagDigit_POINT[11]; //tagDigit_POIN[0]���ȴ�����tagDigit_POIN[1]- tagDigit_POIN[5]���ĸ���λ��  11*6=66bytes
	uint8_t  tmp1 = 0; // ��head��CheckSum֮ǰ���������ݰ����޷����ֽڼӺ͵ĵͰ�λ������CheckSum�Ա�У��
	uint8_t  tmp2 = 0; // Ԥ��ֵ0
}Digit_INFO;  // һ�� 78bytes
