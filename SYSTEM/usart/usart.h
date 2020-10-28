#ifndef __USART_H
#define __USART_H
#include "sys.h"
#include "stdio.h"	 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	
#define USART1_REC_LEN  			200  	//定义最大接收字节数 200
#define USART1_SED_LEN              200
#define EN_USART1_RX 			    1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART1_RX_BUF[USART1_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART1_TX_BUF[USART1_SED_LEN]; //发送缓冲,最大USART_SED_LEN个字节.末字节为换行符 
extern u16 USART1_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 pclk2,u32 bound);

void u1_printf(char* fmt,...);

void Clear_USART1_RX_BUF(void);

#define USART3_MAX_RECV_LEN		200					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		500					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;1,接收.

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态

void usart3_init(u32 pclk1,u32 bound);
void u3_printf(char* fmt,...);
void UART3_DMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u16 len);
void UART3_DMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar);
void TIM4_IRQHandler(void);
void TIM4_Set(u8 sta);
void TIM4_Init(u16 arr,u16 psc);


#define UART4_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_UART4_RX 			1		//使能（1）/禁止（0）串口1接收
extern u8  UART4_RX_BUF[UART4_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 UART4_RX_STA; 
void uart4_init(u32 pclk2,u32 bound);
#endif	   
















