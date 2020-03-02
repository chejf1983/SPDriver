namespace CSGCSDemo
{
    partial class MainForm
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
            this.bt_search = new System.Windows.Forms.Button();
            this.bt_collect = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.tb_intertime = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.tb_intervaltime = new System.Windows.Forms.TextBox();
            this.clst_devs = new System.Windows.Forms.CheckedListBox();
            this.label5 = new System.Windows.Forms.Label();
            this.lb_num = new System.Windows.Forms.Label();
            this.bt_dk = new System.Windows.Forms.Button();
            this.sp_chart = new CSGCSDemo.Common.spchart();
            this.button_cie = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // bt_search
            // 
            this.bt_search.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.bt_search.Location = new System.Drawing.Point(12, 555);
            this.bt_search.Name = "bt_search";
            this.bt_search.Size = new System.Drawing.Size(187, 23);
            this.bt_search.TabIndex = 1;
            this.bt_search.Text = "搜索设备";
            this.bt_search.UseVisualStyleBackColor = true;
            this.bt_search.Click += new System.EventHandler(this.bt_search_Click);
            // 
            // bt_collect
            // 
            this.bt_collect.Location = new System.Drawing.Point(286, 12);
            this.bt_collect.Name = "bt_collect";
            this.bt_collect.Size = new System.Drawing.Size(75, 23);
            this.bt_collect.TabIndex = 2;
            this.bt_collect.Text = "连续采集";
            this.bt_collect.UseVisualStyleBackColor = true;
            this.bt_collect.Click += new System.EventHandler(this.bt_collect_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(367, 12);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 2;
            this.button1.Text = "停止";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(448, 17);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(59, 12);
            this.label1.TabIndex = 3;
            this.label1.Text = "积分时间:";
            // 
            // tb_intertime
            // 
            this.tb_intertime.Location = new System.Drawing.Point(513, 14);
            this.tb_intertime.Name = "tb_intertime";
            this.tb_intertime.Size = new System.Drawing.Size(100, 21);
            this.tb_intertime.TabIndex = 4;
            this.tb_intertime.Text = "10";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(619, 17);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(17, 12);
            this.label2.TabIndex = 3;
            this.label2.Text = "ms";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(641, 17);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(59, 12);
            this.label3.TabIndex = 3;
            this.label3.Text = "采样间隔:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(812, 17);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(17, 12);
            this.label4.TabIndex = 3;
            this.label4.Text = "ms";
            // 
            // tb_intervaltime
            // 
            this.tb_intervaltime.Location = new System.Drawing.Point(706, 14);
            this.tb_intervaltime.Name = "tb_intervaltime";
            this.tb_intervaltime.Size = new System.Drawing.Size(100, 21);
            this.tb_intervaltime.TabIndex = 4;
            this.tb_intervaltime.Text = "50";
            // 
            // clst_devs
            // 
            this.clst_devs.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.clst_devs.FormattingEnabled = true;
            this.clst_devs.Location = new System.Drawing.Point(12, 12);
            this.clst_devs.Name = "clst_devs";
            this.clst_devs.Size = new System.Drawing.Size(187, 532);
            this.clst_devs.TabIndex = 8;
            this.clst_devs.SelectedValueChanged += new System.EventHandler(this.clst_devs_SelectedValueChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(844, 17);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(59, 12);
            this.label5.TabIndex = 10;
            this.label5.Text = "采集次数:";
            // 
            // lb_num
            // 
            this.lb_num.AutoSize = true;
            this.lb_num.Location = new System.Drawing.Point(909, 17);
            this.lb_num.Name = "lb_num";
            this.lb_num.Size = new System.Drawing.Size(11, 12);
            this.lb_num.TabIndex = 10;
            this.lb_num.Text = "0";
            // 
            // bt_dk
            // 
            this.bt_dk.Location = new System.Drawing.Point(205, 12);
            this.bt_dk.Name = "bt_dk";
            this.bt_dk.Size = new System.Drawing.Size(75, 23);
            this.bt_dk.TabIndex = 2;
            this.bt_dk.Text = "扣暗电流";
            this.bt_dk.UseVisualStyleBackColor = true;
            this.bt_dk.Click += new System.EventHandler(this.bt_dk_Click);
            // 
            // sp_chart
            // 
            this.sp_chart.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.sp_chart.Location = new System.Drawing.Point(205, 41);
            this.sp_chart.Name = "sp_chart";
            this.sp_chart.Size = new System.Drawing.Size(1071, 537);
            this.sp_chart.TabIndex = 7;
            // 
            // button_cie
            // 
            this.button_cie.Location = new System.Drawing.Point(974, 12);
            this.button_cie.Name = "button_cie";
            this.button_cie.Size = new System.Drawing.Size(75, 23);
            this.button_cie.TabIndex = 11;
            this.button_cie.Text = "CIE";
            this.button_cie.UseVisualStyleBackColor = true;
            this.button_cie.Click += new System.EventHandler(this.button_cie_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1288, 590);
            this.Controls.Add(this.button_cie);
            this.Controls.Add(this.lb_num);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.clst_devs);
            this.Controls.Add(this.sp_chart);
            this.Controls.Add(this.tb_intervaltime);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.tb_intertime);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.bt_dk);
            this.Controls.Add(this.bt_collect);
            this.Controls.Add(this.bt_search);
            this.Name = "MainForm";
            this.Text = "MainForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button bt_search;
        private System.Windows.Forms.Button bt_collect;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tb_intertime;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox tb_intervaltime;
        private System.Windows.Forms.CheckedListBox clst_devs;
        private Common.spchart sp_chart;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label lb_num;
        private System.Windows.Forms.Button bt_dk;
        private System.Windows.Forms.Button button_cie;
    }
}