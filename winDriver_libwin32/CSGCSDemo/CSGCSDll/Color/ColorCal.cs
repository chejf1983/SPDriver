using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CSGCSDll.Color
{
    public class ColorCal
    {
        private ColorCal_Dll cal_bin = new ColorCal_Dll();
        private float[] g_fLamdaWL = new float [4001];
        
        private float fSpect1 = 380.0f;
        private float fSpect2 = 780.0f;
        private float fInterval = 0.1f;

        public bool InitBill()
        {
            bool ret = cal_bin.InitColorDll("NAHONG_TECH_NAHONG", "GET_COLOR_PARA", ".\\");

            for (int i = 0; i < g_fLamdaWL.Length; i++)
            {
                g_fLamdaWL[i] = cal_bin.GetGLamdWL(fSpect1 + i * fInterval);
            }

            return ret;
        }

        public string Version
        {
            get
            {
                return cal_bin.GetVersion();
            }
        }

        /// <summary>
        /// 牛顿插值
        /// </summary>
        /// <param name="g_fPixWL">原始波长</param>
        /// <param name="fAD">原始值</param>
        /// <param name="new_fwl"></param>
        /// <param name="new_fpl"></param>
        /// <returns>最大new_fpl</returns>
        public float NewTonInsert(double[] g_fPixWL, double[] fAD, ref float[] new_fwl, ref float[] new_fpl)
        {
            int num = (int)((fSpect2 - fSpect1) / fInterval);
            new_fwl = new float[num];
            new_fpl = new float[num];
            int iPerStart = 1;
            float fTempMax = 0;
            for (int i = 0; i < num; i++)
            {
                float fTempL = fSpect1 + i * fInterval;
                float fTempAD = 0;
                double u = 0;

                //外插处理第一个点的数据
                if (fTempL < g_fPixWL[0])
                {
                    //fTempAD = 0;//fAD[0];
                }
                else if (fTempL <= g_fPixWL[g_fPixWL.Length - 3])
                {
                    int m, k;
                    k = 1;
                    for (m = iPerStart; m <= g_fPixWL.Length - 2; m++)
                    {
                        if (fTempL <= g_fPixWL[m])
                        {
                            k = m;
                            break;
                        }
                        else
                            k = g_fPixWL.Length - 1;
                    }

                    if (iPerStart != k)
                    {
                        iPerStart = k;
                        if ((g_fPixWL[k] - g_fPixWL[k - 1]) != 0)
                        {
                            u = (fAD[iPerStart] - fAD[iPerStart - 1]) / (g_fPixWL[iPerStart] - g_fPixWL[iPerStart - 1]);
                        }
                        else
                        {
                            u = 0;
                        }
                    }
                    fTempAD = (float)(fAD[k - 1] + u * (fTempL - g_fPixWL[k - 1]));
                }
                else
                {
                    //fTempAD = 0;//fLastAD;
                }
                new_fwl[i] = fTempL;
                new_fpl[i] = fTempAD;
                if (fTempMax < fTempAD) fTempMax = fTempAD;
            }

            return fTempMax;
        }

        public ColorData GetColor(double[] fwl, double[] fpl)
        {
            float[] x = new float[0], y = new float[0];
            float maxy = NewTonInsert(fwl, fpl,ref x, ref y);
            for (int i = 0; i < y.Length; i++)
            {
                y[i] = y[i] / maxy;
            }
            return cal_bin.CalColor(x, y, 0, 0);
        }
    }
}
