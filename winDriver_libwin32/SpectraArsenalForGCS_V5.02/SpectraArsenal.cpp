/*******************************************************************************************************
**
** Copyright (C), 2011-2012, Nahon photoelectric technology CO., LTD.
** File Name:               SpectraArsenal.cpp
** Description:             
** Created By:             Monster
**
*********************************************************************************************************/
#include <Windows.h>
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

const char acAPIVersion[] = "V5.03";
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
		printf_debug("SA_SearchSpectrometers:: Read EIA Error\r\n");
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
		
		//读取最像素点个数
		iTemp = (int)pbTemp[4 + NVPA_PIXELS_NUMBER_ADDR] << 8;
		iTemp |= (int)pbTemp[4 + NVPA_PIXELS_NUMBER_ADDR + 1];
		apCommParaST[CommIndex]->stSpectrometerPara.iPixelNumber = iTemp;

		iTemp = (int)pbTemp[4 + NVPA_PIXELS_TYPE_ADDR];
		apCommParaST[CommIndex]->stSpectrometerPara.iPixelType = iTemp;

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
 
	DevNum = USBScanDev(1);

	if(DevNum == 0)
	{
		printf_debug("SA_OpenSpectrometers:: NO USB Device");
		return g_iCommIndex;
	}

	for(i = 0; i < DevNum; i++)
	{
		ret = USBOpenDev(i);
		if(USBOpenDev(i) != SEVERITY_ERROR)
		{
			printf_debug("OPEN USB device SUCCESS！\n");
			apCommParaST[g_iCommIndex + 1] = (COMM_PARA_ST *)malloc(sizeof(COMM_PARA_ST));
			if(apCommParaST[g_iCommIndex + 1] == NULL)
			{
				printf_debug("SA_OpenSpectrometers:: malloc NULL");
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
				    printf_debug("SA_OpenSpectrometers:: MIGP_RegArea_Read FAIL");
					USBCloseDev(i);
					free(apCommParaST[g_iCommIndex + 1]);
				}
			}
		}		
	}

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
				strcpy(apCommParaST[g_iCommIndex + 1]->stComPara.acComName, acComName[i]);
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
			strcpy(apCommParaST[0]->stComPara.acComName, acComName[iComTemp]);
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
int SA_SetMultiChannelIntegrationTime0 (int spectrometerIndex, int *usec, int usec_num)
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
			return SA_API_FAIL;
		}
	}

	for(i = 0; i < CHL_NUM; i++)
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


	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 112, 32, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, 144, 32, &bTemp[32], MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	//apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime = usec;
	
	return SA_API_SUCCESS;
}

int SA_SetMultiChannelIntegrationTime (int spectrometerIndex, int *usec)
{
	return SA_SetMultiChannelIntegrationTime0(spectrometerIndex, usec, 16);
}



