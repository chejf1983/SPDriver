using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using CSGCSDll.SPDev;
using Custom.Devices;

namespace CSGCSDll.DLL.NqDll
{
    public class GCSDevice : ISpDevice
    {
        private GCS2000 dev = new GCS2000();

        public static ISpDevice[] SearchNqDevices()
        {
            Nahon_DLL.SA_CloseSpectrometers();

            int devnum = GCS2000.SA_OpenSpectrometers();
            if (devnum < 0)
            {
                throw new Exception("没有找到光谱仪");
            }

            GCSDevice[] devices = new GCSDevice[devnum + 1];
            for (int i = 0; i < devices.Length; i++)
            {
                devices[i] = new GCSDevice(i);
                devices[i].UpdateDevice();
            }
            return devices;
        }

        private int pix_num = 0;
        private int index = 0;
        public GCSDevice(int index)
        {
            this.index = index;
        }

        /// <summary>
        /// 更新设备
        /// </summary>
        public void UpdateDevice()
        {
            name = this.dev.NET_GetSpectrometersName(this.index);
            s = this.dev.NET_GetSerialNumber(this.index);

            pix_num = this.dev.NET_GetSpectrometerPixelsNumber(this.index);
            if (pix_num < 0)
            {
                throw new Exception("获取像素个数失败");
            }

            //获取波长系数
            sp_wave = new double[pix_num];
            int num = 0;
            this.dev.NET_GetWavelength(this.index, sp_wave, out num);
        }

        /// <summary>
        /// 设备名称
        /// </summary>
        private String name;
        public string SpName
        {
            get { return name; }
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

        public int DevIndex
        {
            get { return this.index; }
        }

        private string s;
        public string SpSerialNum
        {
            get { return s; }
        }

        /// <summary>
        /// 获取光谱仪波长范围
        /// </summary>
        private double[] sp_wave;
        public double[] SpWaveArray
        {
            get { return sp_wave; }
        }


        public void SetIntegerTime(int ms)
        {
            int ret = this.dev.NET_SetIntegrationTime(this.index, ms * 1000);
            if (ret < 0)
            {
                throw new Exception("积分时间设置失败");
            }
        }

        public double[] GetSpectralData(bool non_linear = true)
        {
            int data_len = 0;
            double[] data = new double[pix_num];
            int ret = dev.NET_GetSpectum(this.index, data, out data_len);
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
                dev.NET_NonlinearCalibration(this.index, data, newdata, data.Length);
                return newdata;
            }
            else
                return data;
        }

        private double[] dk_data = new double[0];
        /// <summary>
        /// 暗电流扣除
        /// </summary>
        /// <param name="ms"></param>
        /// <returns></returns>
        public double[] DKModify(int ms)
        {
            int ret = this.dev.NET_SetIntegrationTime(this.index, ms * 1000);
            if (ret < 0)
            {
                throw new Exception("积分时间设置失败");
            }

            int data_len = 0;
            double[] data = new double[pix_num];
            ret = dev.NET_GetSpectum(this.index, data, out data_len);
            if (ret < 0)
            {
                throw new Exception("采集光谱失败");
            }

            dk_data = data;
            return data;

        }
    }
}
