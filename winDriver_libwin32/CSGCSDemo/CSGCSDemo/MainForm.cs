using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading.Tasks;
using System.Threading;
using CSGCSDll.SPDev;
using CSGCSDll.Log;
using CSGCSDll.Color;
using CSGCSDemo.Common;

namespace CSGCSDemo
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        static Semaphore sema = new Semaphore(1, 1);

        /// <summary>
        /// 搜索
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void bt_search_Click(object sender, EventArgs e)
        {
            clst_devs.Items.Clear();
            if (SPDevManager.Instance.SearchDevice())
            {
                for (int i = 0; i < SPDevManager.Instance.DevList.Length; i++)
                    clst_devs.Items.Add(SPDevManager.Instance.DevList[i].SpSerialNum, SPDevManager.Instance.DevList[i].Enable);
            }
        }

        /// <summary>
        /// 采集
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void bt_collect_Click(object sender, EventArgs e)
        {
            lock (this)
            {
                if (collect_start) return;
                collect_start = true;
            }

            updatestate();
            num = 0;
            //ciechart.Clear();
            sp_chart.Clear();

            for (int i = 0; i < SPDevManager.Instance.DevList.Length; i++)
            {
                if (SPDevManager.Instance.DevList[i].Enable)
                    RunDevice(SPDevManager.Instance.DevList[i]);
            }
        }

        private int num = 0;
        static Semaphore sp = new Semaphore(1, 1);
        static Semaphore cie = new Semaphore(1, 1);
        /// <summary>
        /// 显示光谱曲线
        /// </summary>
        /// <param name="name"></param>
        /// <param name="datas"></param>
        private void ShowSPData(String name, string dev_index, double[] wave, double[] datas)
        {
            if (!base.InvokeRequired)
            {
                sp_chart.displayData(name, dev_index, wave, datas);
                num++;
                this.lb_num.Text = num + "";
                GC.Collect();
                sp.Release();
            }
            else
            {
                BeginInvoke(new Action<String, string, double[], double[]>(ShowSPData), name, dev_index, wave, datas);
            }
        }

        /// <summary>
        /// 显示CIE
        /// </summary>
        /// <param name="name"></param>
        /// <param name="x"></param>
        /// <param name="y"></param>
        private void ShowCIE(String name, float x, float y)
        {
            if (!base.InvokeRequired)
            {
                //ciechart.displayData(name, x, y);
                cie.Release();
            }
            else
            {
                BeginInvoke(new Action<String, float, float>(ShowCIE), name, x, y);
            }
        }

        private bool collect_start = false;
        /// <summary>
        /// 开始测试
        /// </summary>
        private void RunDevice(ISpDevice dev)
        {
            int int_time = int.Parse(this.tb_intertime.Text);

            int ivl_time = int.Parse(this.tb_intervaltime.Text);

            Task.Factory.StartNew(() =>
               {
                   try
                   {
                       sema.WaitOne();
                       //设置积分时间
                       dev.SetIntegerTime(int_time);
                       sema.Release();
                   }
                   catch (Exception ex)
                   {
                       FaultCenter.Instance.SendFault(FaultLevel.ERROR, ex.Message);
                       //FaultCenter("异常！！" + ex);
                   }

                   try
                   {
                       Thread.Sleep(ivl_time);
                       while (collect_start)
                       {
                           double[] data = dev.GetSpectralData();
                           //Console.WriteLine(dev.DevIndex);
                           sp.WaitOne();
                           ShowSPData(dev.SpSerialNum, dev.DevIndex + "", dev.SpWaveArray, data);
                           ColorData color = SPDevManager.Instance.CalInstance.GetColor(dev.SpWaveArray, data);
                           cie.WaitOne();
                           ShowCIE(dev.SpSerialNum, color.fx, color.fy);
                           Thread.Sleep(ivl_time);
                       }
                   }
                   catch (Exception ex)
                   {
                       FaultCenter.Instance.SendFault(FaultLevel.ERROR, ex.Message);
                       //sema.Release();
                       //FaultCenter("异常！！" + ex);
                   }
               });
        }

        /// <summary>
        /// 停止测试
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void button1_Click(object sender, EventArgs e)
        {
            collect_start = false;
        }

        private void bt_dk_Click(object sender, EventArgs e)
        {
            updatestate();
            int int_time = int.Parse(this.tb_intertime.Text);
            for (int i = 0; i < SPDevManager.Instance.DevList.Length; i++)
            {
                ISpDevice dev = SPDevManager.Instance.DevList[i];
                if (!dev.Enable)
                {
                    continue;
                }

                try
                {
                    double[] data = dev.DKModify(int_time);
                    sp.WaitOne();
                    ShowSPData(dev.SpSerialNum, dev.DevIndex + "", dev.SpWaveArray, data);
                }
                catch (Exception ex)
                {
                    FaultCenter.Instance.SendFault(FaultLevel.ERROR, ex.Message);
                }
            }
        }

        private void clst_devs_ImeModeChanged(object sender, EventArgs e)
        {
            MessageBox.Show("check");
        }

        private void updatestate()
        {
            for (int i = 0; i < clst_devs.Items.Count; i++)
            {
                SPDevManager.Instance.DevList[i].Enable = clst_devs.GetItemChecked(i);
            }

            clst_devs.Items.Clear();
            for (int i = 0; i < SPDevManager.Instance.DevList.Length; i++)
                clst_devs.Items.Add(SPDevManager.Instance.DevList[i].SpSerialNum, SPDevManager.Instance.DevList[i].Enable);
        }

        private void clst_devs_SelectedValueChanged(object sender, EventArgs e)
        {

        }

        private void button_cie_Click(object sender, EventArgs e)
        {
            new CIE().Show(this);
        }
    }
}
