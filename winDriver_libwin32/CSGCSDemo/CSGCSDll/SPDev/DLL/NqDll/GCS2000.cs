using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Custom.Devices
{
    public class GCS2000
    {
        #region API
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_OpenSpectrometers", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_OpenSpectrometers();
        /// <summary>
        /// 打开光谱
        /// </summary>        
        public int NET_OpenSpectrometers()
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_OpenSpectrometers();
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_CloseSpectrometers", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SA_CloseSpectrometers();
        /// <summary>
        /// 关闭光谱
        /// </summary>        
        public void NET_CloseSpectrometers()
        {
            #region

            try
            {
                SA_CloseSpectrometers();
            }
            catch (Exception ex)
            {
                throw ex;
            }

            #endregion
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetWavelength", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetWavelength(int spectrometerIndex, double[] pdWavelengthData, out int pSpectumNumber);
        /// <summary>
        /// 获取波长数据
        /// </summary>        
        public int NET_GetWavelength(int spectrometerIndex, double[] pdWavelengthData, out int pSpectumNumber)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_GetWavelength(spectrometerIndex, pdWavelengthData, out pSpectumNumber);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_SetIntegrationTime", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_SetIntegrationTime(int spectrometerIndex, int usec);
        /// <summary>
        /// 设置积分时间
        /// </summary>        
        public int NET_SetIntegrationTime(int spectrometerIndex, int usec)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_SetIntegrationTime(spectrometerIndex, usec);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_SetAverageTimes", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_SetAverageTimes(int spectrometerIndex, int AverageTimes);
        /// <summary>
        /// 设置平均次数
        /// </summary>        
        public int NET_SetAverageTimes(int spectrometerIndex, int avg)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_SetAverageTimes(spectrometerIndex, avg);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectum", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetSpectum(int spectrometerIndex, double[] pdSpectumData, out int pSpectumNumber);
        /// <summary>
        /// 获取光谱数据
        /// </summary>        
        public int NET_GetSpectum(int spectrometerIndex, double[] pdSpectumData, out int pSpectumNumber)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_GetSpectum(spectrometerIndex, pdSpectumData, out pSpectumNumber);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_NonlinearCalibration", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_NonlinearCalibration(int spectrometerIndex, double[] pbSpectum, double[] pbNewSpectum, int SpectumNumber);
        /// <summary>
        /// 获取光谱非线性数据
        /// </summary>
        /// <param name="spectrometerIndex">光谱编码</param>
        /// <param name="pbSpectum">光谱原始数据</param>
        /// <param name="pbNewSpectum">光谱非线性后数据</param>
        /// <param name="SpectumNumber">数据长度</param>
        /// <returns></returns>        
        public int NET_NonlinearCalibration(int spectrometerIndex, double[] pbSpectum, double[] pbNewSpectum, int SpectumNumber)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_NonlinearCalibration(spectrometerIndex, pbSpectum, pbNewSpectum, SpectumNumber);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        //[DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectrometersName", CallingConvention = CallingConvention.Cdecl)]
        //public static extern string SA_GetSpectrometersName(int spectrometerIndex);
        ///// <summary>
        ///// 获取光谱名称
        ///// </summary>        
        //public string NET_GetSpectrometersName(int spectrometerIndex)
        //{
        //    #region
        //    try
        //    {
        //        string name = "";
        //        name = SA_GetSpectrometersName(spectrometerIndex);
        //        return name;
        //    }
        //    catch (Exception ex)
        //    {
        //        throw ex;
        //    }
        //    #endregion
        //}

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectrometersName", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr SA_GetSpectrometersName(int spectrometerIndex);
        public String NET_GetSpectrometersName(int index)
        {
            try
            {
                string devsn = Marshal.PtrToStringAnsi(SA_GetSpectrometersName(index));

                return devsn;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSerialNumber", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr SA_GetSerialNumber(int spectrometerIndex);
        public String NET_GetSerialNumber(int index)
        {
            try
            {
                string devsn = Marshal.PtrToStringAnsi(SA_GetSerialNumber(index));

                return devsn;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetHardwareVersion", CallingConvention = CallingConvention.Cdecl)]
        public static extern string SA_GetHardwareVersion(int spectrometerIndex);
        /// <summary>
        /// 获取光谱序列号
        /// </summary>        
        public string NET_GetHardwareVersion(int spectrometerIndex)
        {
            #region
            try
            {
                return SA_GetHardwareVersion(spectrometerIndex);
            }
            catch (Exception ex)
            {
                throw ex;
            }
            #endregion
        }

        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetSpectrometerPixelsNumber", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetSpectrometerPixelsNumber(int spectrometerIndex);
        /// <summary>
        /// 获取像素长度
        /// </summary>        
        public int NET_GetSpectrometerPixelsNumber(int spectrometerIndex)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_GetSpectrometerPixelsNumber(spectrometerIndex);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        /// <summary>
        /// 软触发
        /// </summary>
        /// <param name="spectrometerIndex"></param>
        /// <returns></returns>
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_ScanStartAsyncSoftTrigger", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_ScanStartAsyncSoftTrigger(int spectrometerIndex);

        public int NET_ScanStartAsyncSoftTrigger(int spectrometerIndex)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_ScanStartAsyncSoftTrigger(spectrometerIndex);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        /// <summary>
        /// 软件异步状态读取 *pState=0积分结束*pState=1正在积分 *pState=2正在采样
        /// </summary>
        /// <param name="spectrometerIndex"></param>      
        [DllImport("SpectraArsenal.dll", EntryPoint = "SA_GetStateAsyncSoftTrigger", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SA_GetStateAsyncSoftTrigger(int spectrometerIndex, out int pState);

        public int NET_GetStateAsyncSoftTrigger(int spectrometerIndex, out int pState)
        {
            #region
            int iResult = 0;
            try
            {
                iResult = SA_GetStateAsyncSoftTrigger(spectrometerIndex, out pState);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            return iResult;
            #endregion
        }

        #endregion
    }
}
