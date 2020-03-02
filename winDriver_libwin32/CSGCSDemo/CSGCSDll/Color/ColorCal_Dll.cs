using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;

namespace CSGCSDll.Color
{
    public class ColorCal_Dll
    {
        #region 颜色数据

        [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        private struct COLOR_PARA
        {
            public float fx;        //色坐标
            public float fy;
            public float fu;
            public float fv;

            public float fCCT;      //相关色温(K)
            public float fdC;        //色差dC
            public float fLd;       //主波长(nm)
            public float fPur;      //色纯度(%)
            public float fLp;       //峰值波长(nm)
            public float fHW;       //半波宽(nm)
            public float fLav;      //平均波长(nm)
            public float fRa;       //显色性指数 Ra
            public float fRR;       //红色比
            public float fGR;       //绿色比
            public float fBR;       //蓝色比
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 15)]
            public float[] fRi;   //显色性指数 R1-R15

            public float fIp;      //峰值AD

            public float fPh;      //光度值
            public float fPhe;     //辐射度值
            public float fPlambda; //绝对光谱洗漱
            public float fSpect1;  //起始波长
            public float fSpect2;  //
            public float fInterval; //波长间隔

            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10000)]
            public float[] fPL;   //光谱数据

            public float fRf;
            public float fRg;
        };

        /// <summary>
        /// 转换类型
        /// </summary>
        /// <param name="colorpar"></param>
        /// <returns></returns>
        private ColorData ColorSTC(COLOR_PARA colorpar)
        {
            //填值
            ColorData data = new ColorData();
            data.fx = colorpar.fx;
            data.fy = colorpar.fy;
            data.fu = colorpar.fu;
            data.fv = colorpar.fv;
            data.fCCT = colorpar.fCCT;
            data.fdC = colorpar.fdC;
            data.fLd = colorpar.fLd;
            data.fPur = colorpar.fPur;
            data.fLp = colorpar.fLp;
            data.fHW = colorpar.fHW;
            data.fLav = colorpar.fLav;
            data.fRa = colorpar.fRa;
            data.fRR = colorpar.fRR;
            data.fGR = colorpar.fGR;
            data.fBR = colorpar.fBR;
            data.fRi = colorpar.fRi;
            data.fIp = colorpar.fIp;
            data.fPh = colorpar.fPh;
            data.fPhe = colorpar.fPhe;
            data.fPlambda = colorpar.fPlambda;
            data.fSpect1 = colorpar.fSpect1;
            data.fSpect2 = colorpar.fSpect2;
            data.fInterval = colorpar.fInterval;
            float intv = colorpar.fInterval == 0 ? 1 : colorpar.fInterval;
            data.fPL = new float[(int)((colorpar.fSpect2 - colorpar.fSpect1) / intv)];
            if (colorpar.fPL != null)
                Array.Copy(colorpar.fPL, data.fPL, data.fPL.Length);
            return data;
        }
        private COLOR_PARA ColorCTS(ColorData colorpar)
        {
            //填值
            COLOR_PARA data = new COLOR_PARA();
            data.fx = colorpar.fx;
            data.fy = colorpar.fy;
            data.fu = colorpar.fu;
            data.fv = colorpar.fv;
            data.fCCT = colorpar.fCCT;
            data.fdC = colorpar.fdC;
            data.fLd = colorpar.fLd;
            data.fPur = colorpar.fPur;
            data.fLp = colorpar.fLp;
            data.fHW = colorpar.fHW;
            data.fLav = colorpar.fLav;
            data.fRa = colorpar.fRa;
            data.fRR = colorpar.fRR;
            data.fGR = colorpar.fGR;
            data.fBR = colorpar.fBR;
            data.fRi = colorpar.fRi;
            data.fIp = colorpar.fIp;
            data.fPh = colorpar.fPh;
            data.fPhe = colorpar.fPhe;
            data.fPlambda = colorpar.fPlambda;
            data.fSpect1 = colorpar.fSpect1;
            data.fSpect2 = colorpar.fSpect2;
            data.fInterval = colorpar.fInterval;
            float intv = colorpar.fInterval == 0 ? 1 : colorpar.fInterval;
            data.fPL = new float[(int)((colorpar.fSpect2 - colorpar.fSpect1) / intv)];
            if (colorpar.fPL != null)
                Array.Copy(colorpar.fPL, data.fPL, data.fPL.Length);
            return data;
        }
        #endregion

        [DllImport("NH_GetColorParaDLL.dll", EntryPoint = "NH_Init_ASCII")]
        private static extern bool NH_Init_ASCII(char[] strPW1, char[] strPW2, char[] strPath);
        public bool InitColorDll(string str_pw1, string str_pw2, string str_path)
        {
            return NH_Init_ASCII(str_pw1.ToCharArray(), str_pw2.ToCharArray(), str_path.ToCharArray());
        }

        [DllImport("NH_GetColorParaDLL.dll", EntryPoint = "NH_GetVer_ASCII")]
        private static extern void NH_GetVer_ASCII(byte[] strPath);
        public string GetVersion()
        {
            //获取序列号，像素点个数
            byte[] serialnum = new byte[50];
            NH_GetVer_ASCII(serialnum);
            //初始化设备参数
            string serialNum = System.Text.Encoding.Default.GetString(serialnum);
            serialNum = serialNum.Replace('\0', ' ');
            serialNum = serialNum.Trim();

            return serialNum;
        }

        [DllImport("NH_GetColorParaDLL.dll", EntryPoint = "NH_CalcColor")]
        private static extern void NH_CalcColor(float[] fWL, float[] fPL, int iN, ref COLOR_PARA  para,bool bXY2CCT,float xx,float yy);
        public ColorData CalColor(float[] fwl, float [] fPL, float xx, float yy){
            COLOR_PARA par = new COLOR_PARA();
            NH_CalcColor(fwl, fPL, fwl.Length, ref par, false, xx, yy);
            return this.ColorSTC(par);
        }

        [DllImport("NH_GetColorParaDLL.dll", EntryPoint = "NH_GetV")]
        private static extern float NH_GetV(float fWL);
        public float GetGLamdWL(float fwl)
        {
            return NH_GetV(fwl);
        }

        [DllImport("NH_GetColorParaDLL.dll", EntryPoint = "NH_GetNewPara")]
        private static extern float NH_GetNewPara(ref float fCDI, ref float fRf, ref float fRg);

    }


    [Serializable]
    public class ColorData
    {
        public float fx;        //色坐标
        public float fy;
        public float fu;
        public float fv;

        public float fCCT;      //相关色温(K)
        public float fdC;        //色差dC
        public float fLd;       //主波长(nm)
        public float fPur;      //色纯度(%)
        public float fLp;       //峰值波长(nm)
        public float fHW;       //半波宽(nm)
        public float fLav;      //平均波长(nm)
        public float fRa;       //显色性指数 Ra
        public float fRR;       //红色比
        public float fGR;       //绿色比
        public float fBR;       //蓝色比
        public float[] fRi;   //显色性指数 R1-R15

        public float fIp;      //峰值AD

        public float fPh;      //光度值
        public float fPhe;     //辐射度值
        public float fPlambda; //绝对光谱洗漱
        public float fSpect1;  //起始波长
        public float fSpect2;  //
        public float fInterval; //波长间隔

        public float[] fPL;   //光谱数据

        public float fRf;
        public float fRg;

        public ColorData DeepClone()
        {
            object obj = null;
            //将对象序列化成内存中的二进制流
            BinaryFormatter inputFormatter = new BinaryFormatter();
            MemoryStream inputStream;

            using (inputStream = new MemoryStream())
            {
                inputFormatter.Serialize(inputStream, this);
            }

            //将二进制流反序列化为对象
            using (MemoryStream outputStream = new MemoryStream(inputStream.ToArray()))
            {
                BinaryFormatter outputFormatter = new BinaryFormatter();
                obj = outputFormatter.Deserialize(outputStream);
            }

            return (ColorData)obj;
        }
    }
}
