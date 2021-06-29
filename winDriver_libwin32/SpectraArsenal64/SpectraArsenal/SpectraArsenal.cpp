/*******************************************************************************************************
**
** Copyright (C), 2011-2012, Nahon photoelectric technology CO., LTD.
** File Name:               SpectraArsenal.cpp
** Description:             
** Created By:             Monster
**
*********************************************************************************************************/
#include <windows.h>
#include "stdafx.h"
#include "stdlib.h"
#include "malloc.h"
#include "stdio.h"
#include "USB_Driver.h"
#include "MIGP.h"
#include "Com_Driver.h"
#include "SpectraArsenal.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

const char acAPIVersion[] = "V6.11";
const char acComName[32][14] = 
{
	"COM1",
	"COM2",
	"COM3",
	"COM4",
	"COM5",
	"COM6",
	"COM7",
	"COM8",
	"COM9",
	"\\\\.\\COM10",
	"\\\\.\\COM11",
	"\\\\.\\COM12",
	"\\\\.\\COM13",
	"\\\\.\\COM14",
	"\\\\.\\COM15",
	"\\\\.\\COM16",
	"\\\\.\\COM17",
	"\\\\.\\COM18",
	"\\\\.\\COM19",
	"\\\\.\\COM20",
	"\\\\.\\COM21",
	"\\\\.\\COM22",
	"\\\\.\\COM23",
	"\\\\.\\COM24",
	"\\\\.\\COM25",
	"\\\\.\\COM26",
	"\\\\.\\COM27",
	"\\\\.\\COM28",
	"\\\\.\\COM29",
	"\\\\.\\COM30",
	"\\\\.\\COM31",
	"\\\\.\\COM32"
};

COMM_PARA_ST *apCommParaST[32];
int g_iCommIndex = -1;

BOOL g_bNeedMod = FALSE;
int g_iHadNum = 0;
int g_iNeedNum = 5;
float g_fValue = 0.05f;
BOOL g_bHadData[10]; //是否已经有数据
double g_fPerSpec[10][2048];
BOOL g_bUseFFT = FALSE;

//是否读取过波长数据
BOOL g_bHaveRead = FALSE;
double g_pdWavelengthData[2048];
int g_pTempSpectumNumber = 0;


double g_dPixWL_New[2048];
double g_dPixWL_Per[1024];

struct compx { float real, imag; };   //复数结构

#define PI 3.1415926535897932384626433832795028841971 //圆周率值  
int FFT_N = 0;//傅里叶变化点 

float *SIN_TAB;//正弦表的存放空间
float *COS_TAB;//正弦表的存放空间
int F_N = 0;
int FFTData_Len = 2048;

BOOL g_bTrig = FALSE;
int g_iAve = 1;

//滑动窗口
int g_window = 0;

//////////////////////////////////////////////////////////////////////////////////////////
////////滤波
//////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************
函数原型：struct compx EE(struct compx b1,struct compx b2)
函数功能：对两个复数进行乘法运算
输入参数：两个以联合体定义的复数a,b
输出参数：a和b的乘积，以联合体的形式输出
*******************************************************************/
void EE(struct compx a, struct compx b, compx* out)
{
	struct compx c;
	c.real = a.real*b.real - a.imag*b.imag;
	c.imag = a.real*b.imag + a.imag*b.real;
	
	out->real = c.real;
	out->imag = c.imag;
}

/******************************************************************
函数原型：void create_sin_table(int PointNum)
函数功能：创建正弦静态表 
输入参数：傅里叶变换点个数 
输出参数：
******************************************************************/
void create_sin_table(int PointNum){
	//如果静态表不为空，释放之前的静态表 
	if(SIN_TAB != 0){
		free((void *)SIN_TAB);
		free((void *)COS_TAB);
	}
	
	//申请静态表空间 
	SIN_TAB = (float *)malloc(PointNum * sizeof(float));
	COS_TAB = (float *)malloc(PointNum * sizeof(float));
	
	//初始化FFT_N长度 
	FFT_N = PointNum;
	
	//初始化静态表 
	for (int i = 0; i <= FFT_N / 2; i++){
		SIN_TAB[i] = (float)sin(2 * PI * i / FFT_N);
		COS_TAB[i] = (float)cos(2 * PI * i / FFT_N);
	}
}

/******************************************************************
函数原型：void sin_tab(float pi)
函数功能：采用查表的方法计算一个数的正弦值
输入参数：pi 所要计算正弦值弧度值，范围0--2*PI，不满足时需要转换
输出参数：输入值pi的正弦值
******************************************************************/
float sin_tab(int pi)
{
	if(pi < FFT_N / 2)
	{
		return SIN_TAB[pi];
	}
	else{
		return -SIN_TAB[pi - FFT_N / 2];
	}
}

/******************************************************************
函数原型：void cos_tab(float pi)
函数功能：采用查表的方法计算一个数的余弦值
输入参数：pi 所要计算余弦值弧度值，范围0--2*PI，不满足时需要转换
输出参数：输入值pi的余弦值
******************************************************************/
float cos_tab(int pi)
{
//	float a, pi2;
//	pi2 = pi + PI / 2;
//	if (pi2 > 2 * PI)
//		pi2 -= 2 * PI;
//	a = sin_tab(pi2);
	if(pi < FFT_N / 2){
		return COS_TAB[pi];
	}else{
		return COS_TAB[FFT_N * 3 / 2 - pi];
	}
}

/*****************************************************************
函数原型：void FFT(struct compx *xin)
函数功能：对输入的复数组进行快速傅里叶变换（FFT）
输入参数：*xin复数结构体组的首地址指针，struct型
输出参数：无
*****************************************************************/
void Wn_i(int n,int i,compx *Wn)  
{  
//  Wn->real = cos(2*PI*i/n);  
//  Wn->imag = -sin(2*PI*i/n); 
  Wn->real = cos_tab(i); 
  Wn->imag = -sin_tab(i);  
}  

//void c_mul(compx a,compx b,compx *c)  
//{  
//  c->real = a.real * b.real - a.imag * b.imag;  
//  c->imag = a.real * b.imag + a.imag * b.real;     
//}  

void c_sub(compx a,compx b,compx *c)  
{  
  c->real = a.real - b.real;  
  c->imag = a.imag - b.imag;   
}

void c_plus(compx a,compx b,compx *c)  
{  
  c->real = a.real + b.real;  
  c->imag = a.imag + b.imag;  
}

//傅里叶变化  
void FFT(compx f[])  
{  
	int N = FFT_N;
	compx t,wn;//中间变量  
	int i,j,k,m,n,l,r,M;  
	int la,lb,lc;  
	/*----计算分解的级数M=log2(N)----*/  
	for(i=N,M=1;(i=i/2)!=1;M++);   
	/*----按照倒位序重新排列原信号----*/  
	for(i=1,j=N/2;i<=N-2;i++)  
	{  
		if(i<j)  
		{	  
			t=f[j];  
			f[j]=f[i];  
			f[i]=t;  
	    }  
	    k=N/2;  
	    while(k<=j)  
	    {  
			j=j-k;  
			k=k/2;  
	    }  
	    j=j+k;  
	}  
  
  /*----FFT算法----*/  
	for(m=1;m<=M;m++)  
	{  
		la=(int)pow((double)2,m); //la=2^m代表第m级每个分组所含节点数       
		lb=la/2;    //lb代表第m级每个分组所含碟形单元数  
                 //同时它也表示每个碟形单元上下节点之间的距离  
 		/*----碟形运算----*/  
 		for(l=1;l<=lb;l++)  
 		{  
			r=(int)((l-1)*pow((double)2,M-m));
			for(n=l-1;n<N-1;n=n+la) //遍历每个分组，分组总数为N/la  
			{  
 				lc=n+lb;  //n,lc分别代表一个碟形单元的上、下节点编号       
  				Wn_i(N,r,&wn);//wn=Wnr  
  				EE(f[lc],wn,&t);//t = f[lc] * wn复数运算  
				c_sub(f[n],t,&(f[lc]));//f[lc] = f[n] - f[lc] * Wnr  
				c_plus(f[n],t,&(f[n]));//f[n] = f[n] + f[lc] * Wnr  
 			}  
		}  
 	}  
}  

/*****************************************************************
函数原型：void IFFT(struct compx *xin)
函数功能：对输入的复数组进行快速傅里叶逆变换（FFT）
输入参数：*xin复数结构体组的首地址指针，struct型
输出参数：无
*****************************************************************/
void IFFT(struct compx *xin)
{
	int i;
	float tmp;
	//反转复数 
	for(i = 0; i < FFT_N; i++)
	{
		tmp = xin[i].imag;
		xin[i].imag = xin[i].real;
		xin[i].real = tmp;
	}
	
	//再做快速傅里叶变换 
	FFT(xin);
	
	//再次反转复数，并缩小 
	for(i = 0; i < FFT_N; i++)
	{
		tmp = xin[i].imag;
		xin[i].imag = xin[i].real / FFT_N;
		xin[i].real = tmp / FFT_N;
	}
}

//////////////////////////////////////////////////////////////////
////FFTFilter
/******************************************************************
函数原型：void initFFTFilter(int PointNum, int filternum)
函数功能：初始化滤波器 
输入参数：int PointNum 采样个数，, int filternum 过滤点 (0 ~ PointNum / 2) 一般取长度的 1/20 
	      filternum =  PointNum / 2 全通 
	      filternum 越大，通过的频率越高 
输出参数：1，输入点数不正确 
******************************************************************/
int FFT_initFilter(int PointNum, int filternum)
{
	create_sin_table(PointNum);
	F_N = filternum;
	FFTData_Len = PointNum;
	
	return 0;
}

/*****************************************************************
函数原型：void FFTFilter(float *input, int inputlen)
函数功能：对输入的数组进行滤波算法
输入参数：float *input 原始数据，过滤后数据直接填写在原始数据上，节省空间, 
          int inputlen 数据长度
输出参数：返回1 数据长度和初始化的不一致，需要重新初始化滤波器 
*****************************************************************/
    //创建buffer 
	compx* cinput = (compx *)malloc(FFTData_Len * sizeof(compx));
int FFT_Filter(double *input, int inputlen)
{
	int i;
	if(inputlen != FFTData_Len)
	{
		return 1;
	}
	
	
	//将原始数据转换到复平面 
	for(i = 0; i < FFTData_Len; i++)
	{
		cinput[i].real = (float)(input[i]);
		cinput[i].imag = 0;
	}
	//快速傅里叶变换 
	FFT(cinput);	
	
	//滤波 ================================================ 
	for(i = 0; i < FFTData_Len; i++)
	{
		if(i > F_N && i < FFTData_Len - F_N)
		{
			cinput[i].real = cinput[i].real*0.1f;
			cinput[i].imag = cinput[i].imag*0.1f;
		}
	}
	//滤波 ================================================ 
	
	//傅里叶逆变换 
	IFFT(cinput);
	
	for(i = 0; i < FFTData_Len; i++)
	{
		input[i] = cinput[i].real;
	}

	free(cinput);
	return 0;
}

float NewInt_float(float *xa, float *ya, int n, float x)
{
	//牛顿插值

	int i, k;
	float u;
	k=1;
	for(i=1; i<=n-2; i++)
	{
		if( x <= xa[i] )
		{
			k=i;
			break;
		}
		else
			k=n-1;
	}
	u=(x-xa[k-1])/(xa[k]-xa[k-1]);
	return ya[k-1]+u*(ya[k]-ya[k-1]);
}


inline float SA_NewtonDifference(float *xa, float *ya, int n, float x)
{
	int i = 0;
	int k = 1;
	float fTemp = 0;

	for(i = 1; i <= n - 2; i ++)
	{
		if( x <= xa[i] )
		{
			k=i;
			break;
		}
		else
			k=n-1;
	}
	
	fTemp=(x - xa[k - 1]) / (xa[k] - xa[k - 1]);
	
	return ya[k - 1] + fTemp * (ya[k] - ya[k - 1]);
}

char* SA_GetAPIVersion(void)
{
	return (char *)acAPIVersion;
}

