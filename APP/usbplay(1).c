#include "usbplay.h"
#include "memory.h"
#include "usb_bot.h"
#include "mass_mal.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "usb_regs.h"
#include "mmc_sd.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//APP-USB���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/2/16
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//V1.1 20140216
//�����Ը��ֱַ���LCD��֧��.
////////////////////////////////////////////////////////////////////////////////// 	   

//����USB ����/����
//Enable:0,�Ͽ�
//       1,��������	   
void usb_port_set(u8 enable)
{
	RCC->APB2ENR|=1<<2;    //ʹ��PORTAʱ��	   	 
	if(enable)_SetCNTR(_GetCNTR()&(~(1<<1)));//�˳��ϵ�ģʽ
	else
	{	  
		_SetCNTR(_GetCNTR()|(1<<1));  // �ϵ�ģʽ
		GPIOA->CRH&=0XFFF00FFF;
		GPIOA->CRH|=0X00033000;
		PAout(12)=0;	    		  
	}
}  
//USB����  				 
u8 usb_play(void)
{			  
	u8 rval=0;			//����ֵ	  
 	u8 offline_cnt=0;
 	u8 USB_STA=0;
	u8 Divece_STA=0;
	u8 i=0,err_back=0;
 	u16 werr=0,rerr=0;

   	POINT_COLOR=BLUE;  
  	BACK_COLOR=LGRAY;  
	LCD_Clear(BACK_COLOR);		//����
	app_filebrower((u8*)APP_MFUNS_CAPTION_TBL[2][gui_phy.language],0X05);//��ʾ����   
    Data_Buffer=(u32*)gui_memin_malloc(BULK_MAX_PACKET_SIZE*2*4);	//ΪUSBͨѶ�����ڴ�
	Bulk_Data_Buff=gui_memin_malloc(BULK_MAX_PACKET_SIZE);			//ΪUSBͨѶ�����ڴ�
	if(Data_Buffer!=NULL&&Bulk_Data_Buff!=NULL)
	{
		//USB����
		usb_port_set(0);//USB�ȹر�һ��
		delay_ms(300);  
		usb_port_set(1);//USBʹ�� 
	 	//SD������
	    Mass_Memory_Size[0]=(long long)SD_GetSectorCount()*512;//�õ�SD����������SD����������4G��ʱ��,��Ҫ�õ�����u32����ʾ
	    Mass_Block_Size[0] =512;//��Ϊ������Init����������SD���Ĳ����ֽ�Ϊ512��,��������һ����512���ֽ�.
	    Mass_Block_Count[0]=Mass_Memory_Size[0]/Mass_Block_Size[0];
	 	//SPI FLASH���� 
 		Mass_Memory_Size[1]=9832*512;//ǰ4.8M�ֽ�����FLASH U��.
	    Mass_Block_Size[1] =512;//��Ϊ������Init����������SD���Ĳ����ֽ�Ϊ512��,��������һ����512���ֽ�.
	    Mass_Block_Count[1]=Mass_Memory_Size[1]/Mass_Block_Size[1];
	
		USB_Interrupts_Config();    
		Set_USBClock();   
		USB_Init();	 
		LCD_ShowString(60+(lcddev.width-240)/2,130+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"USB DisConnected ");//������ʾUSB������
	}else rval=1;//�ڴ�����ʧ��	 
   	while(rval==0)
	{
 		delay_ms(1000/OS_TICKS_PER_SEC);		//��ʱһ��ʱ�ӽ���
  		if(system_task_return)//��Ҫ����
		{	  
			if(USB_STATUS_REG&0x03)//USB���ڶ�д
			{
				POINT_COLOR=RED;  					    
				LCD_ShowString(60+(lcddev.width-240)/2,110+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"USB BUSY!!!");//��ʾSD���Ѿ�׼����
				POINT_COLOR=BLUE;  
				err_back=0x80;//������λ
				system_task_return=0;//ȡ��   
			}else break;//USB����,���˳�USB	 		
		} 
		if(USB_STA!=USB_STATUS_REG)//״̬�ı��� 
		{	 						   
			LCD_ShowString(60+(lcddev.width-240)/2,150+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"              ");//���  
			if(USB_STATUS_REG&0x01)//����д		  
			{
				LCD_ShowString(60+(lcddev.width-240)/2,150+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"USB Writing...");//��ʾUSB����д������
				USB_STATUS_REG&=0xFD;//��������
			}
			if(USB_STATUS_REG&0x02)//���ڶ�
			{
				LCD_ShowString(60+(lcddev.width-240)/2,150+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"USB Reading...");//��ʾUSB���ڶ�������  
				USB_STATUS_REG&=0xFE;//���д���	 
			}	 										  
			if(USB_STATUS_REG&0x04)
			{
				POINT_COLOR=RED; 
				werr++; 
				LCD_ShowString(60+(lcddev.width-240)/2,170+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"USB Write Err:");//��ʾд�����
				LCD_ShowNum(172,170,werr,3,16);//��ʾ��������Ĵ���
				POINT_COLOR=BLUE;  
			}													        
			if(USB_STATUS_REG&0x08)
			{
				POINT_COLOR=RED;  
				rerr++;
				LCD_ShowString(60+(lcddev.width-240)/2,190+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"USB Read  Err:");//��ʾ��������
				LCD_ShowNum(172+(lcddev.width-240)/2,190+(lcddev.height-320)/2,rerr,3,16);//��ʾ��������Ĵ���
				POINT_COLOR=BLUE;  
			}														   
			USB_STA=USB_STATUS_REG;//��¼����״̬
		}
		if(Divece_STA!=bDeviceState) 
		{
			if(bDeviceState==CONFIGURED)LCD_ShowString(60+(lcddev.width-240)/2,130+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"USB Connected    ");//��ʾUSB�����Ѿ�����
			else LCD_ShowString(60+(lcddev.width-240)/2,130+(lcddev.height-320)/2,lcddev.width,320,16,"USB DisConnected ");//��ʾUSB���γ���
			Divece_STA=bDeviceState;
		}
		i++;
		if(i==200)//200msʱ�䵽��
		{
			i=0;
			if(USB_STATUS_REG&0x10)
			{
				offline_cnt=0;//USB������,�����offline������
				bDeviceState=CONFIGURED;
			}else//û�еõ���ѯ 
			{
				offline_cnt++;  
				if(offline_cnt>10)bDeviceState=UNCONNECTED;//2s��û�յ����߱��,����USB���γ���
			}
			if(err_back&0x80)//�����˴������
			{
				err_back++;
				if((err_back&0x07)==0x07)
				{
					err_back=0;//��� 	 
					LCD_ShowString(60+(lcddev.width-240)/2,110+(lcddev.height-320)/2,lcddev.width,lcddev.height,16,"           ");//�����ʾ
				}
			}
			USB_STATUS_REG=0;
		}
	}	
	usb_port_set(0);				//USB�ر�
 	gui_memin_free(Data_Buffer);	//�ͷ����뵽���ڴ�
	gui_memin_free(Bulk_Data_Buff);	//�ͷ����뵽���ڴ�
	return rval; 
}







