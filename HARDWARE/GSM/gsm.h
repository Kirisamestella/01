#ifndef __gsm_H
#define __gsm_H 
#include "sys.h"

#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 
#include "key.h"	 	 	 	 	 
#include "lcd.h" 
#include "dma.h" 	  
#include "flash.h" 	 
#include "touch.h" 	 
#include "malloc.h"
#include "string.h"    
#include "ff.h"
#include "includes.h"

void sim_at_response(u8 mode);
u8* sim900a_check_cmd(u8 *str);
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime);

u8 sim900a_check_sim(u16 x,u16 y,u8 flag);
u8 sim900a_set_AT(void);
u8 sim900a_connect(u8 flag);
u8 sim900a_star_connect(u8 flag,u8 IP[],u8 PORT[]);


u8 sim900a_send(u8 *p);

u8 work();
u8 sim900a_send_file(u8 *file_path);
u8 sim900a_send_date(u8 *p);
void Num_to_String(u8 *p,u16 len);
void String_to_16Num(u8 *p);
void encryption(u8 *p,u16 len);
#endif
