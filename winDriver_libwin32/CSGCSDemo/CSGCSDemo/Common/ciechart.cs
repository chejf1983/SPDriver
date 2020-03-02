using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;
using LTISForm.other;

namespace CSGCSDemo.Common
{
    public partial class ciechart : UserControl
    {
        public ciechart()
        {
            InitializeComponent(); 
            this.initCIE1931();
        }

        private void initCIE1931()
        {
            //设备背景颜色，从上到下渐变
            this.cie1931.BackColor = Color.WhiteSmoke;
            this.cie1931.BackGradientStyle = GradientStyle.TopBottom;
            this.cie1931.BackSecondaryColor = Color.White;

            //设置边框颜色和样式
            this.cie1931.BorderlineColor = Color.Gray;
            this.cie1931.BorderlineDashStyle = ChartDashStyle.Solid;
            //curvechart.BorderlineWidth = 2;
            //curvechart.BorderSkin.SkinStyle = BorderSkinStyle.Emboss;

            // 设置显示范围
            ChartArea chartArea1 = cie1931.ChartAreas[0];
            //设置指示框位置
            cie1931.Legends[0].Docking = Docking.Bottom;

            /*设置3D阴影*/
            chartArea1.Area3DStyle.Inclination = 15;
            chartArea1.Area3DStyle.IsClustered = true;
            chartArea1.Area3DStyle.IsRightAngleAxes = false;
            chartArea1.Area3DStyle.Perspective = 10;
            chartArea1.Area3DStyle.Rotation = 10;
            chartArea1.Area3DStyle.WallWidth = 0;

            //设置X轴标签
            chartArea1.AxisX.LabelAutoFitStyle = (LabelAutoFitStyles)(((
                          LabelAutoFitStyles.IncreaseFont
                        | LabelAutoFitStyles.DecreaseFont)
                        | LabelAutoFitStyles.WordWrap));
            //x轴字体
            chartArea1.AxisX.LabelStyle.Font = new Font("Trebuchet MS", 8.25F, FontStyle.Bold);
            //x轴颜色
            chartArea1.AxisX.LineColor = Color.FromArgb(64, 64, 64, (64));
            //X轴网格线颜色和样式
            chartArea1.AxisX.MajorGrid.LineColor = Color.Gray;
            chartArea1.AxisX.MajorGrid.LineDashStyle = ChartDashStyle.Dash;
            //X轴滚动条样式
            chartArea1.AxisX.ScrollBar.LineColor = Color.White;
            //chartArea1.AxisX.ScrollBar.Size = 1;
            //设置X轴最大最小值
            chartArea1.AxisX.Minimum = -0.1;
            chartArea1.AxisX.Maximum = 0.8;
            //chartArea1.AxisX.MajorGrid.Interval = 0.1;
            //chartArea1.AxisX.IsStartedFromZero = false;
            //设置X轴间隔单位
            //chartArea1.AxisX.MajorGrid.IntervalOffset = 0.01;
            //chartArea1.AxisX.IntervalAutoMode = IntervalAutoMode.VariableCount;// = 0.1;
            //chartArea1.AxisX.IntervalOffset = 0.01;
            //chartArea1.AxisX.ScaleView.MinSize = double.NaN;
            chartArea1.AxisX.LabelStyle.Format = "#0.000";


            //是否可以缩放
            //chartArea1.CursorX.IsUserEnabled = true;
            chartArea1.CursorX.IsUserSelectionEnabled = true;
            chartArea1.AxisX.ScaleView.Zoomable = true;
            //缩放的最小单位(如果不设置NAN，无法无限放大)
            chartArea1.CursorX.Interval = double.NaN;
            //chartArea1.CursorX.IntervalOffset = 0.01;

            //设置Y轴标签
            chartArea1.AxisY.LabelStyle.Font = new Font("Trebuchet MS", 8.25F, FontStyle.Bold);
            //Y轴颜色
            chartArea1.AxisY.LineColor = Color.FromArgb(64, 64, 64, 64);
            //Y轴网格线颜色和样式
            chartArea1.AxisY.MajorGrid.LineDashStyle = ChartDashStyle.Dash;
            chartArea1.AxisY.MajorGrid.LineColor = Color.Gray;
            //Y轴滚动条样式
            chartArea1.AxisY.ScrollBar.LineColor = Color.White;
            //chartArea1.AxisY.ScrollBar.Size = 10;
            //设置Y轴最大最小值
            chartArea1.AxisY.Minimum = -0.10;
            chartArea1.AxisY.Maximum = 0.9;
            //chartArea1.AxisY.IsStartedFromZero = false;
            //设置Y轴间隔单位
            //chartArea1.AxisY.IntervalOffset = 0.01;
            chartArea1.AxisY.LabelStyle.Format = "#0.000";
            //chartArea1.AxisY.LabelStyle. = 0.001;

            //是否可以缩放
            //chartArea1.CursorY.IsUserEnabled = true;
            chartArea1.CursorY.IsUserSelectionEnabled = true;
            chartArea1.AxisY.ScaleView.Zoomable = true;
            //缩放的最小单位
            chartArea1.CursorY.Interval = double.NaN;
            //chartArea1.CursorY.IntervalOffset = 0.01;

            //设置CHART背景颜色，边框颜色
            chartArea1.BackColor = Color.Black;
            chartArea1.BorderColor = Color.FromArgb(64, 64, 64, 64);
            chartArea1.BorderDashStyle = ChartDashStyle.Solid;

            //标题区不显示
            cie1931.Legends[0].Enabled = false;

            chartArea1.Name = "CIE1931";
            chartArea1.ShadowColor = Color.Transparent;

            
            this.Clear();
        }

