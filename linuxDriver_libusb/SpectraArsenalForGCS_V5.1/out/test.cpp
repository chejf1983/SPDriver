#include "SpectraArsenal.h"
#include "stdio.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "simple_chart.h"
#include <sys/time.h>
#include <unistd.h>

// 清除屏幕
#define CLEAR() system("clear")
using namespace std; 

//*************************************************************
//公共函数
//*************************************************************
//切分字符串，获取多个输入
void split(const string& s, const string& delim,vector<string> &elems)  
{  
    size_t pos = 0;  
    size_t len = s.length();  
    size_t delim_len = delim.length();  
    if (delim_len == 0) return;  
	
    while (pos < len)  
    {  
        int find_pos = s.find(delim, pos);  
        if (find_pos < 0)  
        {  
            elems.push_back(s.substr(pos, len - pos)); 
            break;  
        }  
        elems.push_back(s.substr(pos, find_pos - pos));  
        pos = find_pos + delim_len; 
    }

    return;
} 

//*************************************************************

//*************************************************************
//光谱仪操作函数
//*************************************************************
/*
char* SA_GetAPIVersion(void);
int SA_OpenSpectrometers(void);
char * SA_GetSpectrometersName(int spectrometerIndex);
int SA_SetIntegrationTime (int spectrometerIndex, int usec);
int SA_GetSpectum(int spectrometerIndex, double *pdSpectumData, int *pSpectumNumber);
void SA_CloseSpectrometers(void);
*/
int open_dev = -1;
void GetAPIVersion()
{
	char* ret = SA_GetAPIVersion();
	string version(ret);
	cout<<"当前版本"<<ret<<endl;
}

void OpenSpectrometers()
{
	int ret = SA_OpenSpectrometers();

	if(ret >= 0){
		cout<< "找到光谱仪"<<ret + 1<< "台"<<endl;
		cout<<"输入打开的设备号(0-"<<ret<<"):"<<endl;
		int index;
		char input[100];
		cin.getline(input, 100, '\n');
		string s_input(input);
		stringstream ss;
		ss << s_input;
		ss >> index;
		if(index < 0 || index > ret)
		{
			cout<<"打开失败!!请出入从0-"<<ret<<endl;
			return ;
		}
		open_dev = index;
	}
	else
	{
		cout<<"没有找到光谱仪,返回："<<ret<<endl;
	}

}

void GetSpectrometersName()
{
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}
	char * dev_name = SA_GetSpectrometersName(open_dev);
	if(dev_name != NULL)
	{
		cout<<dev_name<<endl;
	}else{
		cout<<"读取设备名称失败"<<endl;
	}
}

int timeout = 10;
void SetIntegrationTime ()
{
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}
	cout<<"输入积分时间(ms):"<<endl;
	int time;
	char input[100];
	cin.getline(input, 100, '\n');
	string s_input(input);
	stringstream ss;
	ss << s_input;
	ss >> time;
	int ret = SA_SetIntegrationTime(open_dev, time * 1000);
	if(ret == 0)
	{
		timeout = time;
		cout<<"设置成功!"<<endl;		
	}else{
		
		cout<<"设置失败!"<<ret<<endl;	
	}
}

void GetSpectum(){
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}
	double data[3000];
	int spec_num;
	struct timeval start, end;
    gettimeofday(&start,NULL);
	if(SA_GetSpectum(open_dev, data, &spec_num) == 0)
	{		
		gettimeofday(&end,NULL);
		cout<<"获取设备:1获得数据"<<spec_num<<"个,时间："<<
			((double)(end.tv_sec- start.tv_sec)*1000 + (double)(end.tv_usec- start.tv_usec)/1000)
			<<"ms"<<endl;
		print_chart(data, spec_num);
	}else{
		cout<<"采集失败"<<endl;
	}
}

void ScanStartAsyncSoftTrigger()
{
	if(open_dev < 0)
	{
		cout<<"请先打开光谱仪"<<endl;
		return;
	}
	double data[3000];
	int spec_num;
	int state = 1;
	int timeout_index = 0;
	struct timeval start, end;
	
		
    gettimeofday(&start,NULL);

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
			cout<<"采集失败"<<endl;
			return;
		}else{
			cout<<"状态:"<<state<<endl;
		}
		usleep(1000);
	}

	//取光谱
	if(SA_GetSpectum(open_dev, data, &spec_num) == 0)
	{		
		gettimeofday(&end,NULL);

		cout<<"获取设备:1获得数据"<<spec_num<<"个，时间:"<<
		((double)(end.tv_sec- start.tv_sec)*1000 + (double)(end.tv_usec- start.tv_usec)/1000)
		<<endl;
		print_chart(data, spec_num);
	}else{
		cout<<"采集失败"<<endl;
	}
}
void CloseSpectrometers()
{	
	SA_CloseSpectrometers();
	open_dev = -1;
}
//*************************************************************

//*************************************************************
//主程序
//*************************************************************
//提示用户输入
void print_info()
{	
	printf("\n");
	printf("[1] 获取驱动版本：  (SA_GetAPIVersion)\n");
	printf("[2] 打开光谱仪：    (SA_OpenSpectrometers)\n");
	printf("[3] 获取光谱仪名称：(SA_GetSpectrometersName)\n");
	printf("[4] 设置积分时间：  (SA_SetIntegrationTime)\n");
	printf("[5] 读取光谱：      (SA_GetSpectum)\n");
	printf("[6] 读取光谱(异步)：(SA_ScanStartAsyncSoftTrigger)\n");
	printf("[7] 关闭光谱仪：    (SA_CloseSpectrometers)\n");
	printf("请输入操作命令,输入回撤,重复执行上一次命令,输入'exit'退出程序\n");
}

int last_cmd_num = 0;
//获取输入命令
int get_input_cmd()
{
	char input[100];
	cin.getline(input, 100, '\n');
	string s_input(input);
	//检查是否是退出指令
	if(s_input.compare(0,3, "exit", 0,3) == 0)
	{
		return -1;
	}
	else if(s_input.empty()){
		return last_cmd_num;
	}
	else
	{
		int cmd = 0;
		stringstream ss;
		ss << s_input;
		ss >> cmd;
		return cmd;
	}
}

//获取用户输入,调用测试
void input_cmd(int cmd_num)
{
	CLEAR();
	printf("\n");
	switch(cmd_num)
	{
		case 1:
			GetAPIVersion();
			break;
		case 2:
			OpenSpectrometers();
			break;
		case 3:
			GetSpectrometersName();
			break;
		case 4:
			SetIntegrationTime();
			break;
		case 5:
			GetSpectum();
			break;
		case 6:
			ScanStartAsyncSoftTrigger();
			break;
		case 7:
			CloseSpectrometers();
			break;
		default:			
			cout<<cmd_num<<endl;
	}	
}

int main(int argc, char *argv[])
{
	while(last_cmd_num >= 0)
	{		
		print_info();			
		last_cmd_num = get_input_cmd();		
		input_cmd(last_cmd_num);				
	}

    return 0;
}
//*************************************************************