/****************************************************************************************/
/****************************************************************************************/
/***************************** 光谱仪初始化操作接口**********************************/
/****************************************************************************************/
/****************************************************************************************/
BOOL SA_InitSpectrometers(int CommIndex)
{
	BYTE *pbTemp;
	int i = 0;
	int j = 0;
	int iTemp;
	UN_TYPE unTemp;
	UN_32TYPE un32Temp;
	int iLengthTemp = 0;

	for(i = 0; i < 3; i++)
	{
		//读取EIA数据
		Sleep(10);
		if(MIGP_RegArea_Read(apCommParaST[CommIndex], EIA, 0x0000, 0x40, &pbTemp, 500) == TRUE)
		{
			//串口需要清理收发区域并且等待100ms
			if(apCommParaST[CommIndex]->enCommType == COM_SERIAL)
			{
				Sleep(100);
				PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
			}
			for(i = 0; i < 16; i ++)
			{
				apCommParaST[CommIndex]->stEIA.aucEquipmentInfo[i] = pbTemp[i + 4]; 
			}
			//设备名称
			apCommParaST[CommIndex]->stEIA.aucEquipmentInfo[16] = '\0';
			//硬件版本
			apCommParaST[CommIndex]->stEIA.ucHardWareVersion = pbTemp[0x10 + 4];
			//软件版本
			apCommParaST[CommIndex]->stEIA.aucSoftWareVersion[0] = pbTemp[0x18 + 4];
			apCommParaST[CommIndex]->stEIA.aucSoftWareVersion[1] = pbTemp[0x19 + 4];
			apCommParaST[CommIndex]->stEIA.aucSoftWareVersion[2] = pbTemp[0x1A + 4];
			apCommParaST[CommIndex]->stEIA.aucSoftWareVersion[3] = pbTemp[0x1B + 4];
			apCommParaST[CommIndex]->stEIA.aucSoftWareVersion[4] = '\0';

			//获取序列号
			for(i = 0; i < 16; i ++)
			{
				apCommParaST[CommIndex]->stEIA.aucSerialNumber[i] = pbTemp[EIA_SERIAL_NUMBER_ADDR + i + 4]; 
			}
			apCommParaST[CommIndex]->stEIA.aucSerialNumber[16] = '\0';

			//读取生产日期
			for(i = 0; i < 16; i ++)
			{
				apCommParaST[CommIndex]->stEIA.aucManufacturingDate[i] = pbTemp[EIA_MANUFACTURING_DATE_ADDR + i + 4]; 
			}
			apCommParaST[CommIndex]->stEIA.aucManufacturingDate[16] = '\0';
			break;
		}
	}

	if(i == 3)
	{
		return FALSE;
	}

	//初始化基本信息
	if(MIGP_RegArea_Read(apCommParaST[CommIndex], NVPA, 0x0000, 112, &pbTemp, 1000) == TRUE)
	{
		//串口需要清理收发区域并且等待100ms
		if(apCommParaST[CommIndex]->enCommType == COM_SERIAL)
		{
			Sleep(100);
			PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
		}
		//读取起始波长
		iTemp = (int)pbTemp[4 + NVPA_MIN_WAVELENGTH_ADDR] << 8;
		iTemp |= (int)pbTemp[4 + NVPA_MIN_WAVELENGTH_ADDR + 1];
		apCommParaST[CommIndex]->stSpectrometerPara.iStartWavelength = iTemp;
		
		//读取截至波长
		iTemp = (int)pbTemp[4 + NVPA_MAX_WAVELENGTH_ADDR] << 8;
		iTemp |= (int)pbTemp[4 + NVPA_MAX_WAVELENGTH_ADDR + 1];
		apCommParaST[CommIndex]->stSpectrometerPara.iEndWavelength = iTemp;
		
		//读取最小积分时间
		iTemp = (int)pbTemp[4 + NVPA_MIN_INTEGRAL_TIME_ADDR] << 24;
		iTemp |= (int)pbTemp[5 + NVPA_MIN_INTEGRAL_TIME_ADDR] << 16;
		iTemp |= (int)pbTemp[6 + NVPA_MIN_INTEGRAL_TIME_ADDR] << 8;
		iTemp |= (int)pbTemp[7 + NVPA_MIN_INTEGRAL_TIME_ADDR];
		apCommParaST[CommIndex]->stSpectrometerPara.iMinIntegrationTimeUS = iTemp;
		
		//读取最大积分时间
		iTemp = (int)pbTemp[4 + NVPA_MAX_INTEGRAL_TIME_ADDR] << 24;
		iTemp |= (int)pbTemp[5 + NVPA_MAX_INTEGRAL_TIME_ADDR] << 16;
		iTemp |= (int)pbTemp[6 + NVPA_MAX_INTEGRAL_TIME_ADDR] << 8;
		iTemp |= (int)pbTemp[7 + NVPA_MAX_INTEGRAL_TIME_ADDR];
		apCommParaST[CommIndex]->stSpectrometerPara.iMaxIntegrationTimeUS = iTemp;
		
		//读取像素点个数
		iTemp = (int)pbTemp[4 + NVPA_PIXELS_NUMBER_ADDR] << 8;
		iTemp |= (int)pbTemp[4 + NVPA_PIXELS_NUMBER_ADDR + 1];
		apCommParaST[CommIndex]->stSpectrometerPara.iPixelNumber = iTemp;
		//读取电路板信息 新板子为1
		iTemp = (int)pbTemp[4 + NVPA_PIXELS_TYPE_ADDR];
		apCommParaST[CommIndex]->stSpectrometerPara.iPixelType = iTemp;
		
		//读取电路板信息 像素点先后 后为1
		iTemp = (int)pbTemp[4 + NVPA_PIXELS_POS_ADDR];
		apCommParaST[CommIndex]->stSpectrometerPara.iPosType = iTemp;

//		char szBuf[4]; //int 是4个字节，而char是1个字节，所以数组大小是4
//		LPCTSTR Text1;
//		sprintf(szBuf,"%d",iTemp);// 歌曲序号从1开始,所以为i+1
//		Text1=LPCTSTR(szBuf);
//		MessageBox(NULL,Text1,TEXT("YES"),MB_OK);


		//读取波长系数
		for(i = 0; i < 4; i++)
		{
			unTemp.ab8Data[7] = (int)pbTemp[4 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			unTemp.ab8Data[6] = (int)pbTemp[5 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			unTemp.ab8Data[5] = (int)pbTemp[6 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			unTemp.ab8Data[4] = (int)pbTemp[7 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			unTemp.ab8Data[3] = (int)pbTemp[8 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			unTemp.ab8Data[2] = (int)pbTemp[9 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			unTemp.ab8Data[1] = (int)pbTemp[10 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			unTemp.ab8Data[0] = (int)pbTemp[11 + (i * 8) + NVPA_WAVELENGTH_CALIB_ADDR];
			apCommParaST[CommIndex]->stSpectrometerPara.adWavelengthCalib[i] = unTemp.d64Data;
		}
	}
	else
	{
		return FALSE;
	}

	if(MIGP_RegArea_Read(apCommParaST[CommIndex], VPA, VPA_RUN_MODE_ADDR, 10, &pbTemp, 1000) == TRUE)
	{
		//串口需要清理收发区域并且等待100ms
		if(apCommParaST[CommIndex]->enCommType == COM_SERIAL)
		{
			Sleep(100);
			PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
		}
		//初始化触发模式
		apCommParaST[CommIndex]->stSpectrometerPara.cTriggerMode = pbTemp[4 + VPA_RUN_MODE_ADDR];
		iTemp = (int)pbTemp[4 + VPA_INTE_TIME_ADDR] << 24;
		iTemp |= (int)pbTemp[5 + VPA_INTE_TIME_ADDR] << 16;
		iTemp |= (int)pbTemp[6 + VPA_INTE_TIME_ADDR] << 8;
		iTemp |= (int)pbTemp[7 + VPA_INTE_TIME_ADDR];
		//积分时间
		apCommParaST[CommIndex]->stSpectrometerPara.iIntegrationTime = iTemp;
		iTemp = (int)pbTemp[4] << 8;
		iTemp |= (int)pbTemp[5];
		//平均次数
		apCommParaST[CommIndex]->stSpectrometerPara.iAverageTimes = iTemp;
		iTemp = (int)pbTemp[6] << 8;
		iTemp |= (int)pbTemp[7];
		//触发延时
		apCommParaST[CommIndex]->stSpectrometerPara.iTriggerDelay = iTemp;
	}
	else
	{
		return FALSE;
	}

	//读取多同道积分时间
	if(MIGP_RegArea_Read(apCommParaST[CommIndex], VPA, 112, 64, &pbTemp, 1000) == TRUE)
	{
		//串口需要清理收发区域并且等待100ms
		if(apCommParaST[CommIndex]->enCommType == COM_SERIAL)
		{
			Sleep(100);
			PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
		}

		for(i = 0; i < 16; i++)
		{
			iTemp = (int)pbTemp[4 + (i * 4)] << 24;
			iTemp |= (int)pbTemp[5 + (i * 4)] << 16;
			iTemp |= (int)pbTemp[6 + (i * 4)] << 8;
			iTemp |= (int)pbTemp[7 + (i * 4)];
			apCommParaST[CommIndex]->stSpectrometerPara.iChannelIntegrationTime[i] = iTemp;
		}
		
	}
	else
	{
		return FALSE;
	}

	//读取非线性类型
	if(MIGP_RegArea_Read(apCommParaST[CommIndex], NVPA, 0x0040, 1, &pbTemp, 500) == TRUE)
	{
		//串口需要清理收发区域并且等待100ms
		if(apCommParaST[CommIndex]->enCommType == COM_SERIAL)
		{
			Sleep(100);
			PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
		}
		apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = pbTemp[4];

		if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_DISABLE &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_WAVELENGTH &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_PIXEL &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_WAVELENGTH_II &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_PIXEL_II)
		{
			apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
		}

		if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_DISABLE)
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}

	//读取非线性系数
	if(MIGP_RegArea_Read(apCommParaST[CommIndex], NVPA, 0x0041, 760, &pbTemp, 2000) == TRUE)
	{
		//串口需要清理收发区域并且等待100ms
		if(apCommParaST[CommIndex]->enCommType == COM_SERIAL)
		{
			Sleep(100);
			PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
		}
		//非线性定标长度
		un32Temp.ab8Data[3] = (int)pbTemp[4];
		un32Temp.ab8Data[2] = (int)pbTemp[5];
		un32Temp.ab8Data[1] = (int)pbTemp[6];
		un32Temp.ab8Data[0] = (int)pbTemp[7];
		apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength = (int)un32Temp.f32Data;

		//非线性点数只能是1-8
		if(apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength < 1 ||
		   apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength > 8)
		{
			apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
			return TRUE;
		}

		//波长和像素定标I型
		if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_WAVELENGTH ||
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_PIXEL)
		{
			for(i = 0; i < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength; i++)
			{
				//读取当前点个波长值或者像素值
				un32Temp.ab8Data[3] = (int)pbTemp[8 + (i * 88)];
				un32Temp.ab8Data[2] = (int)pbTemp[9 + (i * 88)];
				un32Temp.ab8Data[1] = (int)pbTemp[10 + (i * 88)];
				un32Temp.ab8Data[0] = (int)pbTemp[11 + (i * 88)];
				apCommParaST[CommIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[i] = un32Temp.f32Data;
				
				//读取非线性当前点还有的非线性校准点个数
				un32Temp.ab8Data[3] = (int)pbTemp[12 + (i * 88)];
				un32Temp.ab8Data[2] = (int)pbTemp[13 + (i * 88)];
				un32Temp.ab8Data[1] = (int)pbTemp[14 + (i * 88)];
				un32Temp.ab8Data[0] = (int)pbTemp[15 + (i * 88)];
				apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] = (int)un32Temp.f32Data;
				if(apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] < 1 ||
			   	   apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] > 10)
				{
					apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
					return TRUE;
				}
				
				//读取当前点在不同的AD值下的校准系数
				for(j = 0; j < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i]; j++)
				{
					//读取AD值
					un32Temp.ab8Data[3] = (int)pbTemp[16 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[2] = (int)pbTemp[17 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[1] = (int)pbTemp[18 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[0] = (int)pbTemp[19 + (i * 88) + (j * 8)];
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibAD[i][j] = un32Temp.f32Data;

					//读取点校准系数
					un32Temp.ab8Data[3] = (int)pbTemp[20 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[2] = (int)pbTemp[21 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[1] = (int)pbTemp[22 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[0] = (int)pbTemp[23 + (i * 88) + (j * 8)];
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibCo[i][j] = un32Temp.f32Data;
				}
				
			}
		}//波长和像素定标II型
		else if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_WAVELENGTH_II ||
		        apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_PIXEL_II)
		{
			iLengthTemp = 8;
			for(i = 0; i < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength; i++)
			{
				un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
				un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
				un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
				un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
				iLengthTemp = iLengthTemp + 4;
				apCommParaST[CommIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[i] = un32Temp.f32Data;
				//printf("NonlinearCalibPixelOrWL %f\r\n", un32Temp.f32Data);
				un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
				un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
				un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
				un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
				iLengthTemp = iLengthTemp + 4;
				apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] = (int)un32Temp.f32Data;
				
				//printf("fNonlinearCalibCoNumber %d\r\n", (int)apCommParaST[g_iCommIndex + 1]->stSpectrometerPara.iNonlinearCalibCoNumber[i]);
				for(j = 0; j < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i]; j++)
				{
					un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
					un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
					un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
					un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
					iLengthTemp = iLengthTemp + 4;
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibAD[i][j] = un32Temp.f32Data;
					//printf("AD %f ", apCommParaST[g_iCommIndex + 1]->stSpectrometerPara.afNonlinearCalibAD[i][j]);
					un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
					un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
					un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
					un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
					iLengthTemp = iLengthTemp + 4;
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibCo[i][j] = un32Temp.f32Data;
					//printf("CalibCo %f\r\n", apCommParaST[g_iCommIndex + 1]->stSpectrometerPara.afNonlinearCalibCo[i][j]);
					if(iLengthTemp > 760)
					{
						apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
						return TRUE;
					}
				}
				
			}
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

int SA_OpenSpectrometers(void)
{
	//BYTE *pbTemp;
	//UN_TYPE unTemp;
	//UN_32TYPE un32Temp;
	COMM_PARA_ST *pstMigpPara = NULL;
	//HANDLE CommDev;
	int DevNum;
	int i = 0;
	int ret = 0;
	//int iTemp;
	
	g_iCommIndex = -1;

	g_bHaveRead = FALSE;
 
	DevNum = USBScanDev(1);

	if(DevNum == 0)
	{
		return g_iCommIndex;
	}

	for(i = 0; i < DevNum; i++)
	{
		ret = USBOpenDev(i);
		if(USBOpenDev(i) != SEVERITY_ERROR)
		{
			apCommParaST[g_iCommIndex + 1] = (COMM_PARA_ST *)malloc(sizeof(COMM_PARA_ST));
			if(apCommParaST[g_iCommIndex + 1] == NULL)
			{
				USBCloseDev(i);
				free(apCommParaST[g_iCommIndex + 1]);
			}
			else
			{
				apCommParaST[g_iCommIndex + 1]->enCommType = USB_WINLIB;
				apCommParaST[g_iCommIndex + 1]->stComPara.iUSBDevNum = i;
				apCommParaST[g_iCommIndex + 1]->stMIGP.bMasterAddress = 0xFE;
				apCommParaST[g_iCommIndex + 1]->stMIGP.bSlaveAddress = 0x00;

				if(SA_InitSpectrometers(g_iCommIndex + 1) == TRUE)
				{
					g_iCommIndex++;
				}
				else
				{					
					USBCloseDev(i);
					free(apCommParaST[g_iCommIndex + 1]);
				}
			}
		}		
	}

/*	int iGetNum = 0;
	double dGetAD[2048];
	Sleep(1);
	SA_GetSpectumN(0, dGetAD, iGetNum);
	Sleep(1);
	SA_GetSpectumN(0, dGetAD, iGetNum);
	Sleep(1);
	SA_GetSpectumN(0, dGetAD, iGetNum);
*/
	return g_iCommIndex;
}

int SA_OpenSpectrometersForSerial(void)
{
	COMM_PARA_ST *pstMigpPara = NULL;
	HANDLE CommDev;
	int i = 0;
	
	g_iCommIndex = -1;

	for(i = 0; i < 32; i++)
	{
		if(Com_Open(CommDev, (char *)acComName[i], 115200) == TRUE)
		{
			apCommParaST[g_iCommIndex + 1] = (COMM_PARA_ST *)malloc(sizeof(COMM_PARA_ST));
			if(apCommParaST[g_iCommIndex + 1] == NULL)
			{
				Com_Close(CommDev);
				free(apCommParaST[g_iCommIndex + 1]);
			}
			else
			{
				apCommParaST[g_iCommIndex + 1]->enCommType = COM_SERIAL;
				apCommParaST[g_iCommIndex + 1]->stComPara.ComHandle = CommDev;
				apCommParaST[g_iCommIndex + 1]->stComPara.iBaudRate = 115200;
				strcpy_s(apCommParaST[g_iCommIndex + 1]->stComPara.acComName, acComName[i]);
				apCommParaST[g_iCommIndex + 1]->stMIGP.bMasterAddress = 0xFE;
				apCommParaST[g_iCommIndex + 1]->stMIGP.bSlaveAddress = 0x00;

				if(SA_InitSpectrometers(g_iCommIndex + 1) == TRUE)
				{
					g_iCommIndex++;
				}
				else
				{
					Com_Close(CommDev);
					free(apCommParaST[g_iCommIndex + 1]);
				}
			}
		}
	}

	return g_iCommIndex;
}


int SA_OpenSpectrometersSimplifyForSerial(int iComID)
{
	COMM_PARA_ST *pstMigpPara = NULL;
	HANDLE CommDev;
	int iComTemp = 0;
	
	g_iCommIndex = -1;

	if(iComID < 1 || iComID > 31)
	{
		return g_iCommIndex;
	}

	iComTemp = iComID - 1;
	if(Com_Open(CommDev, (char *)acComName[iComTemp], 115200) == TRUE)
	{
		apCommParaST[0] = (COMM_PARA_ST *)malloc(sizeof(COMM_PARA_ST));
		if(apCommParaST[0] == NULL)
		{
//				printf_debug("SA_OpenSpectrometers:: malloc NULL");
			Com_Close(CommDev);
			free(apCommParaST[0]);
		}
		else
		{
			apCommParaST[0]->enCommType = COM_SERIAL;
			apCommParaST[0]->stComPara.ComHandle = CommDev;
			apCommParaST[0]->stComPara.iBaudRate = 115200;
			strcpy_s(apCommParaST[0]->stComPara.acComName, acComName[iComTemp]);
			apCommParaST[0]->stMIGP.bMasterAddress = 0xFE;
			apCommParaST[0]->stMIGP.bSlaveAddress = 0x00;

			if(SA_InitSpectrometers(0) == TRUE)
			{
				g_iCommIndex = 0;
			}
			else
			{
//					printf_debug("SA_OpenSpectrometers:: MIGP_RegArea_Read FAIL");
				Com_Close(CommDev);
				free(apCommParaST[0]);
			}
		}
	}

	return g_iCommIndex;
}


void SA_CloseSpectrometers(void)
{
	int i = 0;
	
	if(g_iCommIndex != -1)
	{
		for(i = 0; i < g_iCommIndex + 1; i++)
		{
			if(apCommParaST[i]->enCommType == COM_SERIAL)
			{
				Com_Close(apCommParaST[g_iCommIndex]->stComPara.ComHandle);
				free(apCommParaST[g_iCommIndex]);
			}
			else if(apCommParaST[i]->enCommType == USB_WINLIB)
			{
				USBCloseDev(apCommParaST[g_iCommIndex]->stComPara.iUSBDevNum);
				free(apCommParaST[g_iCommIndex]);
			}
			
		}
	}
}

/****************************************************************************************/
/****************************************************************************************/
/***************************** 光谱仪参数设置接口****************************************/
/****************************************************************************************/
/****************************************************************************************/
void WindowSmooth(double *input, int inputlen)
{
	int i = 0;
	int j = 0;
	int w_index = 0;
    int p = g_window;      // 半窗口(M)
	double H[21];
	double output[2048] = {0};

	//设置窗口权重
	for(i = 0; i < 21; i++)
	{
		H[i] = 1.0 / (p*2+1);
	}

	//卷积
	for(i = p + 1; i < inputlen - p - 1; i++)
    {
		//清0
        output[i] = 0;
		//窗口内所有数据拟合
        for(j = i - p, w_index = 0; j <= i + p; j++, w_index ++){
			//只用了H[0]这一行，因为只计算a0
            output[i] += H[w_index] * input[j];
        }
    }

	//头尾不做处理，直接复制，最后差分数据会变成0
    for(i = 0; i < p + 1; i++)
    {
        output[i] =  input[i];
        output[inputlen - p - 1 + i] = input[inputlen - p - 1 + i];
    }

	memcpy(input, output, sizeof(double) * inputlen);
}

int SA_SetMultiChannelIntegrationTime (int spectrometerIndex, int *usec)
{
	BYTE bTemp[64];
	unsigned long ulTemp = 0;
	int i = 0;
		
	if(spectrometerIndex > g_iCommIndex)
	{
		return SA_API_FAIL;
	}

	for(i = 0; i < CHL_NUM; i++)
	{
		if(usec[i] < apCommParaST[spectrometerIndex]->stSpectrometerPara.iMinIntegrationTimeUS ||
		   usec[i] > apCommParaST[spectrometerIndex]->stSpectrometerPara.iMaxIntegrationTimeUS)
		{
			usec[i] = apCommParaST[spectrometerIndex]->stSpectrometerPara.iMinIntegrationTimeUS;
		}
		ulTemp = (unsigned long)(usec[i]);

		bTemp[0 + (i * 4)] = (BYTE)(ulTemp >> 24 & 0x000000ff);
		bTemp[1 + (i * 4)] = (BYTE)(ulTemp >> 16 & 0x000000ff);
		bTemp[2 + (i * 4)] = (BYTE)(ulTemp >> 8 & 0x000000ff);
		bTemp[3 + (i * 4)] = (BYTE)(ulTemp & 0x000000ff);

		apCommParaST[spectrometerIndex]->stSpectrometerPara.iChannelIntegrationTime[i] = usec[i];
	}


	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 112, 32, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 144, 32, &bTemp[32], MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}
	
	return SA_API_SUCCESS;
}


int SA_SetRGBModeIntegrationTime (int spectrometerIndex, int *usec, int usec_num)
{
	BYTE bTemp[64];
	unsigned long ulTemp = 0;
	int i = 0;
	
	
	if(spectrometerIndex > g_iCommIndex)
	{
		return SA_API_FAIL;
	}

	for(i = 0; i < 3; i++)
	{
		if(usec[i] < apCommParaST[spectrometerIndex]->stSpectrometerPara.iMinIntegrationTimeUS ||
		   usec[i] > apCommParaST[spectrometerIndex]->stSpectrometerPara.iMaxIntegrationTimeUS)
		{
			usec[i] = apCommParaST[spectrometerIndex]->stSpectrometerPara.iMinIntegrationTimeUS;
		}
	}

	for(i = 0; i < 3; i++)
	{
		if(i < usec_num)
			ulTemp = (unsigned long)(usec[i]);
		else
			ulTemp = 0;
		bTemp[0 + (i * 4)] = (BYTE)(ulTemp >> 24 & 0x000000ff);
		bTemp[1 + (i * 4)] = (BYTE)(ulTemp >> 16 & 0x000000ff);
		bTemp[2 + (i * 4)] = (BYTE)(ulTemp >> 8 & 0x000000ff);
		bTemp[3 + (i * 4)] = (BYTE)(ulTemp & 0x000000ff);

		apCommParaST[spectrometerIndex]->stSpectrometerPara.iChannelIntegrationTime[i] = usec[i];
	}


	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 112, 12, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}
	
	return SA_API_SUCCESS;
}

int SA_SetPosType(int spectrometerIndex, int iPos)
{
		
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	BYTE bTemp[1];
	bTemp[0] = iPos;

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, NVPA_PIXELS_POS_ADDR, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}
	
	apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType = iPos;

	return SA_API_SUCCESS;
}


int SA_SetIntegrationTime (int spectrometerIndex, int usec)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}
	if(usec == apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime)
	{
		return SA_API_SUCCESS;
	}

	if(usec < apCommParaST[spectrometerIndex]->stSpectrometerPara.iMinIntegrationTimeUS ||
	   usec > apCommParaST[spectrometerIndex]->stSpectrometerPara.iMaxIntegrationTimeUS)
	{
		return SA_API_FAIL;
	}

	BYTE bTemp[4];
	unsigned long ulTemp = 0;

	ulTemp = (unsigned long)(usec);
	bTemp[0] = (BYTE)(ulTemp >> 24 & 0x000000ff);
	bTemp[1] = (BYTE)(ulTemp >> 16 & 0x000000ff);
	bTemp[2] = (BYTE)(ulTemp >> 8 & 0x000000ff);
	bTemp[3] = (BYTE)(ulTemp & 0x000000ff);

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0001, 4, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}
	
	apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime = usec;
	return SA_API_SUCCESS;
}


int SA_SetAverageTimes(int spectrometerIndex, int AverageTimes)
{
	if(AverageTimes == apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes)
	{
		return SA_API_SUCCESS;
	}
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(AverageTimes < 1 || AverageTimes > 1000)
	{	
		return SA_API_FAIL;
	}

	BYTE bTemp[2];
	bTemp[0] = AverageTimes >> 8 & 0x000000ff;
	bTemp[1] = AverageTimes & 0x000000ff;


	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, VPA_AVERAGE_TIME_ADDR, 2, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}
	
	apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes = AverageTimes;
	g_iAve = AverageTimes;

	return SA_API_SUCCESS;
}

 
 int SA_SetSpectumTriggerMode(int spectrometerIndex, TRIGGER_MODE TriggerMode)
 {
	 BYTE bTemp[1];
	 int i = 0;
 		 
	 if(spectrometerIndex > g_iCommIndex)
	 {	 
		 return SA_API_FAIL;
	 }
	 
	 switch(TriggerMode)
	 {
	 	 case SOFTWARE_SYNCHRONOUS:
		 	 bTemp[0] = 0x00;
			 g_bTrig = FALSE;
			 break;
		case SOFTWARE_ASYNCHRONOUS:
			g_bTrig = TRUE;
		 	 bTemp[0] = 0x02;
			 break;
		case CACHE_ASYNCHRONOUS:
		 	 bTemp[0] = 0x03;
			 break;
		 case EXINT_RISING_EDGE:
			 bTemp[0] = 0x04;
			 break;
		 case EXINT_FALLING_EDGE:
			 bTemp[0] = 0x05;
			 break;
		 case EXINT_HIGH_LEVEL:
			 bTemp[0] = 0x08;
			 break;
		 case EXINT_LOW_LEVEL:
			 bTemp[0] = 0x09;
			 break;
		 case EXINT_RGB_RISING_EDGE:
			 bTemp[0] = 0x0A;
			 break;
		 case EXINT_RGB_FALLING_EDGE:
			 bTemp[0] = 0x0B;
			 break;
		 default:
			 return SA_API_FAIL;
	 }

	 apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = TriggerMode;
 
	 if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0000, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	 {
		 if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0000, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		 {
			 if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0000, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
			 {
				 return SA_API_FAIL;
			 }
		 }
	 }

	 return SA_API_SUCCESS;

 }

 /* 上升沿触发启动操作接口 */
int SA_StartRETrigger(int spectrometerIndex)
{
	return SA_SetSpectumTriggerMode(spectrometerIndex, EXINT_RISING_EDGE);
}

/*  下降沿触发启动操作接口 */
int SA_StartFETrigger(int spectrometerIndex)
{
	return SA_SetSpectumTriggerMode(spectrometerIndex, EXINT_FALLING_EDGE);
}

/* RGB上升沿触发启动操作接口 */
int SA_StartRBGModeRETrigger(int spectrometerIndex)
{
	return SA_SetSpectumTriggerMode(spectrometerIndex, EXINT_RGB_RISING_EDGE);
}


/*  RGB下降沿触发启动操作接口 */
int SA_StartRBGModeFETrigger(int spectrometerIndex)
{
	return SA_SetSpectumTriggerMode(spectrometerIndex, EXINT_RGB_FALLING_EDGE);
}


 /****************************************************************************************/
/****************************************************************************************/
/***************************** 光谱仪波长操作接口****************************************/
/****************************************************************************************/
/****************************************************************************************/
 
int SA_GetWavelengthCalibrationCoefficients (int spectrometerIndex, double * WavelengthCalibration)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetWavelengthCalibrationCoefficients FAIL");
		return SA_API_FAIL;
	}

	WavelengthCalibration[0] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[0];
	WavelengthCalibration[1] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[1];
	WavelengthCalibration[2] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[2];
	WavelengthCalibration[3] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[3];

	return SA_API_SUCCESS;
}

