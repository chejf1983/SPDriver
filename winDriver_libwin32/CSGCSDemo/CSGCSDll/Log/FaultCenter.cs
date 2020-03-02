using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace CSGCSDll.Log
{
    public enum FaultLevel
    {
        SYSTEM,
        ERROR,
        WARNING,
        INFO
    }

    public delegate void ReceiveFault(FaultLevel level, String info);

    public class FaultCenter
    {
        private static FaultCenter instance;
        private FaultCenter() {
            this.InitSysLog();
        }
        public static FaultCenter Instance
        {
            get
            {
                if (instance == null) { instance = new FaultCenter(); }
                return instance;
            }
        }
        
        /// <summary>
        /// 初始化Log
        /// </summary>
        private void InitSysLog()
        {
            ///初始化，检查log文件夹是否存在
            if (!Directory.Exists(logDir))
            {
                Directory.CreateDirectory(this.logDir);

            }

            DirectoryInfo dyInfo = new DirectoryInfo(this.logDir);
            int filenum = dyInfo.GetFiles().Length;
            foreach (FileInfo feInfo in dyInfo.GetFiles())
            {
                if (filenum > 50)
                {
                    feInfo.Delete();
                    filenum--;
                }
            }
            this.currentlen = 0;
        }

        #region Error
        /// <summary>
        /// 出错事件
        /// </summary>
        public event ReceiveFault FaultEvent;

        /// <summary>
        /// 上报错误
        /// </summary>
        /// <param name="level"></param>
        /// <param name="info"></param>
        public void SendFault(FaultLevel level, String info)
        {
            lock (this)
            {
                if (this.FaultEvent != null)
                {
                    this.FaultEvent(level, info);
                }
                PrintLog("[" + level.ToString() + "]" + info);
            }
        }
        #endregion

        #region Log
        /// <summary>
        /// log文件夹路径
        /// </summary>
        private string logDir = "./log";
        public string LogPath { get { return this.logDir; } }
        /// <summary>
        /// 当前文件记录行数
        /// </summary>
        private long currentlen = 0;
        /// <summary>
        /// 文件最多保存log条数
        /// </summary>
        private long maxFileLen = 1000;

        private string currentfilename = "";

        /// <summary>
        /// 添加记录
        /// </summary>
        /// <param name="ciedata"></param>
        public void PrintLog(string log)
        {
            lock (this)
            {

                //文件流
                FileStream currentfile;

                if (this.currentlen == 0)
                {
                    ///如果当前记录条目重新计数，则创建新的数据文件，文件名按照时间来取
                    currentfilename = DateTime.Now.ToString(@"yyyy_MM_dd HH_mm_ss") + ".txt";
                    currentfile = File.Create(this.logDir + "/LOG" + currentfilename);
                }
                else
                {
                    //否则打开就文件
                    currentfile = File.Open(this.logDir + "/LOG" + this.currentfilename, FileMode.Append);
                }

                if (currentfile != null)
                {
                    ///写入文件
                    StreamWriter sw = new StreamWriter(currentfile);
                    ///转换格式填入
                    sw.WriteLine(DateTime.Now.ToString(@"yyyy:MM:dd HH:mm:ss fff") + ":" + log);
                    sw.Flush();
                    sw.Close();
                    currentfile.Close();
                    ///增加计数
                    if (this.currentlen++ > this.maxFileLen)
                    {
                        this.currentlen = 0;
                    }
                }
            }
        }
        #endregion 
    }
}