int SA_SetIntegrationTime (int spectrometerIndex, int usec)
{
	BYTE bTemp[4];
	unsigned long ulTemp = 0;
	int i = 0;
	//HANDLE CommDev;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_SetIntegrationTime FAIL");
		return SA_API_FAIL;
	}

	if(usec < apCommParaST[spectrometerIndex]->stSpectrometerPara.iMinIntegrationTimeUS ||
	   usec > apCommParaST[spectrometerIndex]->stSpectrometerPara.iMaxIntegrationTimeUS)
	{
//		printf_debug("SA_SetIntegrationTime FAIL");
		return SA_API_FAIL;
	}

	//if(usec == apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime)
	//{
	//	return SA_API_SUCCESS;
	//}

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
	BYTE bTemp[2];
	int i = 0;
	//HANDLE CommDev;
	
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_SetAverageTimes FAIL");
		return SA_API_FAIL;
	}

	if(AverageTimes < 1 || AverageTimes > 1000)
	{	
//		printf_debug("SA_SetAverageTimes FAIL");
		return SA_API_FAIL;
	}

	if(AverageTimes == apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes)
	{
		return SA_API_SUCCESS;
	}

	bTemp[0] = AverageTimes >> 8 & 0x000000ff;
	bTemp[1] = AverageTimes & 0x000000ff;


	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], VPA, VPA_AVERAGE_TIME_ADDR, 2, bTemp, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}
	
	apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes = AverageTimes;

	return SA_API_SUCCESS;
}

 
 int SA_SetSpectumTriggerMode(int spectrometerIndex, TRIGGER_MODE TriggerMode)
 {
	 BYTE bTemp[1];
	 int i = 0;
 
	 //double dADTemp[2048];
	 //double dAD[2048];
		 
	 if(spectrometerIndex > g_iCommIndex)
	 {	 
 // 	 printf_debug("SA_GetSpectumForHWTrigger FAIL");
		 return SA_API_FAIL;
	 }
	 
	 switch(TriggerMode)
	 {
	 	 case SOFTWARE_SYNCHRONOUS:
		 	 bTemp[0] = 0x00;
			 apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = SOFTWARE_SYNCHRONOUS;
			 break;
		case SOFTWARE_ASYNCHRONOUS:
		 	 bTemp[0] = 0x02;
			 apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = SOFTWARE_ASYNCHRONOUS;
			 break;
		case CACHE_ASYNCHRONOUS:
		 	 bTemp[0] = 0x03;
			 apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode = CACHE_ASYNCHRONOUS;
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
		 default:
			 return SA_API_FAIL;
	 }
 
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


int SA_GetWavelength(int spectrometerIndex, double *pdWavelengthData, int *pSpectumNumber)
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
	
	*pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
	{
		pdWavelengthData[i] = dCalib[0] + (dCalib[1] * i) + (dCalib[2] * i * i) + (dCalib[3] * i * i * i);
	}
	
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


int SA_GetCacheAsyncSpectum(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int iCacheChannelNum)
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

	*pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 0x01)
	{	
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2) + 1] * 256) + pbTemp[4 + (i * 2)]);
		}
	}else{
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
		}	
	}
//	for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
//	{
//		pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
//	}



	
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
	
	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], VPA, VPA_ASYN_SCAN_ADDR, 1, &pbTemp, 50) == FALSE)
	{
		return SA_API_FAIL;
	}

	*pState = pbTemp[4];
		
	return SA_API_SUCCESS;

}

#if 0
void SA_AverageProcess(double* datavalue, int datalen, int IntaverageWidth)
{
	int i = 0;
	double tmp[10000];
	int win_left = 0;
	int win_num = 0;
	int win_index = 0;
	int index = 0;

	//计算窗口启使偏移
	win_left = 0 - IntaverageWidth / 2;

	//循环每个值
	for (i = 0; i < datalen; i++) 
	{
		//初值赋0
		tmp[i] = 0;

		//实际窗内长度
		win_num = 0;
		//循环一个窗口的长度
		for (win_index = 0; win_index < IntaverageWidth; win_index++) 
		{
			//计算窗口对应原始数据的坐标
			index = i + win_left + win_index;

			//如果窗口对应原始数据的坐标，在原始数组范围内，累加
			if (index >= 0 && index < datalen) 
			{
				tmp[i] += datavalue[index];
				win_num++;
			}
		}
		//取平均
		tmp[i] /= win_num;
	}

	//循环每个值,赋值回原始数据
	for (i = 0; i < datalen; i++) 
	{
		datavalue[i] = tmp[i];
	}


}
#endif



/************************************************************/
/************************************************************/
/*************** 光谱仪获取光谱操作接口 *********************/
/************************************************************/
/************************************************************/

int SA_GetSpectum(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber)
{
	BYTE *pbTemp;
	int i = 0;
	int iTimeOut;
	//HANDLE CommDev;
	double pdWavelengthData[2048];
	int pTempSpectumNumber = 0;
	SA_GetWavelength(spectrometerIndex, pdWavelengthData, &pTempSpectumNumber);
			
		
	if(spectrometerIndex > g_iCommIndex)
	{	
		printf_debug("SA_GetSpectum FAIL");
		return SA_API_FAIL;
	}
		
	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_SYNCHRONOUS && 
	   apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_AUTO
	    && 
	   apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_ASYNCHRONOUS)
	{
		printf_debug("SA_GetSpectum FAIL");
		return SA_API_FAIL;//SA_API_FAIL;
	}
		
	iTimeOut = (int)(apCommParaST[spectrometerIndex]->stSpectrometerPara.iIntegrationTime / 1000) + 5;

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes <= 0)
	{
		apCommParaST[spectrometerIndex]->stSpectrometerPara.iAverageTimes = 1;
	}
	
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


	*pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;	

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 0x01)
	{	
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2) + 1] * 256) + pbTemp[4 + (i * 2)]);
		}
	}else{
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
		}	
	}
 	
	return SA_API_SUCCESS;
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
		printf_debug("SA_GetSpectum FAIL");
		return SA_API_FAIL;
	}

	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_SYNCHRONOUS && 
	   apCommParaST[spectrometerIndex]->stSpectrometerPara.cTriggerMode != SOFTWARE_AUTO)
	{
		printf_debug("SA_GetSpectum FAIL");
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
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2) + 1] * 256) + pbTemp[4 + (i * 2)]);
		}
	}else{
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
		}	
	}
	
	//for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
	//{