        private void InitCIE()
        {
            //背景图片填充方式
            //this.cie1931.ChartAreas[0].
           // this.cie1931.ChartAreas[0].BackImageWrapMode = ChartImageWrapMode.Scaled;
            //this.cie1931.ChartAreas[0].BackImage = @".\CIE_1931.png";
            //this.cie1931.ChartAreas[0].BackImageAlignment = ChartImageAlignmentStyle.Center;

            //chartArea1.BackImage
            /*
            Series line = new Series();

            // 线宽2个像素
            line.MarkerSize = 1;
            
            line.BackImage = @".\CIE_1931.png";
            line.BackImageWrapMode = ChartImageWrapMode.Scaled;
            //绘制CIE曲线
            line.ChartType = SeriesChartType.Area;
            line.BorderColor = Color.Red;
            line.CustomProperties = "PriceDownColor=Red,PriceUpColor=Green";
            line.BorderWidth = 2;
            line.Points.Clear();
            List<LTISForm.other.CieLine.FPoint> cieline = CieLine.ReadCieLine();
            for (int i = 0; i < cieline.Count; i += 100)
            {
                line.Points.AddXY(cieline[i].x, cieline[i].y);
                //cie1931.Series[0].Points[cie1931.Series[0].Points.Count - 1].Label = (380 + i / 100).ToString();
            };
            line.Points.AddXY(cieline[0].x, cieline[0].y);

            this.cie1931.Series.Add(line);*/
        }

        #region 打靶点
        private Dictionary<String, Series> seriers_map = new Dictionary<string, Series>();
        private int max_point_num = 3000;
        public int MaxPointNum { get { return this.max_point_num; } set { this.max_point_num = value; } }

        /// <summary>
        /// 清空数据
        /// </summary>
        public void Clear()
        {
            this.cie1931.Series.Clear();
            this.seriers_map.Clear();
            this.InitCIE();
        }

        /// <summary>
        /// 增加点
        /// </summary>
        /// <param name="title">点名称</param>
        /// <param name="x"></param>
        /// <param name="y"></param>
        public void displayData(String title, float x, float y)
        {
            //初始化点序列
            if (!seriers_map.ContainsKey(title)) BuildSeries(title);

            //找到点序列
            Series series = seriers_map[title];
            //修改描述名称
            series.LegendText = title + "(" + x + "," + y + ")";

            //超过最大点，清理旧点
            if (series.Points.Count > MaxPointNum)
            {
                series.Points.RemoveAt(0);
            }
            //添加坐标点
            series.Points.AddXY(x, y);
        }

        //创建点图
        private Series BuildSeries(string name)
        {
            Series line = new Series(name);
            // 画样条曲线(Spline)
            line.ChartType = SeriesChartType.Point;
            // 线宽2个像素
            line.MarkerSize = 2;

            seriers_map.Add(name, line);

            this.cie1931.Series.Add(line);

            return line;
        }
        #endregion

        #region 放大恢复
        int xold = 0;
        int yold = 0;
        private void curvechart_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                xold = e.X;
                yold = e.Y;
            }
        }

        private void curvechart_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                if (e.X < xold && e.Y < yold)
                {
                    this.Reset();
                }

            }
        }

        public void Reset()
        {
            this.cie1931.ChartAreas[0].AxisX.ScaleView.ZoomReset(0);
            this.cie1931.ChartAreas[0].AxisY.ScaleView.ZoomReset(0);
        }
        #endregion
    }
}
