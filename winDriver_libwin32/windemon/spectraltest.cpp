
#include "stdafx.h"
#include "SpectraArsenal.h"
#include "stdio.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "simple_chart.h"
#include "screen_input.h"

#pragma comment(lib, "SpectraArsenal.lib")
using namespace std; 

int open_dev = -1;
//*************************************************************
//光谱仪操作函数
//*************************************************************
//获取API版本号
void GetAPIVersion()
{
	char* ret = SA_GetAPIVersion();
	string version(ret);
	cout<<"当前版本"<<ret<<endl;
}

//打开光谱仪
void OpenSpectrometers()
{
	//打开光谱仪
	int ret = SA_OpenSpectrometers();

	if(ret >= 0){
		cout<< "找到光谱仪"<<ret + 1<< "台"<<endl;
		cout<<"输入打开的设备号(1->"<<ret + 1<<"):"<<endl;
		//读取输入的序号
		int index = readInputInt();
		//检查输入序号
		if(index <= 0 || index > ret + 1)
		{
			cout<<"打开失败!!请出入从(1-"<<ret + 1<<"):"<<endl;
			return ;
		}
		//设置打开的编号
		open_dev = index - 1;
	}
	else
	{
		cout<<"没有找到光谱仪,返回："<<ret<<endl;
	}

}

//获取光谱仪名称
void GetSpectrometersName()
{
	//检查光谱仪是否打开
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}

	//读取光谱仪名称
	char * dev_name = SA_GetSpectrometersName(open_dev);
	if(dev_name != NULL)
	{
		cout<<dev_name<<endl;
	}else{
		cout<<"读取设备名称失败"<<endl;
	}
}

//设置积分时间
int timeout = 10;
void SetIntegrationTime ()
{
	//检查光谱仪是否打开
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}

	cout<<"输入积分时间(ms):"<<endl;
	//读取输入积分时间ms
	int time = readInputInt();
	//设置积分时间(ns)
	int ret = SA_SetIntegrationTime(open_dev, time * 1000);
	if(ret == 0)
	{
		timeout = time;
		cout<<"设置成功!"<<endl;		
	}else{
		
		cout<<"设置失败!"<<ret<<endl;	
	}
}

/* 单通道软件同步操作--获取光谱 */
void GetSpectum(){
	//检查光谱仪是否打开
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}
		
	//开始计时间
	ClickTime();  
	//设置软件出发模式
	SA_SetSpectumTriggerMode(open_dev, SOFTWARE_SYNCHRONOUS);

	//初始化数据缓存
	double data[3000];
	//像素点个数
	int spec_num;	 
	//同步采样
	if(SA_GetSpectum(open_dev, data, spec_num) == 0)
	{		
		//输出信息
		cout<<"获取设备:1获得数据"<<spec_num<<"个，时间:"<<ClickTime()<<endl;
		//打印曲线
		print_chart(data, spec_num);
	}else{
		cout<<"采集失败"<<endl;
	}
}

/* 单通道软件异步操作--异步触发、状态查询、获取光谱*/
void AsyGetSpectum()
{
	//检查光谱仪是否打开
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}
	int state = 1;
	int timeout_index = 0;

	ClickTime();
	//设置开始异步采集
	SA_ScanStartAsyncSoftTrigger(open_dev);
	while(state != 0)
	{
		//确保超时保护
		if(timeout_index ++ > timeout * 2)
		{
			cout<<"超时"<<endl;
			return;
		}

		//查询状态0表示结束，1表示积分，2表示采集
		if(SA_GetStateAsyncSoftTrigger(open_dev, &state) != 0)
		{
			cout<<"读取状态失败"<<endl;
			return;
		}else{
			cout<<"状态:"<<state<<"时间:"<<ClickTime()<<endl;
		}

		Sleep(1);
	}

	
	double data[3000];
	int spec_num;
	//异步获取光谱
	if(SA_GetAsyncSoftSpectum(open_dev, data, spec_num) == 0)
	{
		cout<<"获取设备:1获得数据"<<spec_num<<"个，时间:"<<ClickTime()<<endl;
		//打印曲线
		print_chart(data, spec_num);
	}else{
		cout<<"采集失败"<<endl;
	}
}

//关闭光谱仪
void CloseSpectrometers()
{	
	SA_CloseSpectrometers();
	open_dev = -1;
}

//多通道采集光谱
void MultiChanneTest()
{
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}
	int usec[16] = {0};
	for(int i = 0; i < 16; i++)usec[i] = 100;
	usec[0] = 10000;
	usec[1] = 100000;

	//计时间
	ClickTime();  
    if(SA_SetMultiChannelIntegrationTime(open_dev, usec) != 0)
	{
		cout<<"SA_SetMultiChannelIntegrationTime"<<" failed"<<endl;
	}

	if(SA_ScanStartMultiChannelAsyncSoftTrigger(open_dev, 1) != 0)
	{
		cout<<"SA_SetMultiChannelIntegrationTime"<<" failed"<<endl;
	}



	int state = 1;
	int timeout_index = 0;
	while(state != 0)
	{
		//确保超时保护
		if(timeout_index ++ > timeout * 2)
		{
			cout<<"超时"<<endl;
			return;
		}

		//查询状态0表示结束，1表示积分，2表示采集
		if(SA_GetStateAsyncSoftTrigger(open_dev, &state) != 0)
		{
			cout<<"采集失败"<<endl;
			return;
		}else{
			cout<<"状态:"<<state<<endl;
		}
		Sleep(100);
	}

	
	if(SA_SetSpectumTriggerMode(open_dev, SOFTWARE_SYNCHRONOUS) != 0)
	{
		cout<<"SA_SetSpectumTriggerMode"<<" failed"<<endl;
	}

	double data[3000];
	int spec_num;	
	
	//计时间
	ClickTime(); 
	//采样1
	if(SA_GetMultiChannelAsyncSoftSpectum(open_dev, data, spec_num) == 0)
	{		
		cout<<"获取设备:1获得1数据"<<spec_num<<"个，时间:"<<ClickTime()<<endl;
		print_chart(data, spec_num);
	}else{
		cout<<"采集失败"<<endl;
	}
}