#include "calendar.h" 	      						  
#include "stdio.h"
#include "settings.h" 
#include "adc.h"
#include "ds18b20.h"
#include "24cxx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//APP-���� ����	   
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

_alarm_obj alarm;	//���ӽṹ��
					 
static u16 TIME_TOPY;		//	120
static u16 OTHER_TOPY;		//	200 	


const u8 *calendar_week_table[GUI_LANGUAGE_NUM][7]=
{
{"������","����һ","���ڶ�","������","������","������","������"},
{"������","����һ","���ڶ�","������","������","������","������"},
{"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"},
};
//���ӱ���
const u8 *calendar_alarm_caption_table[GUI_LANGUAGE_NUM]=
{
"����","�[�","ALARM",
};
//���찴ť
const u8 *calendar_alarm_realarm_table[GUI_LANGUAGE_NUM]=
{
"����","���","REALARM",
};

const u8* calendar_loading_str[GUI_LANGUAGE_NUM][3]=
{
{
	"���ڼ���,���Ժ�...",
	"δ��⵽DS18B20!",
	"�����ڲ��¶ȴ�����...",
},
{
	"���ڼ���,���Ժ�...",
	"δ��⵽DS18B20!",
	"�����ڲ��¶ȴ�����...",
},
{
	"Loading...",
	"DS18B20 Check Failed!",
	"Start Inside Sensor...",
},
};	

//���³�ʼ������		    
//alarmx:���ӽṹ��
void calendar_alarm_init(_alarm_obj *alarmx) 
{	  
  	u32 curtime=0;
	u32 temptime=0;
	u32 destime=0XFFFFFFFF;//Ŀ������ʱ��(s)�趨�����	
 	if(alarmx->weekmask)//����Ҫ�����Ӵ���,���յ���������һ�� 
	{										       
		curtime=RTC->CNTH;//�õ��������е�ֵ(������)
		curtime<<=16;
		curtime+=RTC->CNTL;	   
		//ȡһ���뵱ǰʱ����ӽ���ֵ��Ϊ���ӼĴ���������
		temptime=curtime/86400;	//�õ���ǰ��������(�˴�û���õ�����,����˵����)
		temptime=temptime*86400;
 		temptime+=(u32)alarmx->hour*3600+(u32)alarmx->min*60;//�õ�������
 		if(temptime<=curtime)temptime+=86400;//ִ��ʱ���ѹ����Ƴٵ�����
 		destime=temptime;//�������ӼĴ���
 	}
	RCC->APB1ENR|=1<<28;//ʹ�ܵ�Դʱ��
    RCC->APB1ENR|=1<<27;//ʹ�ܱ���ʱ��
	PWR->CR|=1<<8;    //ȡ��������д����
	//���������Ǳ����!
	RTC->CRL|=1<<4;   //�������� 
	RTC->CRH|=1<<1;   //���������ж�
	RTC->ALRL=destime&0xffff;
	RTC->ALRH=destime>>16;
	RTC->CRL&=~(1<<4);//���ø���
	while(!(RTC->CRL&(1<<5)));//�ȴ�RTC�Ĵ����������  
}
//����������
//type:��������	   
//0,��.
//1,��.��.
//2,��.��.��
//4,��.��.��.��
void calendar_alarm_ring(u8 type)
{
	u8 i;	 
	for(i=0;i<(type+1);i++)
	{
		LED1=0;
		delay_ms(100);
		LED1=1;
		delay_ms(100);
	}	 
}

