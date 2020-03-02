#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "libusb.h"
#include "USB_Driver.h"

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
	
#define IdVendor 0x245B       //�������豸�ĳ��̺�
#define IdProduct 0x0300      //�������豸�Ĳ�Ʒ��
#define MaxDevNum 10
#define FALSE 0
#define TRUE  1

//�豸����б�
libusb_device_handle* dev_handle_list[MaxDevNum] = {0};
int dev_interface_list[MaxDevNum] = {0};
//�豸����
int dev_handle_list_len = 0;

//��ӡ�豸��Ϣ
void printdev(libusb_device *dev){
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if(r <0){
        printf("failed to get device descriptor\n");
        return;
    }

    printf("Number of possible configurations: %d\n,Device Class: %d\n",(int)desc.bNumConfigurations,(int)desc.bDeviceClass);
    printf("VendorID: %x\n",desc.idVendor);   //VID
    printf("ProductID: %x\n",desc.idProduct); //PID
    printf("Speed: %x\n",libusb_get_device_speed(dev)); //�ٶ�

    libusb_config_descriptor *config;
    libusb_get_config_descriptor(dev, 0, &config);
    printf("Interfaces: %d\n",(int)config->bNumInterfaces); //�ӿڸ���
    printf("maxpower: %d\n",(int)config->MaxPower);         //�����
    const libusb_interface *inter;
    const libusb_interface_descriptor *interdesc;
    const libusb_endpoint_descriptor *epdesc;

    for(int i=0; i<(int)config->bNumInterfaces; i++){
        inter =&config->interface[i];
        printf("Number of alternate settings: %d\n",inter->num_altsetting);
        for(int j=0; j<inter->num_altsetting; j++){
            interdesc =&inter->altsetting[j];
            printf("Interface Number: %x\n",(int)interdesc->bInterfaceNumber);
            printf("Number of endpoints: %x\n",(int)interdesc->bNumEndpoints);

                for(int k=0; k<(int)interdesc->bNumEndpoints; k++){
                    epdesc =&interdesc->endpoint[k];
                    printf("Descriptor Type: %x\n",(int)epdesc->bDescriptorType);
                    printf("EP Address: %x\n",(int)epdesc->bEndpointAddress);
                }
        }
    }
printf("*************************************\n");
    libusb_free_config_descriptor(config);
}

//��ȡ�ӿڸ���
int get_interface_num(libusb_device *dev){
    libusb_device_descriptor desc;
	//��������ȡʧ�ܣ�����0���ӿ�
    int r = libusb_get_device_descriptor(dev, &desc);
    if(r <0){
        printf("failed to get device descriptor\n");
        return 0;
    }
    
    libusb_config_descriptor *config;
    libusb_get_config_descriptor(dev, 0, &config);
	int ret = (int)config->bNumInterfaces;
    libusb_free_config_descriptor(config);
	return ret;
}

//����Ƿ��ǹ�˾�豸
int IsNahonDev(libusb_device *dev)
{
    //��μ��
    if(dev == NULL)
    {
        return FALSE;
    }
	
    //��ȡ������
    struct libusb_device_descriptor desc;
    //���Ƿ��ܻ�ȡ��
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
        fprintf(stderr, "failed to get device descriptor");
        return FALSE;
    }

    //printf("%04X:%04X \r\n",
      //desc.idVendor, desc.idProduct);
			
    return desc.idVendor == IdVendor && desc.idProduct == IdProduct;
}

//��USB(�Ƿ��ʼ��)
int USBScanDev(int NeedInit)
{	
    libusb_device **devs;
    int r, i;
	
	//��ɨ��USB�豸֮ǰ�����ȹر��Ѿ��򿪵��豸��
	if(dev_handle_list_len > 0)
	{
		for(i = 0; i < dev_handle_list_len; i++)
		{
			USBCloseDev(i);
		}
		dev_handle_list_len = 0;
	}

	if(NeedInit > 0)
	{
		//��ʼ��libusb
		r = libusb_init(NULL);
		if (r < 0)
			return r;
	}



    dev_handle_list_len = 0;
    //��ȡ�����豸�б�
    int cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
    	return (int)cnt;

    for (i = 0; i < cnt; ++i) {
    	if(IsNahonDev(devs[i]))
        {
			//printdev(devs[i]);
            dev_handle_list_len++;
        }
    }

   //�ͷ��豸
   libusb_free_device_list(devs, 1);
   if(dev_handle_list_len > MaxDevNum)
   {
	   dev_handle_list_len = MaxDevNum;
   }
   return dev_handle_list_len;
}

