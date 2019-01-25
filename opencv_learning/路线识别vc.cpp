#include "EasyTrace.h"
#include "LCD.h"

float theabs(int a) {		//绝对值
	if (a >= 0)return a;
	else return -a;
}

float distan(int x1, int y1, int x2, int y2) {		//两点距离
	float r = 0;
	int a = (x1 - x2)*(x1 - x2);
	int b = (y1 - y2)*(y1 - y2);
	r = sqrt(a + b);
	return r;
}

float aera(int x1, int y1, int x2, int y2, int x3, int y3) {		//计算面积
	float a;
	a = 0.5*(x1*y2 + x2*y3 + x3*y1 - x1*y3 - x2*y1 - x3*y2);
	a = theabs(a);
	return a;
}

float curvature(int x1, int y1, int x2, int y2, int x3, int y3) {		//计算曲率
	float r;
	float a, b, c, aeras;
	aeras = aera(x1, y1, x2, y2, x3, y3);
	a = distan(x1, y1, x2, y2);
	b = distan(x2, y2, x3, y3);
	c = distan(x3, y3, x1, y1);
	float x;
	x = (a*b*c) / (4 * aeras);
	//	printf("aera:%f a:%f b:%f c:%f rido:%f\n", aeras, a, b, c, x);
	r = (4 * aeras) / (a*b*c);
	return r;
}


typedef struct
	  {
    unsigned char  red;             // [0,255]
    unsigned char  green;           // [0,255]
    unsigned char  blue;            // [0,255]
    }COLOR_RGB;//RGB格式颜色

typedef struct
	  {
    unsigned char  gray;             // [0,255]
    }COLOR_GRAY;//灰度图格式


//读取RBG格式颜色，唯一需要移植的函数
extern unsigned short LCD_ReadPoint(unsigned short x,unsigned short y);//读某点颜色

