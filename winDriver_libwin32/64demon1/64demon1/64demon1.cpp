// 64demon1.cpp : �������̨Ӧ�ó������ڵ㡣
//
// windemon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SpectraArsenal.h"
#include "stdio.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "simple_chart.h"

#pragma comment(lib, "SpectraArsenal.lib")
// �����Ļ
#define CLEAR() system("CLS")
using namespace std; 

static LONGLONG start = 0;
static LONGLONG tend = 0;
//*************************************************************
//��������
//*************************************************************
//�з��ַ�������ȡ�������
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

int readInputInt()
{
	int index;
	char input[100];
	cin.getline(input, 100, '\n');
	string s_input(input);
	stringstream ss;
	ss << s_input;
	ss >> index;
	return index;
}


double freq; 
void StartTime()
{
	LARGE_INTEGER largeint; 
		
	QueryPerformanceFrequency(&largeint); 
	freq = (double)largeint.QuadPart;   
	QueryPerformanceCounter(&largeint);   
	start = largeint.QuadPart;
}

double StopTime()
{
	LARGE_INTEGER largeint; 
	double TotTime;

	QueryPerformanceCounter(&largeint);   
	tend = largeint.QuadPart; 
	TotTime = (double)(tend - start) / freq; 
	TotTime = TotTime * 1000;
	return TotTime;
}
//*************************************************************

//*************************************************************
//�����ǲ�������
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
//1
void GetAPIVersion()
{
	char* ret = SA_GetAPIVersion();
	string version(ret);
	cout<<"��ǰ�汾"<<ret<<endl;
}

//2
void OpenSpectrometers()
{
	int ret = SA_OpenSpectrometers();

	if(ret >= 0){
		cout<< "�ҵ�������"<<ret + 1<< "̨"<<endl;
		cout<<"����򿪵��豸��(0-"<<ret<<"):"<<endl;
		int index = readInputInt();
		if(index < 0 || index > ret)
		{
			cout<<"��ʧ��!!������0-"<<ret<<endl;
			return ;
		}
		open_dev = index;
	}
	else
	{
		cout<<"û���ҵ�������,���أ�"<<ret<<endl;
	}

}

//3
void GetSpectrometersName()
{
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}

	char * dev_name = SA_GetSpectrometersName(open_dev);
	if(dev_name != NULL)
	{
		cout<<dev_name<<endl;
	}else{
		cout<<"��ȡ�豸����ʧ��"<<endl;
	}
}

//4
int timeout = 10;
void SetIntegrationTime ()
{
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}
	cout<<"�������ʱ��(ms):"<<endl;
	int time = readInputInt();
	int ret = SA_SetIntegrationTime(open_dev, time * 1000);
	if(ret == 0)
	{
		timeout = time;
		cout<<"���óɹ�!"<<endl;		
	}else{
		
		cout<<"����ʧ��!"<<ret<<endl;	
	}
}

//5
void GetSpectum(){
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}
		
	//��ʱ��
	StartTime();  
	//SA_SetSpectumTriggerMode(open_dev, SOFTWARE_SYNCHRONOUS);

	double data[3000];
	int spec_num;	 
	//ͬ������
	if(SA_GetSpectum(open_dev, data, spec_num) == 0)
	{		
		double TotTime = StopTime();

		cout<<"��ȡ�豸:1�������"<<spec_num<<"����ʱ��:"<<TotTime<<endl;
		print_chart(data, spec_num);
	}else{
		cout<<"�ɼ�ʧ��"<<endl;
	}
}

//6
void ScanStartAsyncSoftTrigger()
{
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}
	int state = 1;
	int timeout_index = 0;

	SA_ScanStartAsyncSoftTrigger(open_dev);
	while(state != 0)
	{
		//ȷ����ʱ����
		if(timeout_index ++ > timeout * 2)
		{
			cout<<"��ʱ"<<endl;
			return;
		}

        /*
		//��ѯ״̬0��ʾ������1��ʾ���֣�2��ʾ�ɼ�
		if(SA_GetStateAsyncSoftTrigger(open_dev, &state) != 0)
		{
			cout<<"�ɼ�ʧ��"<<endl;
			return;
		}else{
			cout<<"״̬:"<<state<<endl;
		}*/
		Sleep(1);
	}

	
	double data[3000];
	int spec_num;
	StartTime();
	//ȡ����
	if(SA_GetSpectum(open_dev, data, spec_num) == 0)
	{
		double TotTime = StopTime();

		cout<<"��ȡ�豸:1�������"<<spec_num<<"����ʱ��:"<<TotTime<<endl;
		print_chart(data, spec_num);
	}else{
		cout<<"�ɼ�ʧ��"<<endl;
	}
}

//7
void CloseSpectrometers()
{	
	SA_CloseSpectrometers();
	open_dev = -1;
}
//*************************************************************

//*************************************************************
//������
//*************************************************************
//��ʾ�û�����
void print_info()
{	
	printf("\n");
	printf("[1] ��ȡ�����汾��  (SA_GetAPIVersion)\n");
	printf("[2] �򿪹����ǣ�    (SA_OpenSpectrometers)\n");
	printf("[3] ��ȡ���������ƣ�(SA_GetSpectrometersName)\n");
	printf("[4] ���û���ʱ�䣺  (SA_SetIntegrationTime)\n");
	printf("[5] ��ȡ���ף�      (SA_GetSpectum)\n");
	printf("[6] ��ȡ����(�첽)��(SA_ScanStartAsyncSoftTrigger)\n");
	printf("[7] �رչ����ǣ�    (SA_CloseSpectrometers)\n");
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
//*************************************************************

int _tmain(int argc, _TCHAR* argv[])
{
	while(last_cmd_num >= 0)
	{	
		print_info();			
		last_cmd_num = get_input_cmd();		
		input_cmd(last_cmd_num);				
	}
    
    return 0;
}