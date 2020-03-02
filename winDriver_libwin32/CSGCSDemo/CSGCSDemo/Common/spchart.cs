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

namespace CSGCSDemo.Common
{
    public partial class spchart : UserControl
    {
        public spchart()
        {
            InitializeComponent();
            this.initCurveChart();
        }

        private void initCurveChart()
        {
            // 设置显示范围
            ChartArea chartArea1 = this.curvechart.ChartAreas[0];

            curvechart.Legends[0].Docking = Docking.Bottom;

            //设备背景颜色，从上到下渐变
            curvechart.BackColor = Color.FromArgb((240), (240), (240));
            curvechart.BackGradientStyle = GradientStyle.TopBottom;
            curvechart.BackSecondaryColor = Color.White;

            //设置边框颜色和样式
            curvechart.BorderlineColor = Color.Gray;
            curvechart.BorderlineDashStyle = ChartDashStyle.Solid;
            //curvechart.BorderlineWidth = 2;
            //curvechart.BorderSkin.SkinStyle = BorderSkinStyle.Emboss;


            /*设置3D阴影*/
            chartArea1.Area3DStyle.Inclination = 15;
            chartArea1.Area3DStyle.IsClustered = true;
            chartArea1.Area3DStyle.IsRightAngleAxes = false;
            chartArea1.Area3DStyle.Perspective = 10;
            chartArea1.Area3DStyle.Rotation = 10;
            chartArea1.Area3DStyle.WallWidth = 0;

            //设置X轴标签
            chartArea1.AxisX.LabelAutoFitStyle = (LabelAutoFitStyles)(((LabelAutoFitStyles.IncreaseFont | LabelAutoFitStyles.DecreaseFont)
                        | LabelAutoFitStyles.WordWrap));
            chartArea1.AxisX.LabelStyle.Font = new Font("Trebuchet MS", 8.25F, FontStyle.Bold);
            //x轴颜色
            chartArea1.AxisX.LineColor = Color.FromArgb(64, 64, 64, (64));
            //X轴网格线颜色和样式
            chartArea1.AxisX.MajorGrid.LineColor = Color.Gray;
            chartArea1.AxisX.MajorGrid.LineDashStyle = ChartDashStyle.Dash;
            //X轴滚动条样式
            chartArea1.AxisX.ScrollBar.LineColor = Color.Black;
            chartArea1.AxisX.ScrollBar.Size = 10;

            //chartArea1.AxisX. = 100;

            chartArea1.AxisX.LabelStyle.Format = "#0";


            //设置X方向是否可以移动
            chartArea1.AxisX.ScaleView.Zoomable = true;
            //chartArea1.CursorX.IsUserEnabled = true;
            chartArea1.CursorX.IsUserSelectionEnabled = true;


            //设置Y轴标签
            chartArea1.AxisY.LabelStyle.Font = new Font("Trebuchet MS", 8.25F, FontStyle.Bold);
            //Y轴颜色
            chartArea1.AxisY.LineColor = Color.FromArgb(64, 64, 64, 64);
            //Y轴网格线颜色和样式
            chartArea1.AxisY.MajorGrid.LineDashStyle = ChartDashStyle.Dash;
            chartArea1.AxisY.MajorGrid.LineColor = Color.Gray;
            //Y轴滚动条样式
            chartArea1.AxisY.ScrollBar.LineColor = Color.Black;
            chartArea1.AxisY.ScrollBar.Size = 10;

            // chartArea1.AxisY.IsStartedFromZero = false;
            //            chartArea1.AxisY.Minimum = 0.01;

            //设置Y方向是否可以移动
            //chartArea1.CursorY.IsUserEnabled = true;
            chartArea1.CursorY.IsUserSelectionEnabled = true;
            chartArea1.AxisY.ScaleView.Zoomable = true;
            //设置Y方向最小放大到0.01
            chartArea1.CursorY.Interval = 0.01;
            chartArea1.CursorY.IntervalOffset = 0.01;

            //设置CHART背景颜色，边框颜色
            chartArea1.BackColor = Color.Black;
            chartArea1.BorderColor = Color.FromArgb(64, 64, 64, 64);
            chartArea1.BorderDashStyle = ChartDashStyle.Solid;
            //设置CHART背景颜色，边框颜色
            //chartArea1.BackColor = System.Drawing.Color.WhiteSmoke;
            //chartArea1.BackGradientStyle = System.Windows.Forms.DataVisualization.Charting.GradientStyle.TopBottom;
            //chartArea1.BackSecondaryColor = System.Drawing.Color.White;
            //设置边框颜色样式
            //chartArea1.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            //chartArea1.BorderDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.Solid;

            chartArea1.Name = "Curve";
            chartArea1.ShadowColor = System.Drawing.Color.Transparent;
            //chartArea1.AxisY.MaximumAutoSize = 1.0f;

            //Series series = this.curvechart.Series[0];

            // 画样条曲线（Spline）
            //series.ChartType = SeriesChartType.FastLine;
            // 线宽2个像素
            //series.BorderWidth = 2;
            // 线的颜色：红色
            //series.Color = System.Drawing.Color.CadetBlue;

            //this.curvechart.MouseMove += Spectralchart_MouseMove;

            // 设置显示范围
            //ChartArea chartArea1 = this.curvechart.ChartAreas[0];
            chartArea1.AxisX.Minimum = 380;
            chartArea1.AxisX.Maximum = 780;
            this.curvechart.Series.Clear();
        }

