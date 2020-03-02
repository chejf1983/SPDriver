using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace CSGCSDll.DLL.NqDll
{
    /// <summary>
    /// OEM3接口
    /// </summary>
    public static class Nahon_DLL
    {

        /* 
         #ifdef SPECTRAARSENAL_EXPORTS
         #define SPECTRAARSENAL_API __declspec(dllexport)
         #else
         #define SPECTRAARSENAL_API __declspec(dllimport)
         #endif
         */
        public static readonly int SA_API_SUCCESS = 0;
        public static readonly int SA_API_FAIL = -1;

        public enum TRIGGER_MODE
        {
            SOFTWARE_SYNCHRONOUS,
            SOFTWARE_ASYNCHRONOUS,
            SOFTWARE_AUTO,
            EXINT_RISING_EDGE,
            EXINT_FALLING_EDGE,
            EXINT_HIGH_LEVEL,
            EXINT_LOW_LEVEL,
        };

        public enum AUTO_INTEGRATION_MODE
        {
            AUTO_INTEGRATION_DISABLE,
            AUTO_INTEGRATION_ENABLE,
        };

        /* 
         *   BSTR ---------  StringBuilder

             LPCTSTR --------- StringBuilder

             LPCWSTR ---------  IntPtr

             handle---------IntPtr

             hwnd-----------IntPtr

             char *----------string

             int * -----------ref int

             int &-----------ref int

             void *----------IntPtr

             unsigned char *-----ref byte
         */

        #region 光谱仪设备接口操作
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetAPIVersion", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SA_GetAPIVersion();
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_OpenSpectrometers", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_OpenSpectrometers();
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_CloseSpectrometers", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SA_CloseSpectrometers();
        #endregion

        #region 光谱仪参数设置操作
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_SetIntegrationTime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_SetIntegrationTime(int spectrometerIndex, int usec);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetIntegrationTime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetIntegrationTime(int spectrometerIndex, ref int usec);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_SetAverageTimes", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_SetAverageTimes(int spectrometerIndex, int AverageTimes);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetAverageTimes", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetAverageTimes(int spectrometerIndex, ref int AverageTimes);
        #endregion

        #region 光谱仪信息获取接口
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectrometersName", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SA_GetSpectrometersName(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetMaxIntegrationTime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetMaxIntegrationTime(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetMinIntegrationTime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetMinIntegrationTime(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSoftwareVersion", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SA_GetSoftwareVersion(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetHardwareVersion", CallingConvention = CallingConvention.Cdecl)]
        public static extern char SA_GetHardwareVersion(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSerialNumber", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SA_GetSerialNumber(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectrometerPixelsNumber", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetSpectrometerPixelsNumber(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetManufacturingDate", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SA_GetManufacturingDate(int spectrometerIndex);
        #endregion

        #region 波长定标操作接口
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_NonlinearCalibration", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_NonlinearCalibration(int spectrometerIndex, double[] pbSpectum, double[] pbNewSpectum, int SpectumNumber);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetWavelengthCalibrationCoefficients", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_ReadUserMemory(int spectrometerIndex, int Address, int length, byte[] UserData);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetWavelength", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetWavelength(int spectrometerIndex, double[] pdWavelengthData, ref int pSpectumNumber);
        #endregion
        
        #region 光谱读写操作
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectum", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetSpectum(int spectrometerIndex, double[] pdSpectumData, ref int pSpectumNumber);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectumHWTrigger", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetSpectumHWTrigger(int spectrometerIndex, double[] pdSpectumData, ref int pSpectumNumber, int iTimeOut, TRIGGER_MODE TriggerMode);


        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_ScanStartAsyncSoftTrigger", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_ScanStartAsyncSoftTrigger(int spectrometerIndex);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectumAsyncSoftTrigger", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetSpectumAsyncSoftTrigger(int spectrometerIndex, ref int pState, double[] pdSpectumData, int pSpectumNumber);

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_SetMultiChannelIntegrationTime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_SetMultiChannelIntegrationTime(int spectrometerIndex, int[] usec);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetMultiChannelSpectum", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetMultiChannelSpectum(int spectrometerIndex, double[] pdSpectumData, ref int pSpectumNumber, int iChannelNum);
        #endregion

        #region 内存地址读写
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_ReadUserMemory", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetWavelengthCalibrationCoefficients(int spectrometerIndex, double[] WavelengthCalibration);
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_WriteUserMemory", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_WriteUserMemory(int spectrometerIndex, int Address, int length, byte[] UserData);
        #endregion

    }
}
