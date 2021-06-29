// windemon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "screen_input.h"
#include "spectraltest.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


using namespace std; 
//*************************************************************
//主程序
//*************************************************************
class TestTable
{
	public:
		void (*fun)(void);
		char* fun_name; 

		TestTable(char* fnam, void (*f)(void))
		{
			fun = f;
			fun_name = fnam;
		}

};

TestTable fun_list[] ={
	TestTable("[1] 获取驱动版本：  (SA_GetAPIVersion)\n", &GetAPIVersion),
	TestTable("[2] 打开光谱仪：    (SA_OpenSpectrometers)\n", &OpenSpectrometers),
	TestTable("[3] 获取光谱仪名称：(SA_GetSpectrometersName)\n", &GetSpectrometersName),
	TestTable("[4] 设置积分时间：  (SA_SetIntegrationTime)\n", &SetIntegrationTime),
	TestTable("[5] 读取光谱：      (SA_GetSpectum)\n", &GetSpectum),
	TestTable("[6] 读取光谱(异步)：(SA_AsyGetSpectum)\n", &AsyGetSpectum),
	TestTable("[7] 多积分时间采集：(SA_MultiChanneTest)\n", &MultiChanneTest),
	TestTable("[8] 关闭光谱仪：    (SA_CloseSpectrometers)\n", &CloseSpectrometers),
};

//提示用户输入
void print_info()
{	
	printf("\n");
	int len = sizeof(fun_list)/sizeof(TestTable);
	for(int i = 0; i < len; i++)
	{
		printf(fun_list[i].fun_name);
	}
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
	
	int len = sizeof(fun_list)/sizeof(TestTable);
	if(cmd_num > 0 && cmd_num <= len)
	{
		fun_list[cmd_num - 1].fun();	
	}
}

int main(int argc, char *argv[])
{
	cout<<log(3.1/2)<<endl;
	initDll();
	while(last_cmd_num >= 0)
	{		
		print_info();			
		last_cmd_num = get_input_cmd();		
		input_cmd(last_cmd_num);				
	}

    return 0;
}
//*************************************************************
