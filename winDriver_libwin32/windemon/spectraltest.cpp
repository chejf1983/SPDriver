
#include "stdafx.h"
#include "SpectraArsenal.h"
//#include "SpectraArsenalII.h"
#include "stdio.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "simple_chart.h"
#include "screen_input.h"

#pragma comment(lib, "SpectraArsenal.lib")
using namespace std; 

/*
pSA_GetAPIVersion SA_GetAPIVersion;
pSA_OpenSpectrometers SA_OpenSpectrometers;
pSA_GetSpectrometersName SA_GetSpectrometersName;
pSA_SetIntegrationTime SA_SetIntegrationTime;
pSA_SetSpectumTriggerMode SA_SetSpectumTriggerMode;
pSA_GetSpectum SA_GetSpectum;
pSA_ScanStartAsyncSoftTrigger SA_ScanStartAsyncSoftTrigger;
pSA_GetStateAsyncSoftTrigger SA_GetStateAsyncSoftTrigger;
pSA_GetAsyncSoftSpectum SA_GetAsyncSoftSpectum;
pSA_SetMultiChannelIntegrationTime SA_SetMultiChannelIntegrationTime;
pSA_ScanStartMultiChannelAsyncSoftTrigger SA_ScanStartMultiChannelAsyncSoftTrigger;
pSA_GetMultiChannelAsyncSoftSpectum SA_GetMultiChannelAsyncSoftSpectum;
pSA_CloseSpectrometers SA_CloseSpectrometers;
pSA_GetSpectrometerPixelsNumber SA_GetSpectrometerPixelsNumber;
*/

void initDll()
{
	/*
	HMODULE hModule = LoadLibrary("SpectraArsenal.dll");
	SA_GetAPIVersion = (pSA_GetAPIVersion)GetProcAddress(hModule, "SA_GetAPIVersion");
	SA_OpenSpectrometers = (pSA_OpenSpectrometers)GetProcAddress(hModule, "SA_OpenSpectrometers");
	SA_GetSpectrometersName = (pSA_GetSpectrometersName)GetProcAddress(hModule, "SA_GetSpectrometersName");
	SA_SetIntegrationTime = (pSA_SetIntegrationTime)GetProcAddress(hModule, "SA_SetIntegrationTime");
	SA_SetSpectumTriggerMode = (pSA_SetSpectumTriggerMode)GetProcAddress(hModule, "SA_SetSpectumTriggerMode");
	SA_GetSpectum = (pSA_GetSpectum)GetProcAddress(hModule, "SA_GetSpectum");
	SA_ScanStartAsyncSoftTrigger = (pSA_ScanStartAsyncSoftTrigger)GetProcAddress(hModule, "SA_ScanStartAsyncSoftTrigger");
	SA_GetStateAsyncSoftTrigger = (pSA_GetStateAsyncSoftTrigger)GetProcAddress(hModule, "SA_GetStateAsyncSoftTrigger");
	SA_GetAsyncSoftSpectum = (pSA_GetAsyncSoftSpectum)GetProcAddress(hModule, "SA_GetAsyncSoftSpectum");
	SA_SetMultiChannelIntegrationTime = (pSA_SetMultiChannelIntegrationTime)GetProcAddress(hModule, "SA_SetMultiChannelIntegrationTime");
	SA_ScanStartMultiChannelAsyncSoftTrigger = (pSA_ScanStartMultiChannelAsyncSoftTrigger)GetProcAddress(hModule, "SA_ScanStartMultiChannelAsyncSoftTrigger");
	SA_GetMultiChannelAsyncSoftSpectum = (pSA_GetMultiChannelAsyncSoftSpectum)GetProcAddress(hModule, "SA_GetMultiChannelAsyncSoftSpectum");
	SA_CloseSpectrometers = (pSA_CloseSpectrometers)GetProcAddress(hModule, "SA_CloseSpectrometers");
	SA_GetSpectrometerPixelsNumber = (pSA_GetSpectrometerPixelsNumber)GetProcAddress(hModule, "SA_GetSpectrometerPixelsNumber");*/
	
}

int open_dev = -1;
//*************************************************************
//�����ǲ�������
//*************************************************************
//��ȡAPI�汾��
void GetAPIVersion()
{
	char* ret = SA_GetAPIVersion();
	string version(ret);
	cout<<"��ǰ�汾"<<ret<<endl;
}

//�򿪹�����
void OpenSpectrometers()
{
	//�򿪹�����
	int ret = SA_OpenSpectrometers();

	if(ret >= 0){
		cout<< "�ҵ�������"<<ret + 1<< "̨"<<endl;
		cout<<"����򿪵��豸��(1->"<<ret + 1<<"):"<<endl;
		//��ȡ��������
		int index = readInputInt();
		//����������
		if(index <= 0 || index > ret + 1)
		{
			cout<<"��ʧ��!!������(1-"<<ret + 1<<"):"<<endl;
			return ;
		}
		//���ô򿪵ı��
		open_dev = index - 1;
	}
	else
	{
		cout<<"û���ҵ�������,���أ�"<<ret<<endl;
	}

}

