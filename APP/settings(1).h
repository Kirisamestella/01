#ifndef __SETTINGS_H
#define __SETTINGS_H
#include "sys.h"
#include "includes.h" 	   	 
#include "common.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//APP-设置 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/2/16
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//V1.1 20140216
//新增对各种分辨率LCD的支持.
////////////////////////////////////////////////////////////////////////////////// 	   

//list结构体.链表结构
__packed typedef struct 
{
	u8 syslanguage;			//默认系统语言
	u8 picmode;				//图片浏览模式:0,顺序循环播放;1,随机播放.
	u8 saveflag;			//保存标志,0X0A,保存过了;其他,还从未保存	   
}_system_setings;

extern _system_setings systemset;	//在settings.c里面设置 

u8 sysset_time_set(u16 x,u16 y,u8 *hour,u8 *min,u8*caption);
u8 sysset_date_set(u16 x,u16 y,u16 *year,u8 *month,u8 *date,u8*caption);
u8 sysset_bklight_set(u16 x,u16 y,u8* caption,u16 *bkval);
u8 sysset_system_update_cpymsg(u8*pname,u8 pct,u8 mode);
void sysset_system_update(u8*caption,u16 sx,u16 sy);
void sysset_system_info(u16 x,u16 y,u8*caption);
void sysset_system_status(u16 x,u16 y,u8* caption);
void sysset_system_about(u16 x,u16 y,u8*caption);
u8 * set_search_caption(const u8 *mcaption);	 
void sysset_read_para(_system_setings * sysset);
void sysset_save_para(_system_setings * sysset);

u8 sysset_play(void);
										   
#endif












