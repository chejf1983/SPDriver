﻿namespace CSGCSDemo.Common
{
    partial class spchart
    {
        /// <summary> 
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region 组件设计器生成的代码

        /// <summary> 
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend1 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            this.curvechart = new System.Windows.Forms.DataVisualization.Charting.Chart();
            ((System.ComponentModel.ISupportInitialize)(this.curvechart)).BeginInit();
            this.SuspendLayout();
            // 
            // curvechart
            // 
            this.curvechart.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.curvechart.BackColor = System.Drawing.SystemColors.ActiveBorder;
            chartArea1.Name = "ChartArea1";
            chartArea1.ShadowOffset = 2;
            this.curvechart.ChartAreas.Add(chartArea1);
            legend1.Name = "Legend1";
            legend1.ShadowOffset = 2;
            this.curvechart.Legends.Add(legend1);
            this.curvechart.Location = new System.Drawing.Point(0, 0);
            this.curvechart.Name = "curvechart";
            series1.ChartArea = "ChartArea1";
            series1.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.FastLine;
            series1.Legend = "Legend1";
            series1.Name = "Series1";
            this.curvechart.Series.Add(series1);
            this.curvechart.Size = new System.Drawing.Size(454, 396);
            this.curvechart.TabIndex = 0;
            this.curvechart.Text = "chart1";
            this.curvechart.MouseDown += new System.Windows.Forms.MouseEventHandler(this.curvechart_MouseDown);
            this.curvechart.MouseUp += new System.Windows.Forms.MouseEventHandler(this.curvechart_MouseUp);
            // 
            // spchart
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.curvechart);
            this.Name = "spchart";
            this.Size = new System.Drawing.Size(454, 396);
            ((System.ComponentModel.ISupportInitialize)(this.curvechart)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataVisualization.Charting.Chart curvechart;
    }
}
