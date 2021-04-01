#include "stdafx.h"
#include <iostream>
#include <vector>
#include <sstream>
#include "screen_input.h"
#include <stdlib.h>
#include <string.h>


using namespace std; 

//*************************************************************
//公共函数 string& s 输入  string &delim 分割符号 vector<string> $elems分割结果
//*************************************************************
//切分字符串，获取多个输入
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

//获取屏幕输入的整数
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
//计时
double ClickTime()
{
	LARGE_INTEGER largeint; 
	//采样频率
	QueryPerformanceFrequency(&largeint); 
	double freq = (double)largeint.QuadPart;   

	//采样次数
	QueryPerformanceCounter(&largeint);   
	end = largeint.QuadPart; 

	//换算时间ms
	double TotTime = (double)(end - start) / freq; 
	TotTime = TotTime * 1000;

	//更新上一次时间
	start = end;
	return TotTime;
}
