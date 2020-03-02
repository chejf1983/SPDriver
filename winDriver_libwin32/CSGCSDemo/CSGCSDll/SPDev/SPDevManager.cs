using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using CSGCSDll.Log;
using CSGCSDll.DLL.NqDll;
using CSGCSDll.Color;

namespace CSGCSDll.SPDev
{
    public class SPDevManager
    {
        private SPDevManager() { }
        private static SPDevManager instance;
        public static SPDevManager Instance
        {
            get
            {
                if (instance == null)
                {
                    instance = new SPDevManager();
                    if (instance.CalInstance == null)
                    {
                        FaultCenter.Instance.SendFault(FaultLevel.ERROR, "颜色初始化失败");
                    }

                } return instance;
            }
        }

        /// <summary>
        /// 设备列表
        /// </summary>
        private List<ISpDevice> dev_list = new List<ISpDevice>();
        public ISpDevice[] DevList { get { return dev_list.ToArray(); } }

        public bool SearchDevice()
        {
            try
            {
                dev_list.Clear();

                ISpDevice[] nq_devs = NqSpDevice.SearchNqDevices();
                dev_list.AddRange(nq_devs);

                return true;
            }
            catch (Exception ex)
            {
                FaultCenter.Instance.SendFault(FaultLevel.ERROR, ex.Message);
                return false;
            }
        }

        private ColorCal cal;
        public ColorCal CalInstance
        {
            get
            {
                if (cal == null)
                {
                    cal = new ColorCal();
                    if (cal.InitBill())
                        Console.WriteLine(cal.Version);
                    else
                        cal = null;
                }

                return cal;
            }
        }
    }
}
