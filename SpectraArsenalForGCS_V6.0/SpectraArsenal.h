/*******************************************************************************************************
**
** Copyright (C), 2011-2012, Nahon photoelectric technology CO., LTD.
** File Name:               SpectraArsenal.h
** Description:             
** Created By:             Monster
**
*********************************************************************************************************/

#ifdef SPECTRAARSENAL_EXPORTS
#define SPECTRAARSENAL_API extern "C" __declspec(dllexport)
#else
#define SPECTRAARSENAL_API extern "C" __declspec(dllimport)
#endif

#define SA_API_SUCCESS   0
#define SA_API_FAIL      -1



#define WORK_MODE_SOFT_SYNC_TRIGGER     0x00
#define WORK_MODE_AUTO_TRIGGER          0x01
#define WORK_MODE_SOFT_ASYN_TRIGGER     0x02

#define WORK_MODE_RISE_TRIGGER          0x04
#define WORK_MODE_FALL_TRIGGER          0x05

#define WORK_MODE_HIGH_LEVEL_TRIGGER    0x08
#define WORK_MODE_LOWER_LEVEL_TRIGGER   0x09


typedef enum
{
	SOFTWARE_SYNCHRONOUS,
	SOFTWARE_ASYNCHRONOUS,
	SOFTWARE_AUTO,
	EXINT_RISING_EDGE,
	EXINT_FALLING_EDGE,
	EXINT_HIGH_LEVEL,
	EXINT_LOW_LEVEL,
	CACHE_ASYNCHRONOUS,
	EXINT_RGB_RISING_EDGE,
	EXINT_RGB_FALLING_EDGE,
}TRIGGER_MODE;

typedef enum
{
	AUTO_INTEGRATION_DISABLE,
	AUTO_INTEGRATION_ENABLE,
}AUTO_INTEGRATION_MODE;

#if 0
typedef struct
{
	int pixelNumber;
	double Spectum[2048];
}Spectum_Data;
#endif

/* �������豸�����ӿ� */
SPECTRAARSENAL_API char* SA_GetAPIVersion(void);
SPECTRAARSENAL_API int SA_OpenSpectrometers(void);
SPECTRAARSENAL_API int SA_OpenSpectrometersForSerial(void);
SPECTRAARSENAL_API int SA_OpenSpectrometersSimplifyForSerial(int iComID);
SPECTRAARSENAL_API void SA_CloseSpectrometers(void);

/* �����ǲ������ò����ӿ� */
SPECTRAARSENAL_API int SA_SetPosType(int spectrometerIndex, int iPos);

SPECTRAARSENAL_API int SA_SetSpectumTriggerMode(int spectrometerIndex, TRIGGER_MODE TriggerMode);
SPECTRAARSENAL_API int SA_SetIntegrationTime (int spectrometerIndex, int usec);
SPECTRAARSENAL_API int SA_SetAverageTimes(int spectrometerIndex, int AverageTimes);

/* ������������ӿ� */
SPECTRAARSENAL_API int SA_GetWavelengthCalibrationCoefficients (int spectrometerIndex, double * WavelengthCalibration);
SPECTRAARSENAL_API int SA_SetWavelengthCalibrationCoefficients (int spectrometerIndex, double *WavelengthCalibration);
SPECTRAARSENAL_API int SA_GetWavelength(int spectrometerIndex, double *pdWavelengthData, int &pSpectumNumber);


/* ��������(���㷨)�����ӿ� */
SPECTRAARSENAL_API int SA_GetWavelengthCalibrationNewCoefficients (int spectrometerIndex, float * pfWavelengthData, float * pfPixelData);
SPECTRAARSENAL_API int SA_SetWavelengthCalibrationNewCoefficients (int spectrometerIndex, float * pfWavelengthData, float * pfPixelData);


/* �����Զ�������ӿ� */
SPECTRAARSENAL_API int SA_NonlinearCalibration(int spectrometerIndex, double * pbSpectum, double * pbNewSpectum, int SpectumNumber);
SPECTRAARSENAL_API int SA_SetNonlinearCalibrationPixel(int spectrometerIndex, float fPixelValue, int ParaNum, float *pfADValue, float *pfCalibrationCo);
SPECTRAARSENAL_API int SA_GetNonlinearCalibrationPixel(int spectrometerIndex, float &pfNonlinearCalibPixelOrWL, int &piNonlinearCalibCoNumber,float pfNonlinearCalibAD[], float pfNonlinearCalibCo[]);