//RGB转GRAY
static void RGBtoGRAY(unsigned short *_acAA,unsigned char *grayImg,int width,int height)		//读取并转换为灰度图    
{
		unsigned short C16;
		unsigned char  r,g,b,gray;
		int i,size=width*height;
		unsigned char* pCur=_acAA,*pEnd=_a

		for(i=0;i<size;i++)
		{
			C16 = _acAA[i];     //读某点颜色
			r =	 (unsigned char)((C16&0xf800)>>8);
			g =	 (unsigned char)((C16&0x07e0)>>3);
		    b =  (unsigned char)((C16&0x001f)<<3);
			gray=(unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
			grayImg[i]= gray;
		}
}

//均值滤波
static void AverageFilter(unsigned char *GrayImg,int width,int height,int FilterSize,unsigned char *AveImg)
{
	int x,y,i,j,g;
	int nx=FilterSize/2;
	int ny=FilterSize/2;
	int size=(nx*2+1)*(ny*2+1);
	memcpy(AveImg,GrayImg,width*height);
	unsigned char * pCur;
	for(y=ny,x=nx,pCur=AveImg+y*width;y<height-ny;y++)
	{
		//计算sum
		pCur+=nx;
		int sum=0;
		for(i=y-ny;i<y+ny+1;i++)
		{
			for(j=x-nx;j<x+nx+1;j++)
			{
				unsigned char *pSum=AveImg+i*width+j;
				sum+=*pSum;
			}
		}
		*(pCur++)=sum/size;
		//计算下一列的均值
		for(x=nx+1;x<width-nx;x++)
		{
			int n;
			//-L+R
			unsigned char *pl,*pr;
		    for(n=y-ny,pl=AveImg+n*width+(x-nx-1),pr=AveImg+n*width+(x+nx); n<y+ny+1; n++,pl+=width,pr+=width)
			{
				sum-=*pl;
				sum+=*pr;
			}
			*(pCur++)=sum/size;
		}
		pCur+=nx;
	}
	return;
}
//中值滤波，较均值效果更佳
static void MedianFilter(unsigned char *GrayImg,int width,int height,int FilterSize,unsigned char *MedImg)
{
	int x,y,i,j,hist[256],med,sum;
	int nx=FilterSize/2;
	int ny=FilterSize/2;
	int size=(nx*2+1)*(ny*2+1);
	unsigned char *pCur;
	memcpy(MedImg,GrayImg,width*height);
	//计算直方图
	for(y=ny,x=nx,pCur=MedImg+y*width+x;y<height-ny;y++)
	{
		memset(hist,0,sizeof(hist));
		for(j=y-ny;j<y+ny+1;j++)
		{
			for(i=0;i<FilterSize;i++)
			{
				unsigned char *pHist=MedImg+j*width+i;
				hist[*pHist]++;
			}
		}
		//计算中值med
		for(med=0,sum=0;med<256;med++)
		{
			sum+=hist[med];
			if(sum*2>size)
				break;
		}
		*(pCur++)=med;
		//计算下一列的中值
		for(x=nx+1;x<width-nx;x++)
		{
			//-L+R
		    unsigned char *pl,*pr;
			int n;
			for(n=y-ny,pl=MedImg+n*width+(x-nx-1),pr=MedImg+n*width+(x+nx); n<y+ny+1; n++,pl+=width,pr+=width)
			{
				if(*pl<=med)
					sum--;
				if(*pr<=med)
					sum++;
				hist[*pl]--;
				hist[*pr]++;
			}
			//计算med
			if(sum*2>size)
			{
				for(;med>=0;med--)
				{
					sum-=hist[med];
					if(sum*2<=size) 
					{	
						sum+=hist[med];
						break;
					}
				}
			}
			else if(sum*2<size)
			{
				for(med=med+1;med<256;med++)
				{
					sum+=hist[med];
					if(sum*2>=size) break;
				}
			}
			*(pCur++)=med;
		}
		pCur+=2*nx;
	}
}
//边界处理
void SetBoundary(unsigned char *pGrayImg, int width, int height)
{
	unsigned char *pCur1, *pCur2;
	int x, y;
	for (x = 0, pCur1 = pGrayImg, pCur2 = pGrayImg + (height - 1)*width; x<width; x++)
		*(pCur1++) = *(pCur2++) = 0;
	for (y = 0, pCur1 = pGrayImg, pCur2 = pGrayImg + width - 1; y<height; pCur1 += width, pCur2 += width, y++)
		*pCur1 = *pCur2 = 0;
}
//Sobel算子边缘检测
void SobelFast(unsigned char * pGrayImg, int width, int height, unsigned char * pSobImg)
{
	int x, y, i, sum1, sum2;
	unsigned char *pCur, *pPre, *pNext, *pRes;
	//处理边界
	SetBoundary(pSobImg, width, height);
	//取绝对值表
	int abs[2048], *pAbs;
	pAbs = abs + 1024;
	for (i = 0; i<1024; i++)
	{
		if (i<256)
			pAbs[i] = pAbs[-i] = i;
		else
			pAbs[i] = pAbs[-i] = 255;
	}
	//求和表，min(255,abs(sum1)+abs(sum2));
	int sumLUT[512];
	for (i = 0; i<512; i++)
	{
		if (i<256)
			sumLUT[i] = i;
		else
			sumLUT[i] = 255;
	}
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1, pCur = pGrayImg + y*width + x, pPre = pCur - width, pNext = pCur + width, pRes = pSobImg + y*width + x; x<width - 1; x++, pCur++, pPre++, pNext++, pRes++)
		{
			sum1 = *(pPre - 1) + *(pCur - 1) * 2 + *(pNext - 1) - *(pPre + 1) - *(pCur + 1) * 2 - *(pNext + 1);
			sum2 = *(pPre - 1) + (*pPre) * 2 + *(pPre + 1) - *(pNext - 1) - (*pNext) * 2 - *(pNext + 1);
			//用查表法
			*pRes = sumLUT[pAbs[sum1] + pAbs[sum2]];
		}
	}
	return;
}
void ShenJunFast(unsigned char * pGrayImg, int width, int height, double alpha, unsigned char *pTmpImg,unsigned char * pShenImg, int minGrd)
{
	int x, y, i, j;
	unsigned char *p, *p1, *p2, *p4;
	unsigned char *pre, *pNext, *pEdg;
	int size=width*height;
	//初始化图像
	memcpy(pTmpImg,pGrayImg,size);
	memcpy(pShenImg,pGrayImg,size);
	//过零点表
	int crs[512], *pCross;
	pCross = crs + 256;
	memset(crs, 0, sizeof(crs));
	for (i = 0; i<256; i++)
		pCross[-i] = 1;
	//梯度有效表
	int grd[512], *pGrade;
	pGrade = grd + 256;
	memset(grd, 1, sizeof(grd));
	for (i = 0; i<minGrd; i++)
		pGrade[i] = pGrade[-i] = 0;
	//乘积表
	int multLUT[512], *Mul;
	Mul = multLUT + 256;
	for (Mul[0] = 0, i = 1; i<256; i++)
	{
		Mul[i] = (int)(alpha*i);
		Mul[-i] = (int)(-alpha*i) - 1;
	}
	//先进行四次扫描
	for (y = 1, p = pGrayImg + width, p1 = pTmpImg + width; y<height; y++)
	{
		*(p1++) = *(p++);
		//第1遍扫描p1(x,y)=p1(x-1,y)+a*[p(x,y)-p1(x-1,y)];
		for (x = 1, pre = p1 - 1; x<width; x++, p++, p1++, pre++)
			*p1 = (unsigned char)(*pre + Mul[*p - *pre]);
		//第2遍扫描p2(x,y)=p2(x+1,y)+a*[p1(x,y)-p2(x+1,y)];
		//第3遍扫描p3(x,y)=p3(x,y-1)+a*[p2(x,y)-p3(x,y-1)];
		for (x = width - 2, p2 = p1 - 2, pre = p2 + 1, pNext = pre - width; x >= 0; x--, p2--, pre--, pNext--)
		{
			*p2 = (unsigned char)(*pre + Mul[*p2 - *pre]);
			*pre = (unsigned char)(*pNext + Mul[*pre - *pNext]);
		}
		*pre = (unsigned char)(*pNext + Mul[*pre - *pNext]);
	}
	//第4遍扫描p4(x,y)=p4(x,y+1)+a*[p3(x,y)-p4(x,y+1)];
	for (pre = pTmpImg + height*width - 1, p4 = pre - width; p4 >= pTmpImg; p4--, pre--)
	{
		*p4 = (unsigned char)(*pre + Mul[*p4 - *pre]);
	}

	//过零点检测
	//先将pshenImg初始化为0								
	memset(pShenImg, 0, size);
	for (y = 1, p = pGrayImg + width, p4 = pTmpImg + width, pEdg = pShenImg + width; y<height - 1; y++, pEdg += width)
	{
		p++;
		p4++;
		for (x = 1; x<width - 1; x++, p++, p4++)
		{
			if (*p>*p4)//如果原图像大于沈俊图像
			{
				pre = p - width;
				pNext = p + width;
				//检测该点边缘是否有过零点
				if ((pCross[*(p - 1) - *(p4 - 1)] || pCross[*(p + 1) - *(p4 + 1)] ||
					pCross[*pre - *(p4 - width)] || pCross[*pNext - *(p4 + width)]
					) &&
					(//并且梯度足够大
					pGrade[*p - *(p + 1)] || pGrade[*p - *(p - 1)] ||
					pGrade[*p - *pre] || pGrade[*p - *pNext]))
				{
					*(pEdg + x) = 255;
				}
			}
		}
		p++;
		p4++;
	}
	return;
}
//将一阶算子和二阶算子的图像作与运算
void conjuction(unsigned char* pSobImg, unsigned char* pShenImg, unsigned char* pFinalImg, int width, int height, int thre)
{
	unsigned char *pSob, *pShen, *pFinal;
	int i, size = width*height;
	memcpy(pFinalImg,pShenImg,size);
	for (i = 0, pSob = pSobImg, pShen = pShenImg, pFinal = pFinalImg; i<size; i++, pSob++, pShen++, pFinal++)
	{
		if (*pShen && (*pSob>thre))   //如果沈俊图像的边缘点在sobel图像中的值大于一个值（77），则保留
			*pFinal = 255;
		else
			*pFinal = 0;
	}
}


