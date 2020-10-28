#ifndef __CALENDAR_H
#define __CALENDAR_H
#include "sys.h"
#include "includes.h" 	   	 
#include "common.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32������
//APP-���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/2/16
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//V1.1 20140216
//�����Ը��ֱַ���LCD��֧��.
////////////////////////////////////////////////////////////////////////////////// 	   
	    

//���ӽṹ��						   
__packed typedef struct  
{																				   			  
	u8  weekmask;		//������������   
	u8  ringsta;		//����״̬��������־.
						//[7]:0,������;1,������;
						//[6:3]:����
						//[2:0]:������������
	u8  hour;	   		//����Сʱ
	u8  min;			//�������	 
	u8 saveflag;		//�����־,0X0A,�������;����,����δ����	   
}_alarm_obj;

extern _alarm_obj alarm;//���ӽṹ��
extern const u8 *calendar_week_table[GUI_LANGUAGE_NUM][7];//�ⲿ��������ڱ�

void calendar_alarm_init(_alarm_obj *alarmx);
void calendar_alarm_ring(u8 type);
void calendar_date_refresh(void);
void calendar_read_para(_alarm_obj * alarm);
void calendar_save_para(_alarm_obj * alarm);
u8 calendar_alarm_msg(u16 x,u16 y);
u8 calendar_play(void);

 

					    				   
#endif











