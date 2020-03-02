using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CSGCSDll.SPDev
{
    public interface ISpDevice
    {
        /// <summary>
        /// 刷新设备数据
        /// </summary>
        void UpdateDevice();

        /// <summary>
        /// 获取设备名称
        /// </summary>
        /// <returns></returns>
        String SpName { get; }

        /// <summary>
        /// 选中
        /// </summary>
        bool Enable { get; set; }
        
        /// <summary>
        /// 设置序号
        /// </summary>
        int DevIndex { get; }

        /// <summary>
        /// 获取序列号
        /// </summary>
        /// <returns></returns>
        String SpSerialNum { get; }

        /// <summary>
        /// 获取序列号
        /// </summary>
        /// <returns></returns>
        double [] SpWaveArray { get; }

        /// <summary>
        /// 设置积分时间
        /// </summary>
        /// <param name="ms">积分时间(ms)</param>
        void SetIntegerTime(int ms);

        /// <summary>
        /// 获取光谱仪原始数据
        /// </summary>
        /// <returns></returns>
        double[] GetSpectralData(bool non_linear = true);

        /// <summary>
        /// 扣除暗电流
        /// </summary>
        /// <param name="ms">暗电流时间</param>
        /// <returns>暗电流光谱</returns>
        double[] DKModify(int ms);
    }
}
