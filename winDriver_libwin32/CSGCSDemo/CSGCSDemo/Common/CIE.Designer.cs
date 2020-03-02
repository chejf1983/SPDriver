namespace CSGCSDemo.Common
{
    partial class CIE
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.picture_chart = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.picture_chart)).BeginInit();
            this.SuspendLayout();
            // 
            // picture_chart
            // 
            this.picture_chart.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.picture_chart.Location = new System.Drawing.Point(12, 12);
            this.picture_chart.Name = "picture_chart";
            this.picture_chart.Size = new System.Drawing.Size(441, 406);
            this.picture_chart.TabIndex = 0;
            this.picture_chart.TabStop = false;
            this.picture_chart.Paint += new System.Windows.Forms.PaintEventHandler(this.picture_chart_Paint);
            this.picture_chart.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picture_chart_MouseDown);
            this.picture_chart.MouseMove += new System.Windows.Forms.MouseEventHandler(this.picture_chart_MouseMove);
            this.picture_chart.MouseUp += new System.Windows.Forms.MouseEventHandler(this.picture_chart_MouseUp);
            // 
            // CIE
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(465, 430);
            this.Controls.Add(this.picture_chart);
            this.Name = "CIE";
            this.Text = "CIE";
            ((System.ComponentModel.ISupportInitialize)(this.picture_chart)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox picture_chart;
    }
}