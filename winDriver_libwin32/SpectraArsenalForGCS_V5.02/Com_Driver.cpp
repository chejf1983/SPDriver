// SpectraArsenal.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "stdio.h"
#include "MIGP.h"
#include "Com_Driver.h"
#include "SpectraArsenal.h"


                                 
BOOL Com_Open(HANDLE &CommDev, char *sCom, int iBaudRate)
{
	DCB dcb;

	CommDev = CreateFile(sCom, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if(CommDev == INVALID_HANDLE_VALUE) 
	{
//		printf_debug("Com_Open::Com Open false CommDev is INVALID_HANDLE_VALUE");
		return FALSE;
	}

	if(GetCommState(CommDev, &dcb) == FALSE)
	{
//		printf_debug("Com_Open::Com Can not open");
	    return FALSE;
	}

	dcb.BaudRate = iBaudRate;
	dcb.Parity = NOPARITY;//
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	//dcb.fRtsControl=RTS_CONTROL_DISABLE;
	
	if(SetCommState(CommDev,&dcb) == FALSE) 
	{
//		printf_debug("Com_Open::Com SetCommState false");
	    return FALSE;
	}
	
	SetupComm(CommDev, 4200*10, 4200*10);

//	printf_debug("Com_Open::Com open OK");
	return TRUE;
}

BOOL Com_Close(HANDLE &CommDev)
{
	return CloseHandle(CommDev);
}

BOOL Com_SetReadTime(HANDLE &CommDev, int iReadIntervalTimeOut, int iReadTotalTimeoutMultiplier, int iReadTotalTimeoutConstant)
{
	COMMTIMEOUTS tout;
	
	if(GetCommTimeouts(CommDev, &tout) == FALSE)
	{
//		printf_debug("Com_SetReadTime::GetCommTimeouts FALSE");
		return FALSE;
	}
	
	tout.ReadIntervalTimeout = iReadIntervalTimeOut;     
	tout.ReadTotalTimeoutMultiplier = iReadTotalTimeoutMultiplier; 
    tout.ReadTotalTimeoutConstant = iReadTotalTimeoutConstant; 
	
    //tout.WriteTotalTimeoutMultiplier; 
    //tout.WriteTotalTimeoutConstant; 
	if(SetCommTimeouts(CommDev,&tout) == FALSE)
	{
//		printf_debug("Com_SetReadTime::GetCommTimeouts FALSE");
		return FALSE;
	}
	
	return TRUE;
}

BOOL Com_Write(HANDLE &CommDev, BYTE *TxData, DWORD dLength)
{
	DWORD N = 0;	
	BOOL bR = FALSE;
	DWORD i = 0;
	BYTE bTemp = 0;

	bR = WriteFile(CommDev, TxData, dLength, &N, NULL);
	if(!bR || N != dLength)
	{
//		printf_debug("Com_Write::WriteFile FALSE");
		return FALSE;
	}
	
	return TRUE;
}

BOOL Com_Read(HANDLE CommDev, BYTE *RxData, unsigned int dMaxLength,  unsigned long * readLength)
{
	BYTE bTemp = 0;
	
	if(NULL == RxData)
	{
//		printf_debug("Com_Read::RxData is NULL");
		return FALSE;
	}

	if(NULL == readLength)
	{
//		printf_debug("Com_Read::readLength is NULL");
		return FALSE;
	}

	return ReadFile(CommDev, RxData, dMaxLength, readLength, NULL);
}