int main() {			//inRange + video


	int thresh = 26;
	int min = 0;
	int max = 27;

	int lowc = 105;
	int highc = 500;
	
	int thre1 = 1;
	int thre2=0.5, thre3=80;


	while (1) {
		unsigned char* img;
		cap.read(img);//********读取图片***********
		unsigned char* gray;
		cvtColor(out, gray, CV_RGB2GRAY);//********
		unsigned char* edge;
		int wid = 300, hgt = 200;//********

		Canny(gray, edge, lowc, highc);			//边缘**********

		unsigned char* SobImg, shenImg;
		unsigned char* TmpImg, FinalImg;
		SobelFast(gray, wid, hgt, SobImg);
		ShenJunFast(gray, wid, hgt, thre1, TmpImg, shenImg, thre2);
		conjuction(SobImg, shenImg, FinalImg, wid, hgt, thre3);

		
        float slope;//斜率
		float cur;//曲率

		int x1 = -1, y1 = -1;//路径最高点
		int m1 = -1, n1 = -1, m2 = -1, n2 = -1, m3 = -1, n3 = -1;//计算曲率的三个点
		bool flag = 0;

        int draw[hgt+5];//********储存路径数组
		for(int i = 0; i < hgt + 5; i++) draw[i]=0;

        int iter=0;//********迭代记录
		for (int i = 0; i < hgt; i++) {				//画中线********** 
			int a1 = 0, a2 = 0;
			int mid;
			for (int j = iter; j < iter + wid; j++) {
				if (FinalImg!= 0) {//********   0? 
					if (a1 == 0)a1 = j;
					else { 
						if (j - a1 < 10)continue;
						else {
							a2 = j;
							mid = (a1 + a2) / 2;
							a1 = 0;
							draw[i] = j;//********画中线上的点
							if (!flag) {
								y1 = i;//最高点
								x1 = mid;
								flag = 1; 
								slope = theabs((float)x1 - (float)wid / 2.0) / (float)hgt;//计算斜率
								printf("slope:%f \n", slope);
							}
						}
					}
				}
			}
            iter += wid;
		}


		if (y1 > 0) {		//计算曲率
			m1 = y1;
			n1 = x1;//·第一个曲率点为draw1最高点
			for (int i = hgt / 4; i < hgt / 2; i++) {//·从draw1四分之一行开始寻找第二曲率点
				if (draw[i]) {
					m2 = i; n2 = draw[i]; flag = 1;
					break; 
				};
			}
			for (int i = wid / 2; i < wid; i++) {//·从draw1二分之一行开始寻找第三曲率点
				if (draw1[i]) {
					m3 = i; n3 = draw[i]; flag = 1;
					break; 
				};
			}
			cur = curvature(m1, n1, m2, n2, m3, n3);//计算曲率
			printf("curature:%f\n\n", cur);
		}

		printf("slope:%d cur:%d\n\n",slope,cur);

	}
	system("pause");
}