//���豸
int USBOpenDev(int DevIndex)
{		
	libusb_device **devs_list;
	//������
	if(DevIndex < 0 || DevIndex >= dev_handle_list_len)
	{
		return USB_ERROR;		
	}
	
	//����Ƿ��Ѿ���
    if(dev_handle_list[DevIndex] != NULL)
	{
		return USB_SUCCESS;
	}
		 
     //ˢ���豸�б�
    int dev_num = libusb_get_device_list(NULL, &devs_list);
    if (dev_num == 0)
    	return USB_ERROR;

    int i, index = 0; 
    //�����豸
    for (i = 0; devs_list[i]; ++i) {
    	if(IsNahonDev(devs_list[i]))
        {
            index++;
            if(DevIndex + 1 == index)
            {
               if(libusb_open(devs_list[i], &dev_handle_list[DevIndex]) == 0)
			   {
					dev_interface_list[DevIndex] = get_interface_num(devs_list[i]);
			   }
			   break;
            }
        }
    }
 
    //�ͷ��豸
    libusb_free_device_list(devs_list, 1);
    if(dev_handle_list[DevIndex]){
		for(i = 0; i < dev_interface_list[DevIndex]; i++){
			libusb_claim_interface(dev_handle_list[DevIndex], i);
		}
		return USB_SUCCESS;
	}else{
		return USB_ERROR;
	}
}

int USBCloseDev(int DevIndex)
{
	int i;
	//������
	if(DevIndex < 0 || DevIndex >= dev_handle_list_len)
	{
		return USB_ERROR;		
	}
	
	//���رն�Ӧ��handle
    if(dev_handle_list[DevIndex] != NULL)
	{
		for(i = 0; i < dev_interface_list[DevIndex]; i++){
			libusb_release_interface(dev_handle_list[DevIndex], i);
		}
		libusb_close(dev_handle_list[DevIndex]);
		dev_handle_list[DevIndex] = NULL;
		dev_interface_list[DevIndex] = 0;
	}
    return USB_SUCCESS;
}

//USBд����
int USBBulkWriteData(unsigned int nBoardID,int pipenum,char *sendbuffer,int len,int waittime)
{
    int ret, out_len;
	//������
	if(nBoardID < 0 || nBoardID >= dev_handle_list_len)
	{
		return USB_ERROR;		
	}
	
	//���رն�Ӧ��handle
    if(dev_handle_list[nBoardID] == NULL)
	{
		return USB_ERROR;	
	}

    //libusb_bulk_transfer�����������ͣ��豸������ �˿ڵ�ַ Ҫ���͵����� Ԥ�Ʒ��͵��ֽ��� ʵ�ʷ��͵��ֽ��� ��ʱʱ��
    ret = libusb_bulk_transfer(dev_handle_list[nBoardID],pipenum,(unsigned char*)sendbuffer,len, &out_len, waittime);
    //ret = libusb_interrupt_transfer(dev_handle_list[nBoardID],pipenum,(unsigned char*)sendbuffer,len, &out_len, waittime);	
    if(ret < 0)
        return ret;

    return out_len;
}

//USB������
int USBBulkReadData(unsigned int nBoardID,int pipenum,char *readbuffer,int len,int waittime)
{
    int ret=0, receive_len;
	//������
	if(nBoardID < 0 || nBoardID >= dev_handle_list_len)
	{
		return USB_ERROR;		
	}
	
	//���رն�Ӧ��handle
    if(dev_handle_list[nBoardID] == NULL)
	{
		return USB_ERROR;	
	}

    //libusb_bulk_transfer�����������ͣ��豸������ �˿ڵ�ַ Ҫ���ܵ����� Ԥ�ƽ��ܵ��ֽ��� ʵ�ʽ��ܵ��ֽ��� ��ʱʱ��
    //libusb_bulk_transfer���ݶ˿ھ�����ʱ�����ǽ��ջ��Ƿ�������
    ret = libusb_bulk_transfer(dev_handle_list[nBoardID], pipenum, (unsigned char*) readbuffer, len, &receive_len,waittime);
    //ret = libusb_interrupt_transfer(dev_handle_list[nBoardID], pipenum, (unsigned char*) readbuffer, len, &receive_len,waittime);
    if(ret < 0)
        return ret;

    return receive_len;
}

