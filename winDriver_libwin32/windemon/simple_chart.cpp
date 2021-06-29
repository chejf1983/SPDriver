
#include "stdio.h"
#include <stdlib.h>
#include "stdafx.h"
#include <iostream>
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
const int row_max = 15;//表格最大行数
const int column_max = 70;//表格最大列数
char chart [column_max][row_max] = {0};//表格
int y_axis[row_max] = {0};//y轴
int x_axis[column_max] = {0};//x轴


//*************************************************************
//  初始化y轴 
//  min y轴最小值
//  gap y轴间隔
//*************************************************************
void init_yaxis(double min, int gap)
{
	for(int i = 0; i < row_max; i++)
	{
		y_axis[i] = (int)min + gap * i;
	}
}

//*************************************************************
//  初始化x轴 
//  x[] x轴值
//*************************************************************
void init_xaxis(double x[])
{
	for(int i = 0; i < column_max; i++)
	{
		x_axis[i] = (int)x[i];
	}
}

//转换数据 
void convert_data(double ora_x[], double ora_y[], int ora_len, double x[], int y[], int& len){
	int i;
	
	//寻找y方向最大最小值 
	double min = 0;
	double max = 0;
	for(i = 0; i < ora_len; i++){
		if(ora_y[i] > max) max = ora_y[i];
		if(ora_y[i] < min){ min = ora_y[i];};
	}
	//计算y间隔 
	max += max * 0.1;
	double y_jump = (max - min) / row_max;
	//初始化y轴
	init_yaxis(min, (int)y_jump);
	
	len = column_max;
	int x_jump = ora_len / column_max;
	//转换数据
	int new_index = 0;
	for(i = 0; i < ora_len;i++){
		if(i % x_jump == 0){
			x[new_index] = ora_x[i];
			y[new_index] = (int)((ora_y[i] - min) / y_jump);
			new_index ++;
		}
	}
	//初始化x轴
	init_xaxis(x);
}

//初始化空白表格
void init_table()
{
	for(int row = row_max - 1; row >= 0; row--){
		for(int col = 0; col < column_max; col++){
			chart[col][row] = ' '; 
		} 
	}
}

//画曲线
void draw_line(double x[], int y[], int len){
	for(int i = 0; i < len; i++){
	    //printf("%f:%d \n",x[i],y[i]);
		chart[i][y[i]] = '*';
	}
}

void print_axis(int avr_num){
	int i;
	//打印x轴
	printf("      ");
	for(i = 0; i < column_max; i++)
	{
		printf("-");
	}
	printf("\n");
	
	printf("      ");
	int lag = column_max / avr_num;
	for(i = 0; i < column_max; i++)
	{
		if(i % lag == 0){
			printf("%-10d", x_axis[i]);			
			i += 10;
		}else{
			printf(" ");
		}
	}
	printf("\n");
}

//画表格
void draw_table(){
	for(int row = row_max - 1; row >= 0; row--){		
		//打印y轴
		printf("%5d|", y_axis[row]);
		for(int col = 0; col < column_max; col++){
			printf("%c", chart[col][row]); 
		} 
		printf("\n", y_axis[row]);
	}
	
	print_axis(4);
} 

void print_chart(double x[], double y[], int data_len)
{
	double new_x[100];
	int new_y[100];
	int new_len = 0;
	
	//初始化空白表格
	init_table();
	
	//转换数据
	convert_data(x, y, data_len, new_x, new_y, new_len);
		
	draw_line(new_x, new_y, new_len);
	
	draw_table();
	
}

void print_chart(double y[], int data_len)
{
	double x[3000];
	for(int i = 0; i < data_len; i++)
	{
		x[i] = i;
	}
	print_chart(x, y, data_len);
}

