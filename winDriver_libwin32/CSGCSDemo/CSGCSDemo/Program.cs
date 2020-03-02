using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using CSGCSDll.Log;

namespace CSGCSDemo
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            FaultCenter.Instance.FaultEvent +=
                new ReceiveFault(delegate(FaultLevel level, string info)
                {
                    MessageBox.Show(info, level.ToString());
                });

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}