//���ݵ�ǰ������,����������.
void calendar_date_refresh(void) 
{
 	u8 weekn;   //�ܼĴ�
	u16 offx=(lcddev.width-240)/2;
 	//��ʾ����������
	POINT_COLOR=BRED; 
	BACK_COLOR=BLACK; 
	LCD_ShowxNum(offx+5,OTHER_TOPY+9,(calendar.w_year/100)%100,2,16,0);//��ʾ��  20/19  
	LCD_ShowxNum(offx+21,OTHER_TOPY+9,calendar.w_year%100,2,16,0);     //��ʾ��  
	LCD_ShowString(offx+37,OTHER_TOPY+9,lcddev.width,lcddev.height,16,"-"); //"-"
	LCD_ShowxNum(offx+45,OTHER_TOPY+9,calendar.w_month,2,16,0X80);     //��ʾ��
	LCD_ShowString(offx+61,OTHER_TOPY+9,lcddev.width,lcddev.height,16,"-"); //"-"
	LCD_ShowxNum(offx+69,OTHER_TOPY+9,calendar.w_date,2,16,0X80);      //��ʾ��	  
	//��ʾ�ܼ�?
	POINT_COLOR=RED;
    weekn=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//�õ�����	   
	Show_Str(20+offx,OTHER_TOPY+35,lcddev.width,lcddev.height,(u8 *)calendar_week_table[gui_phy.language][weekn],16,0); //��ʾ�ܼ�?	
													 
}
//�������ݱ�����:SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings)+sizeof(_vs10xx_obj)
//��ȡ����������Ϣ
//alarm:������Ϣ 
void calendar_read_para(_alarm_obj * alarm)
{
	AT24CXX_Read(SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings),(u8*)alarm,sizeof(_alarm_obj));
}
//д������������Ϣ
//alarm:������Ϣ 
void calendar_save_para(_alarm_obj * alarm)
{
  	OS_CPU_SR cpu_sr=0;
	alarm->ringsta&=0X7F;	//������λ
	OS_ENTER_CRITICAL();	//�����ٽ���(�޷����жϴ��) 
	AT24CXX_Write(SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings),(u8*)alarm,sizeof(_alarm_obj));
	OS_EXIT_CRITICAL();		//�˳��ٽ���(���Ա��жϴ��)
}

//���崦��(�ߴ�:44*20)
//x,y:����
//����ֵ,������
u8 calendar_alarm_msg(u16 x,u16 y)
{
	u8 rval=0;  			  	
  	u16 *lcdbuf=0;							//LCD�Դ� 
   	lcdbuf=(u16*)gui_memin_malloc(44*20*2);	//�����ڴ� 
	if(lcdbuf)								//����ɹ�
	{
		app_read_bkcolor(x,y,44,20,lcdbuf);	//��ȡ����ɫ
		gui_fill_rectangle(x,y,44,20,LIGHTBLUE);
		gui_draw_rectangle(x,y,44,20,BROWN);
		gui_show_num(x+2,y+2,2,RED,16,alarm.hour,0X81); 
 		gui_show_ptchar(x+2+16,y+2,x+2+16+8,y+2+16,0,RED,16,':',1);
		gui_show_num(x+2+24,y+2,2,RED,16,alarm.min,0X81); 
  		OSTaskSuspend(6); //����������
		while(rval==0)
		{
			tp_dev.scan(0);
			if(tp_dev.sta&TP_PRES_DOWN)//������������
			{
				if(app_tp_is_in_area(&tp_dev,x,y,44,20))//�ж�ĳ��ʱ��,��������ֵ�ǲ�����ĳ��������
				{
					rval=0XFF;//ȡ��
					break;
				} 
			 }
			delay_ms(5);  
 	 		if(KEY0_Scan()==1)//����ɨ��KEY0����
			{
				rval=0XFF;		//ȡ��
				break;			//KEY0����	
			} 
		}	 
		app_recover_bkcolor(x,y,44,20,lcdbuf);	//��ȡ����ɫ
	}else rval=1;
	system_task_return=0;
	alarm.ringsta&=~(1<<7);	//ȡ������   
	calendar_alarm_init((_alarm_obj*)&alarm);	//���³�ʼ������
	gui_memin_free(lcdbuf); 
	OSTaskResume(6); 		//�ָ�������
	return rval;
}
	    