//		pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
//	}


	
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
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2) + 1] * 256) + pbTemp[4 + (i * 2)]);
		}
	}else{
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



int SA_GetSpectumHWTrigger(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int iTimeOut, TRIGGER_MODE TriggerMode)
{
	BYTE *pbTemp;
	int i = 0;

	//double dADTemp[2048];
	//double dAD[2048];
		
	if(spectrometerIndex > g_iCommIndex)
	{	
//		printf_debug("SA_GetSpectumForHWTrigger FAIL");
		return SA_API_FAIL;
	}

	if(MIGP_RegArea_Read(apCommParaST[spectrometerIndex], MDA, 0x0000, (apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber * 2), &pbTemp, iTimeOut) == FALSE)
	{
		return SA_API_FAIL;
	}

#if 1
	*pSpectumNumber = apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
	if(apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelType == 0x01)
	{	
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2) + 1] * 256) + pbTemp[4 + (i * 2)]);
		}
	}else{
		for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
		{
			pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
		}	
	}
	//for(i = 0; i < apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber; i++)
	//{
///		pdSpectumData[i] = (double)(((double)pbTemp[4 + (i * 2)] * 256) + pbTemp[4 + (i * 2) + 1]);
//	}

#endif

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
	return apCommParaST[spectrometerIndex]->stSpectrometerPara.iPixelNumber;
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
}

int SA_GetNonlinearCalibrationPixel(int spectrometerIndex, 
										 float *pfNonlinearCalibPixelOrWL, int *piNonlinearCalibCoNumber,
	                                     float *pfNonlinearCalibAD, float *pfNonlinearCalibCo)
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

	*pfNonlinearCalibPixelOrWL = apCommParaST[spectrometerIndex]->stSpectrometerPara.fNonlinearCalibPixelOrWL[0];
    *piNonlinearCalibCoNumber  = apCommParaST[spectrometerIndex]->stSpectrometerPara.iNonlinearCalibCoNumber[0];

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
		printf_debug("SA_OpenSpectrometers:: NO USB Device");
		return g_iCommIndex;
	}

	for(i = 0; i < DevNum; i++)
	{
		ret = USBOpenDev(i);
		if(USBOpenDev(i) != SEVERITY_ERROR)
		{
			printf_debug("OPEN USB device SUCCESS！\n");
			apCommParaST[g_iCommIndex + 1] = (COMM_PARA_ST *)malloc(sizeof(COMM_PARA_ST));
			if(apCommParaST[g_iCommIndex + 1] == NULL)
			{
				printf_debug("SA_OpenSpectrometers:: malloc NULL");
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
	
	//g_iCommIndex = -1;

	iComTemp = iComID - 1;
	if(Com_Open(CommDev, acCom, Baudrate) == TRUE)
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
		printf_debug("SA_GetSpectum FAIL");
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
		printf_debug("SA_GetSpectum FAIL");
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
SPECTRAARSENAL_API int SA_ReadMemory(int spectrometerIndex, int MEM, int Address, int length, BYTE * UserData)
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

SPECTRAARSENAL_API int SA_WriteMemory(int spectrometerIndex, int MEM, int Address, int length, BYTE * UserData)
{
	int i = 0;

	if(MIGP_RegArea_Write(apCommParaST[spectrometerIndex], (AREA_TYPE)MEM, Address, length, UserData, MIGP_ACK_RX_ENABLE) == FALSE)
	{
		return SA_API_FAIL;
	}

	return SA_API_SUCCESS;
}

