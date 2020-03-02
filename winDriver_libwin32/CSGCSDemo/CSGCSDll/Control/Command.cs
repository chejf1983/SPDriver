using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CSGCSDll.Control
{
    public abstract class Command
    {
        /// <summary>
        /// 是否执行结束
        /// </summary>
        private bool isFinish = false;
        public bool IsFinish { get { return isFinish; } set { this.isFinish = value; } }

        /// <summary>
        /// 测试准备，申请资源
        /// </summary>
        /// <returns></returns>
        public bool preTest() { return true; }

        /// <summary>
        /// 测试过程
        /// </summary>
        /// <returns>是否结束</returns>
        public abstract void Test();

        /// <summary>
        /// 测试结束，释放资源
        /// </summary>
        /// <returns></returns>
        public bool AfterTest() { return true; }
    }
}