int SA_SetWavelengthCalibrationCoefficients (int spectrometerIndex, double *WavelengthCalibration)
{
	UN_TYPE unTemp;
	BYTE bTemp[32];
	int i = 0;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetWavelengthCalibrationCoefficients FAIL");
		return SA_API_FAIL;
	}

	for(i = 0; i < 4; i++)
	{
		unTemp.d64Data = WavelengthCalibration[i];
		bTemp[(i * 8)] = unTemp.ab8Data[7];
		bTemp[(i * 8) + 1] = unTemp.ab8Data[6];
		bTemp[(i * 8) + 2] = unTemp.ab8Data[5];
		bTemp[(i * 8) + 3] = unTemp.ab8Data[4];
		bTemp[(i * 8) + 4] = unTemp.ab8Data[3];
		bTemp[(i * 8) + 5] = unTemp.ab8Data[2];
		bTemp[(i * 8) + 6] = unTemp.ab8Data[1];
		bTemp[(i * 8) + 7] = unTemp.ab8Data[0];
	}

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, NVPA_WAVELENGTH_CALIB_ADDR, 32, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
//		printf_debug("SA_SetWavelengthCalibrationCoefficients::MIGP_RegArea_Write FAIL");
		return SA_API_FAIL;
	}

	for(i = 0; i < 4; i++)
	{
		apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[i] = WavelengthCalibration[i];
	}

	return SA_API_SUCCESS;
}