//ʱ����ʾģʽ
//ͨ����KEY0�˳�					 
//�����ڲ�sram,ò��OK��.	   
u8 calendar_play(void)
{
	u8 second=0;
	short temperate=0;	//�¶�ֵ		   
	u8 t=0;
	u8 tempdate=0;
	u8 rval=0;			//����ֵ	
	u8 res;
	u16 xoff=0;
	  
	u8 TEMP_SEN_TYPE=0;	//Ĭ��ʹ��DS18B20
	FIL* f_calendar=0;	 
	u8 *bfbase=0;		//���ֿ�Ļ�ַ 
  	f_calendar=(FIL *)gui_memin_malloc(sizeof(FIL));//����FIL�ֽڵ��ڴ����� 
	if(f_calendar==NULL)rval=1;		//����ʧ��
	else
	{
		res=f_open(f_calendar,(const TCHAR*)APP_ASCII_60,FA_READ);//���ļ� 
		if(res==FR_OK)
		{
			bfbase=(u8*)gui_memex_malloc(f_calendar->fsize);	//Ϊ�����忪�ٻ����ַ
			if(bfbase==0)rval=1;
			else 
			{
				res=f_read(f_calendar,bfbase,f_calendar->fsize,(UINT*)&br);	//һ�ζ�ȡ�����ļ�
 			}
			f_close(f_calendar);
		} 
		if(res)rval=res;
	} 	    
	if(rval==0)//�޴���
	{	  
 		LCD_Clear(BLACK);//�����    	  
		second=calendar.sec;//�õ��˿̵�����
		POINT_COLOR=GBLUE;
		Show_Str(48,60,lcddev.width,lcddev.height,(u8*)calendar_loading_str[gui_phy.language][0],16,0x01); //��ʾ������Ϣ	    
		if(DS18B20_Init())
		{
			Show_Str(48,76,lcddev.width,lcddev.height,(u8*)calendar_loading_str[gui_phy.language][1],16,0x01);  
			delay_ms(500);
			Show_Str(48,92,lcddev.width,lcddev.height,(u8*)calendar_loading_str[gui_phy.language][2],16,0x01);  
			Adc_Init();//��ʼ���ڲ��¶ȴ�����	  
			TEMP_SEN_TYPE=1; 
		}   	    
		delay_ms(1100);//�ȴ�1.1s 
		BACK_COLOR= BLACK;
		LCD_Clear(BLACK);//����� 
		
		TIME_TOPY=(lcddev.width-20)/2+20;
		OTHER_TOPY=TIME_TOPY+60+10;
		xoff=(lcddev.width-240)/2;
		
		calendar_date_refresh();  //��������
		tempdate=calendar.w_date;//�����ݴ��� 		
		app_showbigchar(bfbase,xoff+70-4,TIME_TOPY,':',60,GBLUE,BLACK); 	//":"
		app_showbigchar(bfbase,xoff+150-4,TIME_TOPY,':',60,GBLUE,BLACK); 	//":"	 
	}
  	while(rval==0)
	{	
		if(system_task_return)break;	//��Ҫ����	  
 		if(second!=calendar.sec) //���Ӹı���
		{ 	
  			second=calendar.sec;  
			app_showbig2num(bfbase,xoff+10,TIME_TOPY,calendar.hour,60,GBLUE,BLACK);	//��ʾʱ
 			app_showbig2num(bfbase,xoff+90,TIME_TOPY,calendar.min,60,GBLUE,BLACK);	//��ʾ��	 
 			app_showbig2num(bfbase,xoff+170,TIME_TOPY,calendar.sec,60,GBLUE,BLACK);	//��ʾ��	 					   
			if(t%2==0)//�ȴ�2����
			{		 
  				if(TEMP_SEN_TYPE)temperate=Get_Temp();//�õ��ڲ��¶�
				else temperate=DS18B20_Get_Temp();//�õ�18b20�¶�
				if(temperate<0)//�¶�Ϊ������ʱ�򣬺�ɫ��ʾ
				{
					POINT_COLOR=RED;
					temperate=-temperate;//��Ϊ���¶�
				}
				else POINT_COLOR=BRRED;		   //����Ϊ�غ�ɫ������ʾ		 
				app_showbig2num(bfbase,xoff+90,OTHER_TOPY,temperate/10,60,GBLUE,BLACK);			//XX
				app_showbigchar(bfbase,xoff+150,OTHER_TOPY,'.',60,GBLUE,BLACK); 				//"."
				app_showbigchar(bfbase,xoff+180-15,OTHER_TOPY,temperate%10+'0',60,GBLUE,BLACK);	//��ʾС��
				app_showbigchar(bfbase,xoff+210-10,OTHER_TOPY,'C',60,GBLUE,BLACK);				//"��"
				if(t>0)t=0;			 
			}  
			if(calendar.w_date!=tempdate)
			{
				calendar_date_refresh();//�����仯��,��������.  
				tempdate=calendar.w_date;//�޸�tempdate����ֹ�ظ�����
			}
			t++;   
 		} 
		delay_ms(20);
 	};
 	while(tp_dev.sta&TP_PRES_DOWN)tp_dev.scan(0);//�ȴ�TP�ɿ�.
 	gui_memex_free(bfbase);		//ɾ��������ڴ�
	gui_memin_free(f_calendar);	//ɾ��������ڴ�
	GPIOA->CRL&=0XFFFFFFF0;		 	  
	GPIOA->CRL|=0X00000008;		//PA0���ó�����	  
	GPIOA->ODR&=~(1<<0);		//PA0����
	POINT_COLOR=BLUE;
	BACK_COLOR=WHITE ;	
	return rval;
}




















