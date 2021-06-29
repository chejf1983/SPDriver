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
//������
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
	TestTable("[1] ��ȡ�����汾��  (SA_GetAPIVersion)\n", &GetAPIVersion),
	TestTable("[2] �򿪹����ǣ�    (SA_OpenSpectrometers)\n", &OpenSpectrometers),
	TestTable("[3] ��ȡ���������ƣ�(SA_GetSpectrometersName)\n", &GetSpectrometersName),
	TestTable("[4] ���û���ʱ�䣺  (SA_SetIntegrationTime)\n", &SetIntegrationTime),
	TestTable("[5] ��ȡ���ף�      (SA_GetSpectum)\n", &GetSpectum),
	TestTable("[6] ��ȡ����(�첽)��(SA_AsyGetSpectum)\n", &AsyGetSpectum),
	TestTable("[7] �����ʱ��ɼ���(SA_MultiChanneTest)\n", &MultiChanneTest),
	TestTable("[8] �رչ����ǣ�    (SA_CloseSpectrometers)\n", &CloseSpectrometers),
};

//��ʾ�û�����
void print_info()
{	
	printf("\n");
	int len = sizeof(fun_list)/sizeof(TestTable);
	for(int i = 0; i < len; i++)
	{
		printf(fun_list[i].fun_name);
	}
	printf("�������������,����س�,�ظ�ִ����һ������,����'exit'�˳�����\n");
}

int last_cmd_num = 0;
//��ȡ��������
int get_input_cmd()
{
	char input[100];
	cin.getline(input, 100, '\n');
	string s_input(input);
	//����Ƿ����˳�ָ��
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

//��ȡ�û�����,���ò���
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