int SA_GetWavelengthCalibrationNewCoefficients (int spectrometerIndex, float * pfWavelengthData, float * pfPixelData)
{
	int i = 0;
	BYTE *pbTemp;
	UN_32TYPE un32Temp;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], NVPA, NVPA_WAVELENGTH_CALIB_NEW_ADDR, 128, &pbTemp, 100) == FALSE)
	{
		return SA_API_FAIL;
	}

	for(i = 0; i < 16; i++)
	{
		un32Temp.ab8Data[3] = pbTemp[4 + (i * 4)];
		un32Temp.ab8Data[2] = pbTemp[4 + (i * 4) + 1];
		un32Temp.ab8Data[1] = pbTemp[4 + (i * 4) + 2];
		un32Temp.ab8Data[0] = pbTemp[4 + (i * 4) + 3];
		pfWavelengthData[i] = un32Temp.f32Data;
	}

	for(i = 0; i < 16; i++)
	{
		un32Temp.ab8Data[3] = pbTemp[68 + (i * 4)];
		un32Temp.ab8Data[2] = pbTemp[68 + (i * 4) + 1];
		un32Temp.ab8Data[1] = pbTemp[68 + (i * 4) + 2];
		un32Temp.ab8Data[0] = pbTemp[68 + (i * 4) + 3];
		pfPixelData[i] = un32Temp.f32Data;
	}

	return SA_API_SUCCESS;
}
int SA_SetWavelengthCalibrationNewCoefficients (int spectrometerIndex, float * pfWavelengthData, float * pfPixelData)
{
	int i = 0;
	BYTE bTemp[8];
	UN_32TYPE un32Temp;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}
	
	for(i = 0; i < 16; i++)
	{
		un32Temp.f32Data = pfWavelengthData[i];
		bTemp[0] = un32Temp.ab8Data[3];
		bTemp[1] = un32Temp.ab8Data[2];
		bTemp[2] = un32Temp.ab8Data[1];
		bTemp[3] = un32Temp.ab8Data[0];

		if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, NVPA_WAVELENGTH_CALIB_NEW_ADDR + (i * 4), 4, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		{
			return SA_API_FAIL;
		}
		
	}

	for(i = 0; i < 16; i++)
	{
		un32Temp.f32Data = pfPixelData[i];
		bTemp[0] = un32Temp.ab8Data[3];
		bTemp[1] = un32Temp.ab8Data[2];
		bTemp[2] = un32Temp.ab8Data[1];
		bTemp[3] = un32Temp.ab8Data[0];

		if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, NVPA_WAVELENGTH_CALIB_NEW_ADDR + 64 + (i * 4), 4, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		{
			return SA_API_FAIL;
		}
		
	}


	return SA_API_SUCCESS;
}


int SA_GetWavelength(int spectrometerIndex, double *pdWavelengthData, int &pSpectumNumber)
{
	int i = 0;
	double dCalib[4];
	
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetWaveLength FAIL");
		return SA_API_FAIL;
	}

	dCalib[0] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[0];
	dCalib[1] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[1];
	dCalib[2] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[2];
	dCalib[3] = apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[3];
	
	pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	if(pSpectumNumber == 1024)
	{
		pSpectumNumber = 2048;
		//试用分段像素定标
		float fWL[16];
		float fPix[16];
		int iR = SA_GetWavelengthCalibrationNewCoefficients(0,fWL,fPix);
		if (iR == SA_API_SUCCESS)
		{
			for(i = 0; i < 16;i++)
			{
				if(fWL[i] <=0)
				{
					break;
				}
			}
			int iN = i;
			if(iN > 0)
			{
				for (i = 0; i < 2048; i++)
				{
					g_dPixWL_New[i]	= (float)(NewInt_float(fPix,fWL,iN,i*1.0f));
					pdWavelengthData[i] = (float)(NewInt_float(fPix,fWL,iN,i*1.0f));
				}
 				for(i = 0; i < 1024; i++)
 				{
 					g_dPixWL_Per[i] = g_dPixWL_New[i * 2 + 1];
 				}	
			}
			else
			{
							if(dCalib[0] < 0 || dCalib[0] > 1500)
				{
					dCalib[0] = 0;
					dCalib[1] = 0;
					dCalib[2] = 0;
					dCalib[3] = 0;
				}

				for(i = 0; i < 2048; i++)
				{
					double fWL = dCalib[0] + (dCalib[1] * i) + (dCalib[2] * i * i) + (dCalib[3] * i * i * i);

					g_dPixWL_New[i] = fWL;
					pdWavelengthData[i] = fWL;
 				}
 				for(i = 0; i < 1024; i++)
 				{
	 				g_dPixWL_Per[i] = g_dPixWL_New[i * 2 + 1];
 				}
			}
		}
		else
		{
				if(dCalib[0] < 0 || dCalib[0] > 1500)
				{
					dCalib[0] = 0;
					dCalib[1] = 0;
					dCalib[2] = 0;
					dCalib[3] = 0;
				}
			for(i = 0; i < 2048; i++)
			{
				double fWL = dCalib[0] + (dCalib[1] * i) + (dCalib[2] * i * i) + (dCalib[3] * i * i * i);

				g_dPixWL_New[i] = fWL;
				pdWavelengthData[i] = fWL;
 			}
		}
	}
	else
	{
		//试用分段像素定标
		float fWL[16];
		float fPix[16];
		int iR = SA_GetWavelengthCalibrationNewCoefficients(0,fWL,fPix);
		if (iR == SA_API_SUCCESS)
		{
			for(i = 0; i < 16;i++)
			{
				if(fWL[i] <=0)
				{
					break;
				}
			}
			int iN = i;
			if(iN > 0)
			{
				for (i = 0; i < 2048; i++)
				{
					g_dPixWL_New[i]	= (float)(NewInt_float(fPix,fWL,iN,i*1.0f));
					pdWavelengthData[i] = (float)(NewInt_float(fPix,fWL,iN,i*1.0f));
				} 				
			}
			else
			{
				if(dCalib[0] < 0 || dCalib[0] > 1500)
				{
					dCalib[0] = 0;
					dCalib[1] = 0;
					dCalib[2] = 0;
					dCalib[3] = 0;
				}
				for(i = 0; i < 2048; i++)
				{
					double fWL = dCalib[0] + (dCalib[1] * i) + (dCalib[2] * i * i) + (dCalib[3] * i * i * i);

					g_dPixWL_New[i] = fWL;
					pdWavelengthData[i] = fWL;
 				} 			
			}
		}
		else
		{
				if(dCalib[0] < 0 || dCalib[0] > 1500)
				{
					dCalib[0] = 0;
					dCalib[1] = 0;
					dCalib[2] = 0;
					dCalib[3] = 0;
				}
			for(i = 0; i < 2048; i++)
			{
				double fWL = dCalib[0] + (dCalib[1] * i) + (dCalib[2] * i * i) + (dCalib[3] * i * i * i);

				g_dPixWL_New[i] = fWL;
				pdWavelengthData[i] = fWL;
 			} 			
		}
		
	}
	g_bHaveRead = TRUE;

	return SA_API_SUCCESS;
}

/****************************************************************************************/
/****************************************************************************************/
/***************************** 缓冲异步触发相关操作接口**********************************/
/****************************************************************************************/
/****************************************************************************************/

int SA_ScanStartCacheAsyncTrigger(int spectrometerIndex, int iCacheChannelNum)
{
	BYTE bTemp;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(iCacheChannelNum > 7)
	{	
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != CACHE_ASYNCHRONOUS)
	{
		bTemp = 0x03;
		if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0000, 1, &bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		{
			return SA_API_FAIL;
		}

		apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = CACHE_ASYNCHRONOUS;
	}

	bTemp = ASYN_SCAN_CH1_INTEGRAL + iCacheChannelNum;
	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, VPA_ASYN_SCAN_ADDR, 1, &bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;
}

int SA_GetSpectumRBGModeRETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iCacheChannelNum)
{
	return SA_GetCacheAsyncSpectum(spectrometerIndex, pdSpectumData, pSpectumNumber, iCacheChannelNum);
}

int SA_GetSpectumRBGModeFETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iCacheChannelNum)
{
	return SA_GetCacheAsyncSpectum(spectrometerIndex, pdSpectumData, pSpectumNumber, iCacheChannelNum);
}