        private Dictionary<String, Series> seriers_map = new Dictionary<string, Series>();

        public void displayData(String title, double[] y)
        {
            double[] x = new double[y.Length];
            for (int i = 0; i < x.Length; i++)
            {
                x[i] = i;
            }
            displayData(title, x, y);
        }

        public void Clear()
        {
            this.curvechart.Series.Clear();
            this.seriers_map.Clear();
        }

        int num = 0;
        public void displayData(String title, string devindex, double[] x, double[] y)
        {
            if (!seriers_map.ContainsKey(title)) BuildSeries(title);
            // 设置曲线的样式

            Series series = seriers_map[title];

            //清理旧数据
            series.Points.Clear();

            series.LegendText = title + "-" + devindex;
            ChartArea chartArea1 = this.curvechart.ChartAreas[0];
            if (x[0] < chartArea1.AxisX.Minimum) chartArea1.AxisX.Minimum = x[0];
            if (x[x.Length - 1] > chartArea1.AxisX.Maximum) chartArea1.AxisX.Maximum = x[x.Length - 1];

            // 在chart中显示数据
            for (int i = 0; i < x.Length; i++)
            {
                //添加坐标点
                series.Points.AddXY(x[i], y[i]);
            }

            num++;
            if (num % this.seriers_map.Values.Count == 0)
            {
                this.curvechart.Series.Clear();
                foreach (Series line in this.seriers_map.Values)
                {
                    this.curvechart.Series.Add(line);
                }
                this.curvechart.Update();
            }
            //Console.WriteLine(this.curvechart.Series.Count);
        }

        public void displayData(String title, double[] x, double[] y)
        {
            this.displayData(title, "", x, y);
            //this.curvechart.Update();
        }

        private void BuildSeries(string name)
        {
            Series line = new Series(name);
            // 画样条曲线（Spline）
            line.ChartType = SeriesChartType.FastLine;
            // 线宽2个像素
            line.BorderWidth = 1;

            seriers_map.Add(name, line);
            //this.curvechart.Series.Add(line);
        }

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
            this.curvechart.ChartAreas[0].AxisX.ScaleView.ZoomReset(0);
            this.curvechart.ChartAreas[0].AxisY.ScaleView.ZoomReset(0);
        }
        #endregion
    }
}
