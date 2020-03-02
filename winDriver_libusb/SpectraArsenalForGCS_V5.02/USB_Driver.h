#ifndef __USB_DRIVER_H__
#define __USB_DRIVER_H__

// Device endpoint(s)
#define EP0		0x00
#define EP1_IN	0x81	//wMaxPacketSize:   16	Int
#define EP1_OUT	0x01	//wMaxPacketSize:   16	Int
#define EP2_IN	0x82	//wMaxPacketSize:   64	Bulk
#define EP2_OUT	0x02	//wMaxPacketSize:   64	Bulk

#define USB_ERROR -1
#define USB_SUCCESS 0

int  USBScanDev(int NeedInit);
int  USBCloseDev(int DevIndex);
int  USBOpenDev(int DevIndex);
int  USBBulkWriteData(unsigned int nBoardID,int pipenum,char *sendbuffer,int len,int waittime);
	
int  USBBulkReadData(unsigned int nBoardID,int pipenum,char *readbuffer,int len,int waittime);

#endif