/* ��������Ϣ��ȡ�����ӿ� */
SPECTRAARSENAL_API char * SA_GetSpectrometersName(int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetMaxIntegrationTime (int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetMinIntegrationTime (int spectrometerIndex);
SPECTRAARSENAL_API char * SA_GetSoftwareVersion(int spectrometerIndex);
SPECTRAARSENAL_API char SA_GetHardwareVersion(int spectrometerIndex);
SPECTRAARSENAL_API char * SA_GetSerialNumber(int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetSpectrometerPixelsNumber (int spectrometerIndex);
SPECTRAARSENAL_API char * SA_GetManufacturingDate(int spectrometerIndex);

/* �������ݴ�����ؽӿ� */
SPECTRAARSENAL_API int SA_OpenModSpec(BOOL bOpen,int iNum, float fValue);
SPECTRAARSENAL_API int SA_UseFFTFilter(BOOL bUseFFT,float fValue);
SPECTRAARSENAL_API int SA_SetWindow(int window);

/*********************************�����ͨ��������ز���***********************************************/

/* ��ͨ�����ͬ������--��ȡ���� */
SPECTRAARSENAL_API int SA_GetSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);
SPECTRAARSENAL_API int SA_GetSpectumN(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);

/* ��ͨ������첽����--�첽������״̬��ѯ����ȡ����*/
SPECTRAARSENAL_API int SA_ScanStartAsyncSoftTrigger(int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetStateAsyncSoftTrigger(int spectrometerIndex, int *pState);
SPECTRAARSENAL_API int SA_GetAsyncSoftSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);


/*********************************�����ͨ��������ز���***********************************************/
/* ���ö�ͨ������ʱ�� */
SPECTRAARSENAL_API int SA_SetMultiChannelIntegrationTime (int spectrometerIndex, int *usec);

/* ��ͨ�����ͬ������--��ȡ���� */
SPECTRAARSENAL_API int SA_GetMultiChannelSpectum(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int iChannelNum);

/* ��ͨ������첽����--�첽������״̬��ѯ����ȡ����*/
SPECTRAARSENAL_API int SA_ScanStartMultiChannelAsyncSoftTrigger(int spectrometerIndex, int iChannelNum);
SPECTRAARSENAL_API int SA_GetStateMultiChannelAsyncSoftTrigger(int spectrometerIndex, int *pState);
SPECTRAARSENAL_API int SA_GetMultiChannelAsyncSoftSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);


/* ��ͨ���洢�첽������ز���--�첽������״̬��ѯ�����׻�ȡ */
SPECTRAARSENAL_API int SA_ScanStartCacheAsyncTrigger(int spectrometerIndex, int iCacheChannelNum);
SPECTRAARSENAL_API int SA_GetStateCacheAsyncTrigger(int spectrometerIndex, int *pState);
SPECTRAARSENAL_API int SA_GetCacheAsyncSpectum(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iCacheChannelNum);


/*********************************Ӳ��������ز���***********************************************/

/* �ߵ�ƽӲ�����������ӿ� */
SPECTRAARSENAL_API int SA_GetSpectumHLTrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iTimeOut);

/* �͵�ƽӲ�����������ӿ� */
SPECTRAARSENAL_API int SA_GetSpectumLLTrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iTimeOut);


/* �����ش������������ӿ� */
SPECTRAARSENAL_API int SA_StartRETrigger(int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetSpectumRETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);

/*  �½��ش������������ӿ� */
SPECTRAARSENAL_API int SA_StartFETrigger(int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetSpectumFETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);


/* RGB�����ش������������ӿ� */
SPECTRAARSENAL_API int SA_SetRGBModeIntegrationTime (int spectrometerIndex, int *usec, int usec_num);
SPECTRAARSENAL_API int SA_StartRBGModeRETrigger(int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetSpectumRBGModeRETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iChannelNum);

/*  RGB�½��ش������������ӿ� */
SPECTRAARSENAL_API int SA_StartRBGModeFETrigger(int spectrometerIndex);
SPECTRAARSENAL_API int SA_GetSpectumRBGModeFETrigger(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iChannelNum);

/* �Զ�����ʱ����ز����ӿ� */
SPECTRAARSENAL_API int SA_GetSpectumAutoIntegrationTime(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int usec);

/* ����믵���ز����ӿ� */
SPECTRAARSENAL_API int SA_SetXenonFlashPara(int spectrometerIndex, int iPulseWidth, int IntervalTime, int iDelayTime, int PulseNumber);
SPECTRAARSENAL_API int SA_GetXenonFlashPara(int spectrometerIndex, int *iPulseWidth, int *IntervalTime, int *iDelayTime, int *PulseNumber);
SPECTRAARSENAL_API int SA_XenonFlashDisable(int spectrometerIndex);
SPECTRAARSENAL_API int SA_XenonFlashEnable(int spectrometerIndex);

/* �û��洢�����ӿ� */
SPECTRAARSENAL_API int SA_WriteUserMemory(int spectrometerIndex, int Address, int length, BYTE * UserData);
SPECTRAARSENAL_API int SA_ReadUserMemory(int spectrometerIndex, int Address, int length, BYTE * UserData);