int SA_GetCacheAsyncSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iCacheChannelNum)
{
	BYTE *pbTemp;
	int i = 0;
	int iTimeOut;
	//HANDLE CommDev;
		
	if(spectrometerIndex > g_iCommIndex)
	{	
		
		return SA_API_FAIL;
	}

	if(iCacheChannelNum > 7)
	{	
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != CACHE_ASYNCHRONOUS)
	{
		return SA_API_FAIL;
	}
		
	iTimeOut = (int)(apCommParaST[spectrometerIndex]->stSpectrometerPara.iChannelIntegrationTime[iCacheChannelNum] / 1000) + 5;

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes <= 0)
	{
		apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes = 1;
	}

	if(apCommParaST[spectrometerIndex]->enCommType == USB_WINLIB)
	{
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 50;
	}
	else
	{
	
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 500;
	}

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], MDA, iCacheChannelNum, (apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber * 2), &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;
	}

	pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	if(pSpectumNumber == 1024)
	{
	//	MessageBox(NULL,TEXT("1024"),TEXT("YES"),MB_OK);
		double pTempData[1024];
		if(0)//!g_bHaveRead)
		{
			SA_GetWavelength(spectrometerIndex, g_pdWavelengthData, g_pTempSpectumNumber);
			g_bHaveRead = TRUE;
		}

		int iTempAD;
		for(i = 0; i < 1024; i++)
		{			
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pTempData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pTempData[i] = iTempAD;//反
			}
			else
			{
				iTempAD = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
				pTempData[i] = iTempAD;//正
			}
		}
		pSpectumNumber = 2048;//apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
		int iPerStart = 1;
		int m, k;
		float u;
		k=1;
		for(i = 0; i < 2048; i++)
 		{
 			if (1)
			{
				for(m = iPerStart; m <= 1024-2; m++)
				{
					if( g_dPixWL_New[i] <= g_dPixWL_Per[m] )
					{
						k=m;
						break;
					}
					else
						k=1024-1;
				}
				iPerStart = k;
				if ((g_dPixWL_Per[k]-g_dPixWL_Per[k-1]) != 0)
				{
					u=(float)((g_dPixWL_New[i]-g_dPixWL_Per[k-1])/(g_dPixWL_Per[k]-g_dPixWL_Per[k-1]));
				}
				else
				{
					u = 0;
				}
				pdSpectumData[i] = pTempData[k-1]+u*(pTempData[k]-pTempData[k-1]);
			}

 		}
		u=(float)((g_dPixWL_New[2047]-g_dPixWL_Per[1023])/(g_dPixWL_Per[1023]-g_dPixWL_Per[1022]));
		pdSpectumData[2047] = pTempData[1023]+u*(pTempData[1023]-pTempData[1022]);
	}
	else
	{
		int iTempAD;
		for(i = 0; i < pSpectumNumber; i++)
		{
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pdSpectumData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pdSpectumData[i] = iTempAD;//反
			}
			else
			{
				pdSpectumData[i] = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
			}
		}
	}

	if(g_bUseFFT)
	{
 //	MessageBox(NULL,TEXT("AAA"),TEXT("YES"),MB_OK);
		FFT_Filter(pdSpectumData, 2048);
	}

	if(g_window > 1)
	{
		WindowSmooth(pdSpectumData, 2048);
	}

	if(g_bNeedMod)
	{
//	MessageBox(NULL,TEXT("BBB"),TEXT("YES"),MB_OK);
		for(i = 0; i < 2048; i++)
		{
			//当前数据
			g_fPerSpec[g_iHadNum%g_iNeedNum][i] = pdSpectumData[i];
		}

		g_bHadData[g_iHadNum%g_iNeedNum] = TRUE;

		g_iHadNum++;
		if(g_iHadNum > g_iNeedNum)
		{
			g_iHadNum = 0;
		}

		double fSpecA=0; //总和
		double fMaxA = pdSpectumData[0];
		int iMaxA = 0;
		for(i = 0; i < 2048; i++)
		{
			fSpecA = fSpecA + pdSpectumData[i];
			if(fMaxA < pdSpectumData[i])
			{
				fMaxA = pdSpectumData[i];
				iMaxA = i;
			}
		}
		int iNumAve = 0;
		float fSum[2048];
		for(i = 0; i < 2048; i++)
			fSum[i] = 0;

		for(int j = 0; j < g_iNeedNum; j++)
		{
			//比较相似度
			double fSpecD=0; //差值
			if(g_bHadData[j])
			{
				double fMaxD = g_fPerSpec[j][0];
				int iMaxD = 0;
				for(i = 0; i < 2048; i++)
				{
					fSpecD = fSpecD + g_fPerSpec[j][i];
					if(fMaxD < g_fPerSpec[j][i])
					{
						fMaxD = g_fPerSpec[j][i];
						iMaxD = i;
					}
				}
				if(fSpecA!=0)
				{
					if((abs(iMaxD - iMaxA) < 8 && fMaxA > 2500)
					|| fMaxA < 2500) //判断峰值
					{
						if(fabs((fSpecD - fSpecA) / fSpecA) <= g_fValue)
						{
							for(i = 0; i < 2048; i++)
								fSum[i] = (float)((g_fPerSpec[j][i] + fSum[i]));
					
							iNumAve++;
						}
					}
				}
			}
			
		}
			if(iNumAve >0)
			{
				for(i = 0; i < 2048; i++)
					pdSpectumData[i] = (float)((fSum[i]/iNumAve));
			}
	}


	
	return SA_API_SUCCESS;
}


/****************************************************************************************/
/****************************************************************************************/
/***************************** 软件异步触发相关操作接口**********************************/
/****************************************************************************************/
/****************************************************************************************/

int SA_ScanStartAsyncSoftTrigger(int spectrometerIndex)
{
	BYTE bTemp;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_ASYNCHRONOUS)
	{
		bTemp = 0x02;
		if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0000, 1, &bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		{
			return SA_API_FAIL;
		}

		apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = SOFTWARE_ASYNCHRONOUS;
	}

	bTemp = ASYN_SCAN_INTEGRAL;
	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, VPA_ASYN_SCAN_ADDR, 1, &bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	g_bTrig = TRUE;

	return SA_API_SUCCESS;

}

int SA_ScanStartMultiChannelAsyncSoftTrigger(int spectrometerIndex, int iChannelNum)
{
	BYTE bTemp;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_ASYNCHRONOUS)
	{
		bTemp = 0x02;
		if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0000, 1, &bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		{
			return SA_API_FAIL;
		}

		apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = SOFTWARE_ASYNCHRONOUS;
	}

	bTemp = ASYN_SCAN_CH1_INTEGRAL + iChannelNum;
	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, VPA_ASYN_SCAN_ADDR, 1, &bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;

}

int SA_GetStateAsyncSoftTrigger(int spectrometerIndex, int *pState)
{
	BYTE *pbTemp;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}
	
	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], VPA, VPA_ASYN_SCAN_ADDR, 1, &pbTemp, 1000) == FALSE)
	{
		return SA_API_FAIL;
	}

	*pState = pbTemp[4];
		
	return SA_API_SUCCESS;

}

int SA_GetStateMultiChannelAsyncSoftTrigger(int spectrometerIndex, int *pState)
{
	return SA_GetStateAsyncSoftTrigger(spectrometerIndex, pState);
}

int SA_GetStateCacheAsyncTrigger(int spectrometerIndex, int *pState)
{
	return SA_GetStateAsyncSoftTrigger(spectrometerIndex, pState);
}

/************************************************************/
/************************************************************/
/*************** 光谱仪获取光谱操作接口 *********************/
/************************************************************/
/************************************************************/

int SA_OpenModSpec(BOOL bOpen,int iNum, float fValue)
{
	g_bNeedMod = bOpen;
	g_iNeedNum = iNum;
	g_fValue	= fValue;
	return SA_API_SUCCESS;
}

int SA_UseFFTFilter(BOOL bUseFFT,float fValue)
{
	g_bUseFFT = bUseFFT;
	int iPixN = 2048;
	if(bUseFFT)
	{
		int inum = (int)(iPixN*fValue);
		FFT_initFilter(iPixN,inum);
	}
	return SA_API_SUCCESS;
}


int SA_GetSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber)
{
	if(!g_bTrig || g_iAve == 1)
	{
		return SA_GetSpectumN(spectrometerIndex, pdSpectumData, pSpectumNumber);	
	}
	else
	{
		double pdTempSpectumData[2048];
		for(int j = 0; j < 2048; j++)
		{
			pdSpectumData[j] = 0;
		}
		for(int i = 0; i < g_iAve; i++)
		{
			SA_ScanStartAsyncSoftTrigger(spectrometerIndex);
			SA_GetSpectumN(spectrometerIndex, pdTempSpectumData, pSpectumNumber);
			for(int j = 0; j < pSpectumNumber; j++)
			{
				pdSpectumData[j] = pdSpectumData[j] + pdTempSpectumData[j]/g_iAve;
			}
		}
		return SA_API_SUCCESS;
	}
}

int SA_GetSpectumN(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber)
{
	BYTE *pbTemp;
	int i = 0;
	int iTimeOut;

	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_SYNCHRONOUS
	   && apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_AUTO
	   && apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_ASYNCHRONOUS
	   && apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != CACHE_ASYNCHRONOUS
	   && apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != EXINT_RISING_EDGE
	   && apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != EXINT_FALLING_EDGE
	   && apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != EXINT_RGB_RISING_EDGE
	   && apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != EXINT_RGB_FALLING_EDGE)
	{
		return SA_API_FAIL;
	}
		
	iTimeOut = (int)(apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime / 1000) + 4;

	if(apCommParaST[spectrometerIndex]->enCommType == USB_WINLIB)
	{
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 100;
	}
	else
	{	
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 500;
	}

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], MDA, 0x0000, (apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber * 2), &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;
	}

	pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	if(pSpectumNumber == 1024)
	{
		double pTempData[1024];
		if(0)//!g_bHaveRead)
		{
			SA_GetWavelength(spectrometerIndex, g_pdWavelengthData, g_pTempSpectumNumber);
			g_bHaveRead = TRUE;
		}

		int iTempAD;
		for(i = 0; i < 1024; i++)
		{			
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pTempData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pTempData[i] = iTempAD;//反
			}
			else
			{
				iTempAD = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
				pTempData[i] = iTempAD;//正
			}
		}
		pSpectumNumber = 2048;//apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
		int iPerStart = 1;
		int m, k;
		float u;
		k=1;
		for(i = 0; i < 2048; i++)
 		{
 			if (1)
			{
				for(m = iPerStart; m <= 1024-2; m++)
				{
					if( g_dPixWL_New[i] <= g_dPixWL_Per[m] )
					{
						k=m;
						break;
					}
					else
						k=1024-1;
				}
				iPerStart = k;
				if ((g_dPixWL_Per[k]-g_dPixWL_Per[k-1]) != 0)
				{
					u=(float)((g_dPixWL_New[i]-g_dPixWL_Per[k-1])/(g_dPixWL_Per[k]-g_dPixWL_Per[k-1]));
				}
				else
				{
					u = 0;
				}
				pdSpectumData[i] = pTempData[k-1]+u*(pTempData[k]-pTempData[k-1]);
			}

 		}
		u=(float)((g_dPixWL_New[2047]-g_dPixWL_Per[1023])/(g_dPixWL_Per[1023]-g_dPixWL_Per[1022]));
		pdSpectumData[2047] = pTempData[1023]+u*(pTempData[1023]-pTempData[1022]);
	}
	else
	{
		int iTempAD;
		for(i = 0; i < pSpectumNumber; i++)
		{
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pdSpectumData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pdSpectumData[i] = iTempAD;//反
			}
			else
			{
				pdSpectumData[i] = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
			}
		}
	}

	if(g_bUseFFT)
	{
 //	MessageBox(NULL,TEXT("AAA"),TEXT("YES"),MB_OK);
		FFT_Filter(pdSpectumData, 2048);
	}
	if(g_window > 1)
	{
		WindowSmooth(pdSpectumData, 2048);
	}


	if(g_bNeedMod)
	{
//	MessageBox(NULL,TEXT("BBB"),TEXT("YES"),MB_OK);
		for(i = 0; i < 2048; i++)
		{
			//当前数据
			g_fPerSpec[g_iHadNum%g_iNeedNum][i] = pdSpectumData[i];
		}

		g_bHadData[g_iHadNum%g_iNeedNum] = TRUE;

		g_iHadNum++;
		if(g_iHadNum > g_iNeedNum)
		{
			g_iHadNum = 0;
		}

		double fSpecA=0; //总和
		double fMaxA = pdSpectumData[0];
		int iMaxA = 0;
		for(i = 0; i < 2048; i++)
		{
			fSpecA = fSpecA + pdSpectumData[i];
			if(fMaxA < pdSpectumData[i])
			{
				fMaxA = pdSpectumData[i];
				iMaxA = i;
			}
		}
		int iNumAve = 0;
		float fSum[2048];
		for(i = 0; i < 2048; i++)
			fSum[i] = 0;

		for(int j = 0; j < g_iNeedNum; j++)
		{
			//比较相似度
			double fSpecD=0; //差值
			if(g_bHadData[j])
			{
				double fMaxD = g_fPerSpec[j][0];
				int iMaxD = 0;
				for(i = 0; i < 2048; i++)
				{
					fSpecD = fSpecD + g_fPerSpec[j][i];
					if(fMaxD < g_fPerSpec[j][i])
					{
						fMaxD = g_fPerSpec[j][i];
						iMaxD = i;
					}
				}
				if(fSpecA!=0)
				{
					if((abs(iMaxD - iMaxA) < 8 && fMaxA > 2500)
					|| fMaxA < 2500) //判断峰值
					{
						if(fabs((fSpecD - fSpecA) / fSpecA) <= g_fValue)
						{
							for(i = 0; i < 2048; i++)
								fSum[i] = (float)((g_fPerSpec[j][i] + fSum[i]));
					
							iNumAve++;
						}
					}
				}
			}
			
		}
			if(iNumAve >0)
			{
				for(i = 0; i < 2048; i++)
					pdSpectumData[i] = (float)((fSum[i]/iNumAve));
			}
	}

	return SA_API_SUCCESS;
}
double g_pdTempSpectumData1[2048];

int SA_GetAsyncSoftSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber)
{
	if(g_iAve == 1)
		return SA_GetSpectumN(spectrometerIndex, pdSpectumData, pSpectumNumber);
	else
	{
		for(int j = 0; j < 2048; j++)
		{
			pdSpectumData[j] = 0;
		}
		for(int i = 0; i < g_iAve; i++)
		{
			if(i!=0)
				SA_ScanStartAsyncSoftTrigger(spectrometerIndex);
			SA_GetSpectumN(spectrometerIndex, g_pdTempSpectumData1, pSpectumNumber);
			for(int j = 0; j < pSpectumNumber; j++)
			{
				pdSpectumData[j] = pdSpectumData[j] + g_pdTempSpectumData1[j]/g_iAve;
			}
			Sleep(1);
		}
		return SA_API_SUCCESS;
	}
}

int SA_GetMultiChannelAsyncSoftSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber)
{
	return SA_GetSpectumN(spectrometerIndex, pdSpectumData, pSpectumNumber);
}

int SA_GetSpectumRETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber)
{
	return SA_GetSpectumN(spectrometerIndex, pdSpectumData, pSpectumNumber);
}

int SA_GetSpectumFETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber)
{
	return SA_GetSpectumN(spectrometerIndex, pdSpectumData, pSpectumNumber);
}

/************************************************************/
/************************************************************/
/*************** 光谱仪自动积分操作接口 *********************/
/************************************************************/
/************************************************************/

int SA_GetSpectumAutoIntegrationTime(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int usec)
{
	BYTE *pbTemp;
	int i = 0;
	int iTimeOut;
	BYTE bTemp[4];
	unsigned long ulTemp = 0;

	//HANDLE CommDev;
		
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_SYNCHRONOUS && 
	   apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_AUTO)
	{
		return SA_API_FAIL;
	}

	apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime = usec;
		
	iTimeOut = (int)(apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime / 1000) + 5;

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes <= 0)
	{
		apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes = 1;
	}
	
	if(apCommParaST[spectrometerIndex]->enCommType == USB_WINLIB)
	{
		
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 50;
		/*****test*****/
		//iTimeOut = 20;
		//printf("iTimeOut = %d\r\n", iTimeOut);
	}
	else
	{
	
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 500;
	}


	ulTemp = (unsigned long)(usec);
	bTemp[0] = (BYTE)(ulTemp >> 24 & 0x000000ff);
	bTemp[1] = (BYTE)(ulTemp >> 16 & 0x000000ff);
	bTemp[2] = (BYTE)(ulTemp >> 8 & 0x000000ff);
	bTemp[3] = (BYTE)(ulTemp & 0x000000ff);
	if(MIGP_RegArea_Read_Plus(apCommParaST[spectrometerIndex], MDA, 0x0011, (apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber * 2), &pbTemp, iTimeOut, bTemp,4) == FALSE)
	{
		return SA_API_FAIL;
	}

	*pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 0x01)
	{
		double iTempAD;
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			iTempAD = (double)((double)pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256);//高位在后 新
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
				pdSpectumData[*pSpectumNumber-1-i] = iTempAD;//反
			else
				pdSpectumData[i] = iTempAD;//反

		}
	}
	else
	{
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
		}	
	}
	

	
	return SA_API_SUCCESS;
}


int SA_GetMultiChannelSpectum(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int iChannelNum)
{
	BYTE *pbTemp;
	int i = 0;
	int iTimeOut;
	//HANDLE CommDev;

		
	if(spectrometerIndex > g_iCommIndex)
	{	
		
		return SA_API_FAIL;
	}

	if(iChannelNum >= CHL_NUM)
	{	
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_SYNCHRONOUS)
	{
		return SA_API_FAIL;
	}
		
	iTimeOut = (int)(apCommParaST[spectrometerIndex]->stSpectrometerPara.iChannelIntegrationTime[iChannelNum] / 1000) + 5;

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes <= 0)
	{
		apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes = 1;
	}

	if(apCommParaST[spectrometerIndex]->enCommType == USB_WINLIB)
	{
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 50;
	}
	else
	{
	
		iTimeOut = iTimeOut * apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes + 500;
	}


	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], MDA, 0x0001 + iChannelNum, (apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber * 2), &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;
	}

	*pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 0x01)
	{	
		int iTempAD;
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
				pdSpectumData[*pSpectumNumber-1-i] = iTempAD;//反
			else
				pdSpectumData[i] = iTempAD;//反
		}
	}
	else
	{
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
		}	
	}
	//for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
	//{
	//	pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
	//}



	
	return SA_API_SUCCESS;
}



int SA_GetSpectumHLTrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iTimeOut)
{
	BYTE *pbTemp;
	int i = 0;
	int ret = 0;
	
	if(SA_SetSpectumTriggerMode(spectrometerIndex, EXINT_HIGH_LEVEL) == SA_API_FAIL)
	{
		return SA_API_FAIL;
	}

	/*获取光谱*/
	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], MDA, 0x0000, (apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber * 2), &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;
	}

	pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	if(pSpectumNumber == 1024)
	{
	//	MessageBox(NULL,TEXT("1024"),TEXT("YES"),MB_OK);
		double pTempData[1024];
		if(0)//!g_bHaveRead)
		{
			SA_GetWavelength(spectrometerIndex, g_pdWavelengthData, g_pTempSpectumNumber);
			g_bHaveRead = TRUE;
		}

		int iTempAD;
		for(i = 0; i < 1024; i++)
		{			
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pdSpectumData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pdSpectumData[i] = iTempAD;//反
			}
			else
			{
				iTempAD = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
				pTempData[i] = iTempAD;//正
			}
		}
		pSpectumNumber = 2048;//apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
		int iPerStart = 1;
		int m, k;
		float u;
		k=1;
		for(i = 0; i < 2048; i++)
 		{
 			if (1)
			{
				for(m = iPerStart; m <= 1024-2; m++)
				{
					if( g_dPixWL_New[i] <= g_dPixWL_Per[m] )
					{
						k=m;
						break;
					}
					else
						k=1024-1;
				}
				iPerStart = k;
				if ((g_dPixWL_Per[k]-g_dPixWL_Per[k-1]) != 0)
				{
					u=(float)((g_dPixWL_New[i]-g_dPixWL_Per[k-1])/(g_dPixWL_Per[k]-g_dPixWL_Per[k-1]));
				}
				else
				{
					u = 0;
				}
				pdSpectumData[i] = pTempData[k-1]+u*(pTempData[k]-pTempData[k-1]);
			}

 		}
		u=(float)((g_dPixWL_New[2047]-g_dPixWL_Per[1023])/(g_dPixWL_Per[1023]-g_dPixWL_Per[1022]));
		pdSpectumData[2047] = pTempData[1023]+u*(pTempData[1023]-pTempData[1022]);
	}
	else
	{
		double iTempAD;
		for(i = 0; i < pSpectumNumber; i++)
		{
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pdSpectumData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pdSpectumData[i] = iTempAD;//反
			}
			else
			{
				pdSpectumData[i] = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
			}
		}
	}

	if(g_bUseFFT)
	{
 //	MessageBox(NULL,TEXT("AAA"),TEXT("YES"),MB_OK);
		FFT_Filter(pdSpectumData, 2048);
	}
	if(g_window > 1)
	{
		WindowSmooth(pdSpectumData, 2048);
	}


	if(g_bNeedMod)
	{
//	MessageBox(NULL,TEXT("BBB"),TEXT("YES"),MB_OK);
		for(i = 0; i < 2048; i++)
		{
			//当前数据
			g_fPerSpec[g_iHadNum%g_iNeedNum][i] = pdSpectumData[i];
		}

		g_bHadData[g_iHadNum%g_iNeedNum] = TRUE;

		g_iHadNum++;
		if(g_iHadNum > g_iNeedNum)
		{
			g_iHadNum = 0;
		}

		double fSpecA=0; //总和
		double fMaxA = pdSpectumData[0];
		int iMaxA = 0;
		for(i = 0; i < 2048; i++)
		{
			fSpecA = fSpecA + pdSpectumData[i];
			if(fMaxA < pdSpectumData[i])
			{
				fMaxA = pdSpectumData[i];
				iMaxA = i;
			}
		}
		int iNumAve = 0;
		float fSum[2048];
		for(i = 0; i < 2048; i++)
			fSum[i] = 0;

		for(int j = 0; j < g_iNeedNum; j++)
		{
			//比较相似度
			double fSpecD=0; //差值
			if(g_bHadData[j])
			{
				double fMaxD = g_fPerSpec[j][0];
				int iMaxD = 0;
				for(i = 0; i < 2048; i++)
				{
					fSpecD = fSpecD + g_fPerSpec[j][i];
					if(fMaxD < g_fPerSpec[j][i])
					{
						fMaxD = g_fPerSpec[j][i];
						iMaxD = i;
					}
				}
				if(fSpecA!=0)
				{
					if((abs(iMaxD - iMaxA) < 8 && fMaxA > 2500)
					|| fMaxA < 2500) //判断峰值
					{
						if(fabs((fSpecD - fSpecA) / fSpecA) <= g_fValue)
						{
							for(i = 0; i < 2048; i++)
								fSum[i] = (float)((g_fPerSpec[j][i] + fSum[i]));
					
							iNumAve++;
						}
					}
				}
			}
			
		}
			if(iNumAve >0)
			{
				for(i = 0; i < 2048; i++)
					pdSpectumData[i] = (float)((fSum[i]/iNumAve));
			}
	}

	apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = (BYTE)SOFTWARE_SYNCHRONOUS;
	
	return SA_API_SUCCESS;
}

int SA_GetSpectumLLTrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iTimeOut)
{
	BYTE *pbTemp;
	int i = 0;
	int ret = 0;
	
	if(SA_SetSpectumTriggerMode(spectrometerIndex, EXINT_LOW_LEVEL) == SA_API_FAIL)
	{
		return SA_API_FAIL;
	}

	/*获取光谱*/
	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], MDA, 0x0000, (apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber * 2), &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;
	}

	pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	if(pSpectumNumber == 1024)
	{
	//	MessageBox(NULL,TEXT("1024"),TEXT("YES"),MB_OK);
		double pTempData[1024];
		if(0)//!g_bHaveRead)
		{
			SA_GetWavelength(spectrometerIndex, g_pdWavelengthData, g_pTempSpectumNumber);
			g_bHaveRead = TRUE;
		}

		int iTempAD;
		for(i = 0; i < 1024; i++)
		{			
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pTempData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pTempData[i] = iTempAD;//反
			}
			else
			{
				iTempAD = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
				pTempData[i] = iTempAD;//正
			}
		}
		pSpectumNumber = 2048;//apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
		int iPerStart = 1;
		int m, k;
		float u;
		k=1;
		for(i = 0; i < 2048; i++)
 		{
 			if (1)
			{
				for(m = iPerStart; m <= 1024-2; m++)
				{
					if( g_dPixWL_New[i] <= g_dPixWL_Per[m] )
					{
						k=m;
						break;
					}
					else
						k=1024-1;
				}
				iPerStart = k;
				if ((g_dPixWL_Per[k]-g_dPixWL_Per[k-1]) != 0)
				{
					u=(float)((g_dPixWL_New[i]-g_dPixWL_Per[k-1])/(g_dPixWL_Per[k]-g_dPixWL_Per[k-1]));
				}
				else
				{
					u = 0;
				}
				pdSpectumData[i] = pTempData[k-1]+u*(pTempData[k]-pTempData[k-1]);
			}

 		}
		u=(float)((g_dPixWL_New[2047]-g_dPixWL_Per[1023])/(g_dPixWL_Per[1023]-g_dPixWL_Per[1022]));
		pdSpectumData[2047] = pTempData[1023]+u*(pTempData[1023]-pTempData[1022]);
	}
	else
	{
		double iTempAD;
		for(i = 0; i < pSpectumNumber; i++)
		{
			if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 1)
			{
				iTempAD = pbTemp[4 + (i * 2)] + pbTemp[4 + (i * 2) + 1] * 256;//高位在后 新
				if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPosType == 1)
					pdSpectumData[pSpectumNumber-1-i] = iTempAD;//反
				else
					pdSpectumData[i] = iTempAD;//反
			}
			else
			{
				pdSpectumData[i] = pbTemp[4 + (i * 2)] * 256 + pbTemp[4 + (i * 2) + 1];//高位在前 老
			}
		}
	}

	if(g_bUseFFT)
	{
 //	MessageBox(NULL,TEXT("AAA"),TEXT("YES"),MB_OK);
		FFT_Filter(pdSpectumData, 2048);
	}
	if(g_window > 1)
	{
		WindowSmooth(pdSpectumData, 2048);
	}

	if(g_bNeedMod)
	{
//	MessageBox(NULL,TEXT("BBB"),TEXT("YES"),MB_OK);
		for(i = 0; i < 2048; i++)
		{
			//当前数据
			g_fPerSpec[g_iHadNum%g_iNeedNum][i] = pdSpectumData[i];
		}

		g_bHadData[g_iHadNum%g_iNeedNum] = TRUE;

		g_iHadNum++;
		if(g_iHadNum > g_iNeedNum)
		{
			g_iHadNum = 0;
		}

		double fSpecA=0; //总和
		double fMaxA = pdSpectumData[0];
		int iMaxA = 0;
		for(i = 0; i < 2048; i++)
		{
			fSpecA = fSpecA + pdSpectumData[i];
			if(fMaxA < pdSpectumData[i])
			{
				fMaxA = pdSpectumData[i];
				iMaxA = i;
			}
		}
		int iNumAve = 0;
		float fSum[2048];
		for(i = 0; i < 2048; i++)
			fSum[i] = 0;

		for(int j = 0; j < g_iNeedNum; j++)
		{
			//比较相似度
			double fSpecD=0; //差值
			if(g_bHadData[j])
			{
				double fMaxD = g_fPerSpec[j][0];
				int iMaxD = 0;
				for(i = 0; i < 2048; i++)
				{
					fSpecD = fSpecD + g_fPerSpec[j][i];
					if(fMaxD < g_fPerSpec[j][i])
					{
						fMaxD = g_fPerSpec[j][i];
						iMaxD = i;
					}
				}
				if(fSpecA!=0)
				{
					if((abs(iMaxD - iMaxA) < 8 && fMaxA > 2500)
					|| fMaxA < 2500) //判断峰值
					{
						if(fabs((fSpecD - fSpecA) / fSpecA) <= g_fValue)
						{
							for(i = 0; i < 2048; i++)
								fSum[i] = (float)((g_fPerSpec[j][i] + fSum[i]));
					
							iNumAve++;
						}
					}
				}
			}
			
		}
			if(iNumAve >0)
			{
				for(i = 0; i < 2048; i++)
					pdSpectumData[i] = (float)((fSum[i]/iNumAve));
			}
	}

	apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = (BYTE)SOFTWARE_SYNCHRONOUS;
	
	return SA_API_SUCCESS;
}


