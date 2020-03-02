using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSGCSDll.Control
{
    class Excutors
    {
        private int run_num = 0;
        private int gc_limit = 100;

        public void RunCommand(Command cmd)
        {
            //启动一个线程
            Task.Factory.StartNew(() =>
               {
                   //测试准备
                   if (!cmd.preTest()) return;

                   try
                   {
                       //循环测试
                       while (cmd.IsFinish)
                       {
                           cmd.Test();
                           //强制垃圾内存回收
                           run_num++;
                           if (run_num % gc_limit == 0)
                           {
                               run_num = 0;
                               GC.Collect();
                           }
                       }
                   }
                   catch (Exception ex)
                   {
                       //异常打印错误，一般异常在Test()函数内打印
                       Log.FaultCenter.Instance.SendFault(Log.FaultLevel.SYSTEM, ex.Message);
                   }
                   finally
                   {
                       //测试结束
                       cmd.AfterTest();
                   }
               });
        }
    }
}

