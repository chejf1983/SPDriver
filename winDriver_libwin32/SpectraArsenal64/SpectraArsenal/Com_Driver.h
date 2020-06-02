
BOOL Com_SetReadTime(HANDLE &CommDev, int iReadIntervalTimeOut, int iReadTotalTimeoutMultiplier, int iReadTotalTimeoutConstant);
BOOL Com_Open(HANDLE &CommDev, char *sCom, int iBaudRate);
BOOL Com_Close(HANDLE &CommDev);
BOOL Com_Write(HANDLE &CommDev, BYTE *TxData, DWORD dLength);
BOOL Com_Read(HANDLE CommDev, BYTE *RxData, unsigned int dMaxLength,  unsigned long * readLength);


