#include "stdafx.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "screen_input.h"
#include <stdlib.h>
#include <string.h>


using namespace std; 

//*************************************************************
//�������� string& s ����  string &delim �ָ���� vector<string> $elems�ָ���
//*************************************************************
//�з��ַ�������ȡ�������
void split(const string& s, const string& delim, vector<string> &elems)  
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

//��ȡ��Ļ���������
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


LONGLONG start = 0,end = 0;   
//��ʱ
double ClickTime()
{
	LARGE_INTEGER largeint; 
	//����Ƶ��
	QueryPerformanceFrequency(&largeint); 
	double freq = (double)largeint.QuadPart;   

	//��������
	QueryPerformanceCounter(&largeint);   
	end = largeint.QuadPart; 

	//����ʱ��ms
	double TotTime = (double)(end - start) / freq; 
	TotTime = TotTime * 1000;

	//������һ��ʱ��
	start = end;
	return TotTime;
}
