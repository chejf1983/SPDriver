using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using CSGCSDll.SPDev;

namespace CSGCSDll.DLL.NqDll
{
    public class NqSpDevice : ISpDevice
    {
        public static ISpDevice[] SearchNqDevices()
        {
            Nahon_DLL.SA_CloseSpectrometers();

            int devnum = Nahon_DLL.SA_OpenSpectrometers();
            if (devnum < 0)
            {
                throw new Exception("没有找到光谱仪");
            }

            ISpDevice[] devices = new ISpDevice[devnum + 1];
            for (int i = 0; i < devices.Length; i++)
            {
                devices[i] = new NqSpDevice(i);
                devices[i].UpdateDevice();
            }
            return devices;
        }

        /// <summary>
        /// 设备序列号
        /// </summary>
        private int devindex;
        public int DevIndex { get { return devindex; } }
        private NqSpDevice(int index)
        {
            this.devindex = index;
        }

        /// <summary>
        /// 刷新设备数据
        /// </summary>
        public void UpdateDevice()
        {
            //读取序列号
            IntPtr ptr = Nahon_DLL.SA_GetSerialNumber(this.devindex);
            sp_serialNum = Marshal.PtrToStringAnsi(ptr);

            //读取设备名称
            ptr = Nahon_DLL.SA_GetSpectrometersName(this.devindex);
            sp_name = Marshal.PtrToStringAnsi(ptr);

            pix_num = Nahon_DLL.SA_GetSpectrometerPixelsNumber(this.devindex);
            if (pix_num < 0)
            {
                throw new Exception("获取像素个数失败");
            }

            //获取波长系数
            sp_wave = new double[pix_num];
            int num = 0;
            Nahon_DLL.SA_GetWavelength(this.devindex, sp_wave, ref num);
        }

        /// <summary>
        /// 设备名称
        /// </summary>
        private String sp_name = "";
        public string SpName
        {
            get { return sp_name; }
        }

        /// <summary>
        /// 设备序列号
        /// </summary>
        private String sp_serialNum = "";
        public string SpSerialNum
        {
            get { return sp_serialNum; }
        }

        private int pix_num = 0;

        /// <summary>
        /// 设置积分时间
        /// </summary>
        public void SetIntegerTime(int ms)
        {
            int ret = Nahon_DLL.SA_SetIntegrationTime(this.devindex, ms * 1000);
            if (ret < 0)
            {
                throw new Exception("积分时间设置失败");
            }
        }

        //double[] data = new double[5000];
        //double[] newdata = new double[5000];
        /// <summary>
        /// 获取光谱仪原始数据
        /// </summary>
        /// <returns></returns>
        public double[] GetSpectralData(bool non_linear = true)
        {
            int data_len = 0;
            double[] data = new double[pix_num];
            int ret = Nahon_DLL.SA_GetSpectum(this.devindex, data, ref data_len);
            if (ret < 0)
            {
                throw new Exception("采集光谱失败");
            }

            if (data.Length == dk_data.Length)
            {
                for (int i = 0; i < data.Length; i++)
                {
                    data[i] -= dk_data[i];
                }
            }

            if (non_linear)
            {
                double[] newdata = new double[data.Length];
                Nahon_DLL.SA_NonlinearCalibration(this.devindex, data, newdata, data.Length);
                return newdata;
            }
            else
                return data;
        }



        /// <summary>
        /// 获取光谱仪波长范围
        /// </summary>
        private double[] sp_wave;
        public double[] SpWaveArray
        {
            get { return sp_wave; }
        }

        private double[] dk_data = new double[0];
        /// <summary>
        /// 暗电流扣除
        /// </summary>
        /// <param name="ms"></param>
        /// <returns></returns>
        public double[] DKModify(int ms)
        {
            double[] dk_data = new double[0];

            SetIntegerTime(ms);

            dk_data = GetSpectralData(false);
            return dk_data;

        }

        private bool enable = true;
        public bool Enable
        {
            get
            {
                return enable;
            }
            set
            {
                enable = value;
            }
        }
    }
}