/************************************************************/
/************************************************************/
/*************** 光谱仪信息获取操作接口 *********************/
/************************************************************/
/************************************************************/


char * SA_GetSpectrometersName(int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetSpectrometersName FAIL");
		return NULL;
	}

	return apCommParaST[spectrometerIndex]->stEIA.aucEquipmentInfo;
}

int SA_GetMaxIntegrationTime (int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetMaxIntegrationTime FAIL");
		return 0;
	}

	return apCommParaST[spectrometerIndex]->stSpectrometerPara.iMaxIntegrationTimeUS;
}

int SA_GetMinIntegrationTime (int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetMinIntegrationTime FAIL");
		return 0;
	}

	return apCommParaST[spectrometerIndex]->stSpectrometerPara.iMinIntegrationTimeUS;
}

char * SA_GetSoftwareVersion(int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetSoftwareVersion FAIL");
		return NULL;
	}

	return apCommParaST[spectrometerIndex]->stEIA.aucSoftWareVersion;
}

char SA_GetHardwareVersion(int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetHardwareVersion FAIL");
		return NULL;
	}

	return apCommParaST[spectrometerIndex]->stEIA.ucHardWareVersion;
}


char * SA_GetSerialNumber(int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetSerialNumber FAIL");
		return NULL;
	}

	return apCommParaST[spectrometerIndex]->stEIA.aucSerialNumber;
}

char * SA_GetManufacturingDate(int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetSerialNumber FAIL");
		return NULL;
	}

	return apCommParaST[spectrometerIndex]->stEIA.aucManufacturingDate;
}

int SA_GetSpectrometerPixelsNumber (int spectrometerIndex)
{
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetSpectrometerPixelsNumber FAIL");
		return 0;
	}

	int pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	return 2048;//pSpectumNumber;
}

/****************************************************************************************/
/****************************************************************************************/
/***************************** 非线性定标操作接口****************************************/
/****************************************************************************************/
/****************************************************************************************/

int SA_NonlinearCalibration(int spectrometerIndex, double * pbSpectum, double * pbNewSpectum, int SpectumNumber)
{
	int i = 0;
	int j = 0;
	float fTemp = 0;
	float fTemp2 = 0;
	float fCoTemp = 0;
	int iMaxPixelOrWL = 0;
	float fWL = 0;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_NonlinearCalibration FAIL");
		return SA_API_FAIL;
	}

	//非线性关闭
	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_DISABLE )
	{
		for(i = 0; i < SpectumNumber; i++)
		{
			pbNewSpectum[i] = pbSpectum[i];
		}
	}
	else if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_WAVELENGTH || 
		    apCommParaST[spectrometerIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_WAVELENGTH_II)
	{ 
		//检查非线性系数个数
		if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength < 1 ||
		   apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength > 8)
		{
			for(i = 0; i < SpectumNumber; i++)
			{
				pbNewSpectum[i] = pbSpectum[i];
			}
		}
		//单个非线性系数
		else if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength == 1)
		{
			for(i = 0; i < SpectumNumber; i++)
			{

				fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[0],
										    apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[0], 
										    apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[0], 
										    (float)pbSpectum[i]);
				pbNewSpectum[i] = pbSpectum[i] * fTemp;
			}
			
		}
		else
		{
			iMaxPixelOrWL = apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength - 1;
			int start = 1;
			for(i = 0; i < SpectumNumber; i++)
			{
				fWL = (float)(apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[0] + 
					  (apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[1] * i) + 
					  (apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[2] * i * i) + 
					  (apCommParaST[spectrometerIndex]->stSpectrometerPara.adWavelengthCalib[3] * i * i * i));
				if(fWL <= apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[0])
				{
					fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[0],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[0], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[0], 
											    (float)pbSpectum[i]);
					pbNewSpectum[i] = pbSpectum[i] * fTemp;
				}
				else if(fWL >= apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[iMaxPixelOrWL])
				{
					fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[iMaxPixelOrWL],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[iMaxPixelOrWL], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[iMaxPixelOrWL], 
											    (float)pbSpectum[i]);
					pbNewSpectum[i] = pbSpectum[i] * fTemp;
				}
				else
				{
					for(j = 1; j < apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength; j++)
					{
						if(fWL < apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j])
						{
							break;
						}
					}

					fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[j-1],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[j-1], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[j-1], 
											    (float)pbSpectum[i]);
					fTemp2 = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[j],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[j], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[j], 
											    (float)pbSpectum[i]);
					fCoTemp = fWL - apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j-1];
					fCoTemp = fCoTemp / (apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j] - 
						      apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j-1]);
					fCoTemp = (fCoTemp * (fTemp2 - fTemp)) + fTemp;
					pbNewSpectum[i] = pbSpectum[i] * fCoTemp;
					
				}
				
			}
		}
	}
	else if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_PIXEL || 
			apCommParaST[spectrometerIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_PIXEL_II)
	{ 
		if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength < 1 ||
		   apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength > 8)
		{
			for(i = 0; i < SpectumNumber; i++)
			{
				pbNewSpectum[i] = pbSpectum[i];
			}
		}
		else if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength == 1)
		{
			for(i = 0; i < SpectumNumber; i++)
			{
				fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[0],
										    apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[0], 
										    apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[0], 
										    (float)pbSpectum[i]);
				pbNewSpectum[i] = pbSpectum[i] * fTemp;
			}
			
		}
		else
		{
			iMaxPixelOrWL = apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength - 1;
			for(i = 0; i < SpectumNumber; i++)
			{
				if(i <= apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[0])
				{
					fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[0],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[0], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[0], 
											    (float)pbSpectum[i]);
					pbNewSpectum[i] = pbSpectum[i] * fTemp;
				}
				else if(i >= apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[iMaxPixelOrWL])
				{
					fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[iMaxPixelOrWL],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[iMaxPixelOrWL], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[iMaxPixelOrWL], 
											    (float)pbSpectum[i]);
					pbNewSpectum[i] = pbSpectum[i] * fTemp;
				}
				else
				{
					for(j = 1; j < apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibLength; j++)
					{
						if(i < apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j])
						{
							break;
						}
					}

					fTemp = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[j-1],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[j-1], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[j-1], 
											    (float)pbSpectum[i]);
					fTemp2 = SA_NewtonDifference(apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[j],
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[j], 
										        apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[j], 
											    (float)pbSpectum[i]);
					fCoTemp = i - apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j-1];
					fCoTemp = fCoTemp / (apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j] - 
						      apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[j-1]);
					fCoTemp = (fCoTemp * (fTemp2 - fTemp)) + fTemp;
					pbNewSpectum[i] = pbSpectum[i] * fCoTemp;
					
				}
				
			}
		}
	}
	else
	{
		for(i = 0; i < SpectumNumber; i++)
		{
			pbNewSpectum[i] = pbSpectum[i];
		}
	}

	return SA_API_SUCCESS;
}

int SA_SetNonlinearCalibrationPixel(int spectrometerIndex, float fPixelValue, int ParaNum, float *pfADValue, float *pfCalibrationCo)
{
	BYTE bTemp[64];
	unsigned long ulTemp = 0;
	UN_32TYPE un32Temp;
	int i = 0;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}
	
	bTemp[0] = NONLINEAR_CALIB_PIXEL_II;
		
	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0040, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		
		return SA_API_FAIL;
	}
	
	un32Temp.f32Data = 1;
	bTemp[0] = un32Temp.ab8Data[3];
	bTemp[1] = un32Temp.ab8Data[2];
	bTemp[2] = un32Temp.ab8Data[1];
	bTemp[3] = un32Temp.ab8Data[0];
	
	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0041, 4, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		
		return SA_API_FAIL;
	}
	
	un32Temp.f32Data = fPixelValue;
	bTemp[0] = un32Temp.ab8Data[3];
	bTemp[1] = un32Temp.ab8Data[2];
	bTemp[2] = un32Temp.ab8Data[1];
	bTemp[3] = un32Temp.ab8Data[0];

	un32Temp.f32Data = (float)ParaNum;
	bTemp[4] = un32Temp.ab8Data[3];
	bTemp[5] = un32Temp.ab8Data[2];
	bTemp[6] = un32Temp.ab8Data[1];
	bTemp[7] = un32Temp.ab8Data[0];
	
	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0045, 8, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		
		return SA_API_FAIL;
	}
	
	for(i = 0; i < ParaNum; i++)
	{
		un32Temp.f32Data = pfADValue[i];
		bTemp[0] = un32Temp.ab8Data[3];
		bTemp[1] = un32Temp.ab8Data[2];
		bTemp[2] = un32Temp.ab8Data[1];
		bTemp[3] = un32Temp.ab8Data[0];
		un32Temp.f32Data = pfCalibrationCo[i];
		bTemp[4] = un32Temp.ab8Data[3];
		bTemp[5] = un32Temp.ab8Data[2];
		bTemp[6] = un32Temp.ab8Data[1];
		bTemp[7] = un32Temp.ab8Data[0];
		if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0045 + 8 + (i * 8), 8, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		{
			
			return SA_API_FAIL;
		}
	}
	
	
	return SA_API_SUCCESS;
/*	BYTE bTemp[64];
	unsigned long ulTemp = 0;
	UN_32TYPE un32Temp;
	int i = 0;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	bTemp[0] = NONLINEAR_CALIB_PIXEL_II;


	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0040, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{

		return SA_API_FAIL;
	}
	Sleep(100);

	un32Temp.f32Data = 1;
	bTemp[0] = un32Temp.ab8Data[3];
	bTemp[1] = un32Temp.ab8Data[2];
	bTemp[2] = un32Temp.ab8Data[1];
	bTemp[3] = un32Temp.ab8Data[0];

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0040, 5, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{

		return SA_API_FAIL;
	}
	Sleep(100);

	un32Temp.f32Data = fPixelValue;
	bTemp[0] = un32Temp.ab8Data[3];
	bTemp[1] = un32Temp.ab8Data[2];
	bTemp[2] = un32Temp.ab8Data[1];
	bTemp[3] = un32Temp.ab8Data[0];
	un32Temp.f32Data = (float)ParaNum;
	bTemp[4] = un32Temp.ab8Data[3];
	bTemp[5] = un32Temp.ab8Data[2];
	bTemp[6] = un32Temp.ab8Data[1];
	bTemp[7] = un32Temp.ab8Data[0];

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0045, 8, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{

		return SA_API_FAIL;
	}

	for(i = 0; i < ParaNum; i++)
	{
		un32Temp.f32Data = pfADValue[i];
		bTemp[0] = un32Temp.ab8Data[3];
		bTemp[1] = un32Temp.ab8Data[2];
		bTemp[2] = un32Temp.ab8Data[1];
		bTemp[3] = un32Temp.ab8Data[0];
		un32Temp.f32Data = pfCalibrationCo[i];
		bTemp[4] = un32Temp.ab8Data[3];
		bTemp[5] = un32Temp.ab8Data[2];
		bTemp[6] = un32Temp.ab8Data[1];
		bTemp[7] = un32Temp.ab8Data[0];
		if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0045 + 8 + (i * 8), 8, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
		{

			return SA_API_FAIL;
		}
	}

	
	return SA_API_SUCCESS;*/
}

