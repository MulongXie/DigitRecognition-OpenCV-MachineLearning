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
	uint8 Head; //	uint8_t FrameID; //帧ID，每发一次，会加一
	uint8 Key; //帧关键字 ，暂定为常值 	2
	uint8 DigitName; //正在喷涂的数字ID号
	s16 DisImg_mm;  // 相机到数字的深度
	s16 AngleImg_mrad;	// 相机与屏幕的夹角 单位 毫弧度 
	s16 TargSprayPoint_x_mm;  // 喷涂区面积比例识别坐标X
	s16 TargSprayPoint_y_mm;  // 喷涂区面积比例识别坐标Y
	uint8 Spraygun_status;   //0,3-close 1-ready 2-running 3-finish
	uint8 tmp1; // 预留
	uint8 tmp2; // 预留
}Uav2Img_INFO;

typedef struct tagDigit_POINT
{
	unsigned char ImgStatue;//  图像识别状态  0：没有识别成功  1识别成功 2 识别质量不高
	unsigned char DigitName = 0; // 颜色值  0：识别区  1,2,3：RGB
	short int xPiont_mm = 0;  // 数字中心位置的x坐标值 单位mm  如下图
	short int yPiont_mm = 0;  // 数字中心位置的y坐标值 单位mm
}Digit_POINT;

typedef struct tagDigit_INFO
{
	unsigned char Head = 36; //暂定为 36
	unsigned char FrameID = flag; //帧ID，每发一次，会加一
	unsigned char Key = 1; //帧关键字 ，暂定为常值 	1

	unsigned char Spray_Statue = 0; //对喷涂区识别状态 0：识别失败 1：识别成功
	unsigned char SprayScale = 0; //当前喷涂比例大小  0-100%
	unsigned char ShotPoint_Statue = 0; //红外瞄准点识别状态 0：识别识别 1：识别成功 
	short int ShotPoint_x_mm = 0;  // 红外瞄准点 坐标X
	short int ShotPoint_y_mm = 0;  // 红外瞄准点 坐标Y


	Digit_POINT tagDigit_POINT[11]; //tagDigit_POIN[0]：等待区；tagDigit_POIN[1]- tagDigit_POIN[5]：四个靶位区  11*6=66bytes
	uint8_t  tmp1 = 0; // 从head到CheckSum之前的所有数据按照无符号字节加和的低八位，存在CheckSum以便校验
	uint8_t  tmp2 = 0; // 预设值0
}Digit_INFO;  // 一共 78bytes
