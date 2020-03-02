using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace CSGCSDemo.Common
{
    public partial class CIE : Form
    {
        public CIE()
        {
            InitializeComponent();

            initPictureBox();
        }

        private void initPictureBox()
        {
            this.picture_chart.LoadAsync(@"./CIE_1931.png");
            //this.picture_chart.Image = @"./CIE_1931.png";
            this.picture_chart.SizeMode = PictureBoxSizeMode.StretchImage;
        }

        Point M_pot_p = new Point();//原始位置
        int M_int_mx = 0, M_int_my = 0;//下次能继续
        int M_int_maxX, M_int_maxY;//加快读取用
        bool isMove = false;


        private void picture_chart_MouseDown(object sender, MouseEventArgs e)
        {
            if (!isMove && picture_chart.Image != null)
            {
                M_pot_p = e.Location;
                M_int_maxX = picture_chart.Width - picture_chart.Image.Width;
                M_int_maxY = picture_chart.Height - picture_chart.Image.Height;
                Cursor = Cursors.Hand;
                isMove = true;
            }
            else
            {
                MessageBox.Show("没有云图", "提示", MessageBoxButtons.OK, MessageBoxIcon.Question);
            }


        }

        private void picture_chart_MouseMove(object sender, MouseEventArgs e)
        {
            if (isMove)//当按左键的时候
            {
                lock (this)
                {
                    //算差值
                    M_int_mx = M_int_mx - M_pot_p.X + e.X;
                    M_int_my = M_int_my - M_pot_p.Y + e.Y;
                    //锁定范围
                    M_int_mx = Math.Min(0, Math.Max(M_int_maxX, M_int_mx));
                    M_int_my = Math.Min(0, Math.Max(M_int_maxY, M_int_my));

                    picture_chart.Refresh();
                    M_pot_p = e.Location;
                }
            }
            else
            {
                Cursor = Cursors.Default;
            }

        }

        private void picture_chart_MouseUp(object sender, MouseEventArgs e)
        {
            if (isMove)
                isMove = false;

        }

        private void picture_chart_Paint(object sender, PaintEventArgs e)
        {
            if (picture_chart.Image != null)
            {
                e.Graphics.DrawImage(picture_chart.Image, new Rectangle(0, 0, picture_chart.Width, picture_chart.Height), new Rectangle(-M_int_mx, -M_int_my, picture_chart.Width, picture_chart.Height), GraphicsUnit.Pixel);
            }

        }
    }
}