int SA_GetNonlinearCalibrationPixel(int spectrometerIndex, 
										 float &pfNonlinearCalibPixelOrWL, int &piNonlinearCalibCoNumber,
	                                     float pfNonlinearCalibAD[], float pfNonlinearCalibCo[])
{
	BYTE *pbTemp;
	int i = 0;
	int j = 0;
	int CommFlag = 0;
	UN_32TYPE un32Temp;
	int iLengthTemp = 0;

	int CommIndex = 0;


	if(MIGP_RegArea_Read(apCommParaST[CommIndex], NVPA, 0x0040, 1, &pbTemp, 500) == TRUE)
	{
		Sleep(100);
		PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
		apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = pbTemp[4];

		//printf("cNonlinearCalibType %d \r\n", (int)apCommParaST[g_iCommIndex + 1]->stSpectrometerPara.cNonlinearCalibType);

		if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_DISABLE &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_WAVELENGTH &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_PIXEL &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_WAVELENGTH_II &&
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType != NONLINEAR_CALIB_PIXEL_II)
		{
			apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
		}

		if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_DISABLE)
		{
			return -2;
		}
	}
	else
	{
		return -3;
	}

	if(MIGP_RegArea_Read(apCommParaST[CommIndex], NVPA, 0x0041, 760, &pbTemp, 2000) == TRUE)
	{
		Sleep(100);
		PurgeComm(apCommParaST[CommIndex]->stComPara.ComHandle,  PURGE_RXCLEAR | PURGE_TXCLEAR);
		un32Temp.ab8Data[3] = (int)pbTemp[4];
		un32Temp.ab8Data[2] = (int)pbTemp[5];
		un32Temp.ab8Data[1] = (int)pbTemp[6];
		un32Temp.ab8Data[0] = (int)pbTemp[7];
		apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength = 1;//(int)un32Temp.f32Data;

		if(apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength < 1 ||
		   apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength > 8)
		{
			apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
			return SA_API_FAIL;
		}

		if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_WAVELENGTH ||
		   apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_PIXEL)
		{
			for(i = 0; i < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength; i++)
			{
				un32Temp.ab8Data[3] = (int)pbTemp[8 + (i * 88)];
				un32Temp.ab8Data[2] = (int)pbTemp[9 + (i * 88)];
				un32Temp.ab8Data[1] = (int)pbTemp[10 + (i * 88)];
				un32Temp.ab8Data[0] = (int)pbTemp[11 + (i * 88)];
				apCommParaST[CommIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[i] = un32Temp.f32Data;

				un32Temp.ab8Data[3] = (int)pbTemp[12 + (i * 88)];
				un32Temp.ab8Data[2] = (int)pbTemp[13 + (i * 88)];
				un32Temp.ab8Data[1] = (int)pbTemp[14 + (i * 88)];
				un32Temp.ab8Data[0] = (int)pbTemp[15 + (i * 88)];
				apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] = (int)un32Temp.f32Data;
				if(apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] < 1 ||
			   	   apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] > 10)
				{
					apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
					return SA_API_FAIL;
				}
				
				for(j = 0; j < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i]; j++)
				{
					un32Temp.ab8Data[3] = (int)pbTemp[16 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[2] = (int)pbTemp[17 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[1] = (int)pbTemp[18 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[0] = (int)pbTemp[19 + (i * 88) + (j * 8)];
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibAD[i][j] = un32Temp.f32Data;

					un32Temp.ab8Data[3] = (int)pbTemp[20 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[2] = (int)pbTemp[21 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[1] = (int)pbTemp[22 + (i * 88) + (j * 8)];
					un32Temp.ab8Data[0] = (int)pbTemp[23 + (i * 88) + (j * 8)];
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibCo[i][j] = un32Temp.f32Data;
				}
				
			}
		}
		else if(apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_WAVELENGTH_II ||
		        apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType == NONLINEAR_CALIB_PIXEL_II)
		{
			iLengthTemp = 8;
			for(i = 0; i < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibLength; i++)
			{
				un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
				un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
				un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
				un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
				iLengthTemp = iLengthTemp + 4;
				apCommParaST[CommIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[i] = un32Temp.f32Data;

				un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
				un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
				un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
				un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
				iLengthTemp = iLengthTemp + 4;
				apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i] = (int)un32Temp.f32Data;

				for(j = 0; j < apCommParaST[CommIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[i]; j++)
				{
					un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
					un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
					un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
					un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
					iLengthTemp = iLengthTemp + 4;
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibAD[i][j] = un32Temp.f32Data;

					un32Temp.ab8Data[3] = (int)pbTemp[0 + iLengthTemp];
					un32Temp.ab8Data[2] = (int)pbTemp[1 + iLengthTemp];
					un32Temp.ab8Data[1] = (int)pbTemp[2 + iLengthTemp];
					un32Temp.ab8Data[0] = (int)pbTemp[3 + iLengthTemp];
					iLengthTemp = iLengthTemp + 4;
					apCommParaST[CommIndex]->stSpectrometerPara.afNonlinearCalibCo[i][j] = un32Temp.f32Data;

					if(iLengthTemp > 760)
					{
						apCommParaST[CommIndex]->stSpectrometerPara.cNonlinearCalibType = NONLINEAR_CALIB_DISABLE;
						return SA_API_FAIL;
					}
				}
				
			}
		}
	}
	else
	{
		return SA_API_FAIL;
	}

	float *pfTempAD = &apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibAD[0][0];
	float *pfTempCo = &apCommParaST[spectrometerIndex]->stSpectrometerPara.afNonlinearCalibCo[0][0];
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	pfNonlinearCalibPixelOrWL = apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[0];
	piNonlinearCalibCoNumber  = apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[0];

	for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[0]; i++)
	{
		pfNonlinearCalibAD[i] = pfTempAD[i];
		pfNonlinearCalibCo[i] = pfTempCo[i];
	}
	
	return SA_API_SUCCESS;
}


#if INTER

int SA_OpenSpectrometersForInteral(void)
{
	//BYTE *pbTemp;
	//UN_TYPE unTemp;
	//UN_32TYPE un32Temp;
	COMM_PARA_ST *pstMigpPara = NULL;
	//HANDLE CommDev;
	int DevNum;
	int i = 0;
	int ret = 0;
	//int iTemp;
	
	g_iCommIndex = -1;
 
	DevNum = USBScanDev(1);

	if(DevNum == 0)
	{
		return g_iCommIndex;
	}

	for(i = 0; i < DevNum; i++)
	{
		ret = USBOpenDev(i);
		if(USBOpenDev(i) != SEVERITY_ERROR)
		{
			apCommParaST[g_iCommIndex + 1] = (COMM_PARA_ST *)malloc(sizeof(COMM_PARA_ST));
			if(apCommParaST[g_iCommIndex + 1] == NULL)
			{
				USBCloseDev(i);
				free(apCommParaST[g_iCommIndex + 1]);
			}
			else
			{
				apCommParaST[g_iCommIndex + 1]->enCommType = USB_WINLIB;
				apCommParaST[g_iCommIndex + 1]->stComPara.iUSBDevNum = i;
				apCommParaST[g_iCommIndex + 1]->stMIGP.bMasterAddress = 0xFE;
				apCommParaST[g_iCommIndex + 1]->stMIGP.bSlaveAddress = 0x00;
				g_iCommIndex++;
			}
		}

		
	}

	return g_iCommIndex;
}


int SA_OpenSpectrometersSimplifyForSerialForInteral(char * acCom, int Baudrate)
{
	COMM_PARA_ST *pstMigpPara = NULL;
	HANDLE CommDev;
	int iComTemp = 0;

	iComTemp = iComID - 1;
	if(Com_Open(CommDev, acCom, Baudrate) == TRUE)
	{
		apCommParaST[0] = (COMM_PARA_ST *)malloc(sizeof(COMM_PARA_ST));
		if(apCommParaST[0] == NULL)
		{
			Com_Close(CommDev);
			free(apCommParaST[0]);
		}
		else
		{
			apCommParaST[0]->enCommType = COM_SERIAL;
			apCommParaST[0]->stComPara.ComHandle = CommDev;
			apCommParaST[0]->stComPara.iBaudRate = 115200;
			strcpy(apCommParaST[0]->stComPara.acComName, acCom);
			apCommParaST[0]->stMIGP.bMasterAddress = 0xFE;
			apCommParaST[0]->stMIGP.bSlaveAddress = 0x00;
			g_iCommIndex++;
		}
	}

	return g_iCommIndex;
}


int SA_MIGPReadForInteral(int spectrometerIndex, AREA_TYPE enAreaType, 
	                   int ulAddress, int ulLength, 
	                   BYTE *pbRxData, int iTimeOut)
{
	BYTE *pbTemp;
	int i = 0;
	//HANDLE CommDev;

		
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], enAreaType, ulAddress, ulLength, &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;

	}

	
	for(i = 0; i < ulLength; i++)
	{
		pbRxData[i] = pbTemp[4 + i];
	}

	return SA_API_SUCCESS;
}



#endif

/****************************************************************************************/
/****************************************************************************************/
/******************************* 脉冲氙灯相关操作接口 ***********************************/
/****************************************************************************************/
/****************************************************************************************/


int SA_XenonFlashEnable(int spectrometerIndex)
{
	BYTE bTemp[1];
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	bTemp[0] = 1;

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0016, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{

		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;
}

int SA_XenonFlashDisable(int spectrometerIndex)
{
	BYTE bTemp[1];
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	bTemp[0] = 0;

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0016, 1, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{

		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;
}

int SA_SetXenonFlashPara(int spectrometerIndex, int iPulseWidth, int IntervalTime, int iDelayTime, int PulseNumber)
{
	BYTE bTemp[12];
	unsigned long ulTemp = 0;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	bTemp[0] = iPulseWidth >> 8 & 0x000000ff;
	bTemp[1] = iPulseWidth & 0x000000ff;

	bTemp[2] = IntervalTime >> 8 & 0x000000ff;
	bTemp[3] = IntervalTime & 0x000000ff;

	ulTemp = (unsigned long)(iDelayTime);
	bTemp[4] = (BYTE)(ulTemp >> 24 & 0x000000ff);
	bTemp[5] = (BYTE)(ulTemp >> 16 & 0x000000ff);
	bTemp[6] = (BYTE)(ulTemp >> 8 & 0x000000ff);
	bTemp[7] = (BYTE)(ulTemp & 0x000000ff);

	ulTemp = (unsigned long)(PulseNumber);
	bTemp[8] = (BYTE)(ulTemp >> 24 & 0x000000ff);
	bTemp[9] = (BYTE)(ulTemp >> 16 & 0x000000ff);
	bTemp[10] = (BYTE)(ulTemp >> 8 & 0x000000ff);
	bTemp[11] = (BYTE)(ulTemp & 0x000000ff);

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 0x0018, 12, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{

		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;
}

int SA_GetXenonFlashPara(int spectrometerIndex, int *iPulseWidth, int *IntervalTime, int *iDelayTime, int *PulseNumber)
{	
	BYTE *pbTemp;
	int i = 0;
	int iTimeOut = 500;
		
	if(spectrometerIndex > g_iCommIndex)
	{	
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], VPA, 0x0018, 12, &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;
	}

	*iPulseWidth = ((int)pbTemp[4] << 8) | (int)pbTemp[5];
	*IntervalTime = ((int)pbTemp[6] << 8) | (int)pbTemp[7];
	*iDelayTime =	((int)pbTemp[8] << 24) | ((int)pbTemp[9] << 16) | ((int)pbTemp[10] << 8)  | (int)pbTemp[11];
	*PulseNumber =	((int)pbTemp[12] << 24) | ((int)pbTemp[13] << 16) | ((int)pbTemp[14] << 8) | (int)pbTemp[15];

	return SA_API_SUCCESS;
}
/****************************************************************************************/
/****************************************************************************************/
/******************************* 用户存储操作接口 ***************************************/
/****************************************************************************************/
/****************************************************************************************/

int SA_ReadUserMemory(int spectrometerIndex, int Address, int length, BYTE * UserData)
{
	BYTE *pbTemp;
	int i = 0;

	if(Address + length > 4096)
	{
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], NVPA, 0x0400 + Address, length, &pbTemp, 1000) == FALSE)
	{
		return SA_API_FAIL;
	}

	for(i = 0; i < length; i++)
	{
		UserData[i] = pbTemp[i + 4];
	}

	return SA_API_SUCCESS;
}

int SA_WriteUserMemory(int spectrometerIndex, int Address, int length, BYTE * UserData)
{
	int i = 0;

	if(Address + length > 4096)
	{
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], NVPA, 0x0400 + Address, length, UserData, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;
}

int SA_SetWindow(int window)
{
	g_window = window;

	if(window < 0){
		g_window = 0;
	}

	if(window > 10){
		g_window = 10;
	}

	return g_window;
}

/* 用户存储操作接口 
typedef enum
{
	EIA,
	NVPA,
	VPA,
	MDA,
	SRA
}AREA_TYPE;
*/
int SA_ReadMemory(int spectrometerIndex, int MEM, int Address, int length, BYTE * UserData)
{
	BYTE *pbTemp;
	int i = 0;

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], (AREA_TYPE)MEM, Address, length, &pbTemp, 1000) == FALSE)
	{
		return SA_API_FAIL;
	}

	for(i = 0; i < length; i++)
	{
		UserData[i] = pbTemp[i + 4];
	}

	return SA_API_SUCCESS;
}

int SA_WriteMemory(int spectrometerIndex, int MEM, int Address, int length, BYTE * UserData)
{
	int i = 0;

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], (AREA_TYPE)MEM, Address, length, UserData, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;
}

