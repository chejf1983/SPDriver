
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


/* 光谱仪设备操作接口 */
typedef char* (*pSA_GetAPIVersion)(void);
typedef int (*pSA_OpenSpectrometers)(void);
typedef int (*pSA_OpenSpectrometersForSerial)(void);
typedef int (*pSA_OpenSpectrometersSimplifyForSerial)(int iComID);
typedef void (*pSA_CloseSpectrometers)(void);

/* 光谱仪参数设置操作接口 */
typedef int (*pSA_SetPosType)(int spectrometerIndex, int iPos);

typedef int (*pSA_SetSpectumTriggerMode)(int spectrometerIndex, TRIGGER_MODE TriggerMode);
typedef int (*pSA_SetIntegrationTime )(int spectrometerIndex, int usec);
typedef int (*pSA_SetAverageTimes)(int spectrometerIndex, int AverageTimes);

/* 波长定标操作接口 */
typedef int (*pSA_GetWavelengthCalibrationCoefficients )(int spectrometerIndex, double * WavelengthCalibration);
typedef int (*pSA_SetWavelengthCalibrationCoefficients )(int spectrometerIndex, double *WavelengthCalibration);
typedef int (*pSA_GetWavelength)(int spectrometerIndex, double *pdWavelengthData, int &pSpectumNumber);


/* 波长定标(新算法)操作接口 */
typedef int (*pSA_GetWavelengthCalibrationNewCoefficients )(int spectrometerIndex, float * pfWavelengthData, float * pfPixelData);
typedef int (*pSA_SetWavelengthCalibrationNewCoefficients )(int spectrometerIndex, float * pfWavelengthData, float * pfPixelData);


/* 非线性定标操作接口 */
typedef int (*pSA_NonlinearCalibration)(int spectrometerIndex, double * pbSpectum, double * pbNewSpectum, int SpectumNumber);
typedef int (*pSA_SetNonlinearCalibrationPixel)(int spectrometerIndex, float fPixelValue, int ParaNum, float *pfADValue, float *pfCalibrationCo);
typedef int (*pSA_GetNonlinearCalibrationPixel)(int spectrometerIndex, float &pfNonlinearCalibPixelOrWL, int &piNonlinearCalibCoNumber,float pfNonlinearCalibAD[], float pfNonlinearCalibCo[]);


/* 光谱仪信息获取操作接口 */
typedef char * (*pSA_GetSpectrometersName)(int spectrometerIndex);
typedef int (*pSA_GetMaxIntegrationTime )(int spectrometerIndex);
typedef int (*pSA_GetMinIntegrationTime )(int spectrometerIndex);
typedef char * (*pSA_GetSoftwareVersion)(int spectrometerIndex);
typedef char (*pSA_GetHardwareVersion)(int spectrometerIndex);
typedef char * (*pSA_GetSerialNumber)(int spectrometerIndex);
typedef int (*pSA_GetSpectrometerPixelsNumber )(int spectrometerIndex);
typedef char * (*pSA_GetManufacturingDate)(int spectrometerIndex);

/* 光谱数据处理相关接口 */
typedef int (*pSA_OpenModSpec)(BOOL bOpen,int iNum, float fValue);
typedef int (*pSA_UseFFTFilter)(BOOL bUseFFT,float fValue);

/*********************************软件单通道触发相关操作***********************************************/

/* 单通道软件同步操作--获取光谱 */
typedef int (*pSA_GetSpectum)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);

/* 单通道软件异步操作--异步触发、状态查询、获取光谱*/
typedef int (*pSA_ScanStartAsyncSoftTrigger)(int spectrometerIndex);
typedef int (*pSA_GetStateAsyncSoftTrigger)(int spectrometerIndex, int *pState);
typedef int (*pSA_GetAsyncSoftSpectum)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);


/*********************************软件多通道触发相关操作***********************************************/
/* 设置多通道积分时间 */
typedef int (*pSA_SetMultiChannelIntegrationTime )(int spectrometerIndex, int *usec);

/* 多通道软件同步操作--获取光谱 */
typedef int (*pSA_GetMultiChannelSpectum)(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int iChannelNum);

/* 多通道软件异步操作--异步触发、状态查询、获取光谱*/
typedef int (*pSA_ScanStartMultiChannelAsyncSoftTrigger)(int spectrometerIndex, int iChannelNum);
typedef int (*pSA_GetStateMultiChannelAsyncSoftTrigger)(int spectrometerIndex, int *pState);
typedef int (*pSA_GetMultiChannelAsyncSoftSpectum)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);


/* 多通道存储异步触发相关操作--异步触发、状态查询、光谱获取 */
typedef int (*pSA_ScanStartCacheAsyncTrigger)(int spectrometerIndex, int iCacheChannelNum);
typedef int (*pSA_GetStateCacheAsyncTrigger)(int spectrometerIndex, int *pState);
typedef int (*pSA_GetCacheAsyncSpectum)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iCacheChannelNum);


/*********************************硬件触发相关操作***********************************************/

/* 高电平硬件触发操作接口 */
typedef int (*pSA_GetSpectumHLTrigger)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iTimeOut);

/* 低电平硬件触发操作接口 */
typedef int (*pSA_GetSpectumLLTrigger)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iTimeOut);


/* 上升沿触发启动操作接口 */
typedef int (*pSA_StartRETrigger)(int spectrometerIndex);
typedef int (*pSA_GetSpectumRETrigger)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);

/*  下降沿触发启动操作接口 */
typedef int (*pSA_StartFETrigger)(int spectrometerIndex);
typedef int (*pSA_GetSpectumFETrigger)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber);






/* RGB上升沿触发启动操作接口 */
typedef int (*pSA_SetRGBModeIntegrationTime )(int spectrometerIndex, int *usec, int usec_num);
typedef int (*pSA_StartRBGModeRETrigger)(int spectrometerIndex);
typedef int (*pSA_GetSpectumRBGModeRETrigger)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iChannelNum);

/*  RGB下降沿触发启动操作接口 */
typedef int (*pSA_StartRBGModeFETrigger)(int spectrometerIndex);
typedef int (*pSA_GetSpectumRBGModeFETrigger)(int spectrometerIndex, double *pdSpectumData, int &pSpectumNumber, int iChannelNum);






/* 自动积分时间相关操作接口 */
typedef int (*pSA_GetSpectumAutoIntegrationTime)(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber, int usec);



/* 脉冲氙灯相关操作接口 */
typedef int (*pSA_SetXenonFlashPara)(int spectrometerIndex, int iPulseWidth, int IntervalTime, int iDelayTime, int PulseNumber);
typedef int (*pSA_GetXenonFlashPara)(int spectrometerIndex, int *iPulseWidth, int *IntervalTime, int *iDelayTime, int *PulseNumber);
typedef int (*pSA_XenonFlashDisable)(int spectrometerIndex);
typedef int (*pSA_XenonFlashEnable)(int spectrometerIndex);



/* 用户存储操作接口 */
typedef int (*pSA_WriteUserMemory)(int spectrometerIndex, int Address, int length, BYTE * UserData);
typedef int (*pSA_ReadUserMemory)(int spectrometerIndex, int Address, int length, BYTE * UserData);