//��ȡ����������
void GetSpectrometersName()
{
	//���������Ƿ��
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}

	//��ȡ����������
	char * dev_name = SA_GetSpectrometersName(open_dev);
	if(dev_name != NULL)
	{
		cout<<dev_name<<"�����"<<SA_GetSpectrometerPixelsNumber(open_dev)<<endl;
	}else{
		cout<<"��ȡ�豸����ʧ��"<<endl;
	}

	
}

//���û���ʱ��
int timeout = 10;
void SetIntegrationTime ()
{
	//���������Ƿ��
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}

	cout<<"�������ʱ��(ms):"<<endl;
	//��ȡ�������ʱ��ms
	int time = readInputInt();
	//���û���ʱ��(ns)
	int ret = SA_SetIntegrationTime(open_dev, time * 1000);
	if(ret == 0)
	{
		timeout = time;
		cout<<"���óɹ�!"<<endl;		
	}else{
		
		cout<<"����ʧ��!"<<ret<<endl;	
	}
}

/* ��ͨ�����ͬ������--��ȡ���� */
void GetSpectum(){
	//���������Ƿ��
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}
		
	//��ʼ��ʱ��
	ClickTime();  
	//�����������ģʽ
	SA_SetSpectumTriggerMode(open_dev, SOFTWARE_SYNCHRONOUS);

	//��ʼ�����ݻ���
	double data[3000];
	//���ص����
	int spec_num;	 
	//ͬ������
	if(SA_GetSpectum(open_dev, data, spec_num) == 0)
	{		
		//�����Ϣ
		cout<<"��ȡ�豸:1�������"<<data[spec_num-1]<<"����ʱ��:"<<ClickTime()<<endl;
		//��ӡ����
		print_chart(data, spec_num);
	}else{
		cout<<"�ɼ�ʧ��"<<endl;
	}
}

/* ��ͨ������첽����--�첽������״̬��ѯ����ȡ����*/
void AsyGetSpectum()
{
	//���������Ƿ��
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}
	int state = 1;
	int timeout_index = 0;

	ClickTime();
	//���ÿ�ʼ�첽�ɼ�
	SA_ScanStartAsyncSoftTrigger(open_dev);
	while(state != 0)
	{
		//ȷ����ʱ����
		if(timeout_index ++ > timeout * 2)
		{
			cout<<"��ʱ"<<endl;
			return;
		}

		//��ѯ״̬0��ʾ������1��ʾ���֣�2��ʾ�ɼ�
		if(SA_GetStateAsyncSoftTrigger(open_dev, &state) != 0)
		{
			cout<<"��ȡ״̬ʧ��"<<endl;
			return;
		}else{
			cout<<"״̬:"<<state<<"ʱ��:"<<ClickTime()<<endl;
		}

		Sleep(1);
	}

	
	double data[3000];
	int spec_num;
	//�첽��ȡ����
	if(SA_GetAsyncSoftSpectum(open_dev, data, spec_num) == 0)
	{
		cout<<"��ȡ�豸:1�������"<<spec_num<<"����ʱ��:"<<ClickTime()<<endl;
		//��ӡ����
		print_chart(data, spec_num);
	}else{
		cout<<"�ɼ�ʧ��"<<endl;
	}
}

//�رչ�����
void CloseSpectrometers()
{	
	SA_CloseSpectrometers();
	open_dev = -1;
}

//��ͨ���ɼ�����
void MultiChanneTest()
{
	if(open_dev < 0)
	{
		cout<<"���ȴ򿪹�����"<<endl;
		return;
	}
	int usec[16] = {0};
	for(int i = 0; i < 16; i++)usec[i] = 100;
	usec[0] = 10000;
	usec[1] = 100000;

	//��ʱ��
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
		//ȷ����ʱ����
		if(timeout_index ++ > timeout * 2)
		{
			cout<<"��ʱ"<<endl;
			return;
		}

		//��ѯ״̬0��ʾ������1��ʾ���֣�2��ʾ�ɼ�
		if(SA_GetStateAsyncSoftTrigger(open_dev, &state) != 0)
		{
			cout<<"�ɼ�ʧ��"<<endl;
			return;
		}else{
			cout<<"״̬:"<<state<<endl;
		}
		Sleep(100);
	}

	
	if(SA_SetSpectumTriggerMode(open_dev, SOFTWARE_SYNCHRONOUS) != 0)
	{
		cout<<"SA_SetSpectumTriggerMode"<<" failed"<<endl;
	}

	double data[3000];
	int spec_num;	
	
	//��ʱ��
	ClickTime(); 
	//����1
	if(SA_GetMultiChannelAsyncSoftSpectum(open_dev, data, spec_num) == 0)
	{		
		cout<<"��ȡ�豸:1���1����"<<spec_num<<"����ʱ��:"<<ClickTime()<<endl;
		print_chart(data, spec_num);
	}else{
		cout<<"�ɼ�ʧ��"<<endl;
	}
}