#include "calendar.h" 	      						  
#include "stdio.h"
#include "settings.h" 
#include "adc.h"
#include "ds18b20.h"
#include "24cxx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//APP-日历 代码	   
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

_alarm_obj alarm;	//闹钟结构体
					 
static u16 TIME_TOPY;		//	120
static u16 OTHER_TOPY;		//	200 	


const u8 *calendar_week_table[GUI_LANGUAGE_NUM][7]=
{
{"星期天","星期一","星期二","星期三","星期四","星期五","星期六"},
{"星期天","星期一","星期二","星期三","星期四","星期五","星期六"},
{"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"},
};
//闹钟标题
const u8 *calendar_alarm_caption_table[GUI_LANGUAGE_NUM]=
{
"闹钟","鬧鐘","ALARM",
};
//再响按钮
const u8 *calendar_alarm_realarm_table[GUI_LANGUAGE_NUM]=
{
"再响","再響","REALARM",
};

const u8* calendar_loading_str[GUI_LANGUAGE_NUM][3]=
{
{
	"正在加载,请稍候...",
	"未检测到DS18B20!",
	"启用内部温度传感器...",
},
{
	"正在加载,请稍候...",
	"未检测到DS18B20!",
	"启用内部温度传感器...",
},
{
	"Loading...",
	"DS18B20 Check Failed!",
	"Start Inside Sensor...",
},
};	

//重新初始化闹钟		    
//alarmx:闹钟结构体
void calendar_alarm_init(_alarm_obj *alarmx) 
{	  
  	u32 curtime=0;
	u32 temptime=0;
	u32 destime=0XFFFFFFFF;//目标闹铃时间(s)设定到最大	
 	if(alarmx->weekmask)//必须要有闹钟存在,周日到周六任意一天 
	{										       
		curtime=RTC->CNTH;//得到计数器中的值(秒钟数)
		curtime<<=16;
		curtime+=RTC->CNTL;	   
		//取一个与当前时间最接近的值作为闹钟寄存器的内容
		temptime=curtime/86400;	//得到当前运行天数(此处没有用到天数,仅作说明用)
		temptime=temptime*86400;
 		temptime+=(u32)alarmx->hour*3600+(u32)alarmx->min*60;//得到秒钟数
 		if(temptime<=curtime)temptime+=86400;//执行时间已过，推迟到明天
 		destime=temptime;//更改闹钟寄存器
 	}
	RCC->APB1ENR|=1<<28;//使能电源时钟
    RCC->APB1ENR|=1<<27;//使能备份时钟
	PWR->CR|=1<<8;    //取消备份区写保护
	//上面三步是必须的!
	RTC->CRL|=1<<4;   //允许配置 
	RTC->CRH|=1<<1;   //允许闹钟中断
	RTC->ALRL=destime&0xffff;
	RTC->ALRH=destime>>16;
	RTC->CRL&=~(1<<4);//配置更新
	while(!(RTC->CRL&(1<<5)));//等待RTC寄存器操作完成  
}
//闹钟响闹铃
//type:闹铃类型	   
//0,滴.
//1,滴.滴.
//2,滴.滴.滴
//4,滴.滴.滴.滴
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

//根据当前的日期,更新日历表.
void calendar_date_refresh(void) 
{
 	u8 weekn;   //周寄存
	u16 offx=(lcddev.width-240)/2;
 	//显示阳历年月日
	POINT_COLOR=BRED; 
	BACK_COLOR=BLACK; 
	LCD_ShowxNum(offx+5,OTHER_TOPY+9,(calendar.w_year/100)%100,2,16,0);//显示年  20/19  
	LCD_ShowxNum(offx+21,OTHER_TOPY+9,calendar.w_year%100,2,16,0);     //显示年  
	LCD_ShowString(offx+37,OTHER_TOPY+9,lcddev.width,lcddev.height,16,"-"); //"-"
	LCD_ShowxNum(offx+45,OTHER_TOPY+9,calendar.w_month,2,16,0X80);     //显示月
	LCD_ShowString(offx+61,OTHER_TOPY+9,lcddev.width,lcddev.height,16,"-"); //"-"
	LCD_ShowxNum(offx+69,OTHER_TOPY+9,calendar.w_date,2,16,0X80);      //显示日	  
	//显示周几?
	POINT_COLOR=RED;
    weekn=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//得到星期	   
	Show_Str(20+offx,OTHER_TOPY+35,lcddev.width,lcddev.height,(u8 *)calendar_week_table[gui_phy.language][weekn],16,0); //显示周几?	
													 
}
//闹钟数据保存在:SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings)+sizeof(_vs10xx_obj)
//读取日历闹钟信息
//alarm:闹钟信息 
void calendar_read_para(_alarm_obj * alarm)
{
	AT24CXX_Read(SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings),(u8*)alarm,sizeof(_alarm_obj));
}
//写入日历闹钟信息
//alarm:闹钟信息 
void calendar_save_para(_alarm_obj * alarm)
{
  	OS_CPU_SR cpu_sr=0;
	alarm->ringsta&=0X7F;	//清空最高位
	OS_ENTER_CRITICAL();	//进入临界区(无法被中断打断) 
	AT24CXX_Write(SYSTEM_PARA_SAVE_BASE+sizeof(_system_setings),(u8*)alarm,sizeof(_alarm_obj));
	OS_EXIT_CRITICAL();		//退出临界区(可以被中断打断)
}

//闹铃处理(尺寸:44*20)
//x,y:坐标
//返回值,处理结果
u8 calendar_alarm_msg(u16 x,u16 y)
{
	u8 rval=0;  			  	
  	u16 *lcdbuf=0;							//LCD显存 
   	lcdbuf=(u16*)gui_memin_malloc(44*20*2);	//申请内存 
	if(lcdbuf)								//申请成功
	{
		app_read_bkcolor(x,y,44,20,lcdbuf);	//读取背景色
		gui_fill_rectangle(x,y,44,20,LIGHTBLUE);
		gui_draw_rectangle(x,y,44,20,BROWN);
		gui_show_num(x+2,y+2,2,RED,16,alarm.hour,0X81); 
 		gui_show_ptchar(x+2+16,y+2,x+2+16+8,y+2+16,0,RED,16,':',1);
		gui_show_num(x+2+24,y+2,2,RED,16,alarm.min,0X81); 
  		OSTaskSuspend(6); //挂起主任务
		while(rval==0)
		{
			tp_dev.scan(0);
			if(tp_dev.sta&TP_PRES_DOWN)//触摸屏被按下
			{
				if(app_tp_is_in_area(&tp_dev,x,y,44,20))//判断某个时刻,触摸屏的值是不是在某个区域内
				{
					rval=0XFF;//取消
					break;
				} 
			 }
			delay_ms(5);  
 	 		if(KEY0_Scan()==1)//单独扫描KEY0按键
			{
				rval=0XFF;		//取消
				break;			//KEY0返回	
			} 
		}	 
		app_recover_bkcolor(x,y,44,20,lcdbuf);	//读取背景色
	}else rval=1;
	system_task_return=0;
	alarm.ringsta&=~(1<<7);	//取消闹铃   
	calendar_alarm_init((_alarm_obj*)&alarm);	//重新初始化闹钟
	gui_memin_free(lcdbuf); 
	OSTaskResume(6); 		//恢复主任务
	return rval;
}
	    
//时间显示模式
//通过按KEY0退出					 
//改用内部sram,貌似OK了.	   
u8 calendar_play(void)
{
	u8 second=0;
	short temperate=0;	//温度值		   
	u8 t=0;
	u8 tempdate=0;
	u8 rval=0;			//返回值	
	u8 res;
	u16 xoff=0;
	  
	u8 TEMP_SEN_TYPE=0;	//默认使用DS18B20
	FIL* f_calendar=0;	 
	u8 *bfbase=0;		//大字库的基址 
  	f_calendar=(FIL *)gui_memin_malloc(sizeof(FIL));//开辟FIL字节的内存区域 
	if(f_calendar==NULL)rval=1;		//申请失败
	else
	{
		res=f_open(f_calendar,(const TCHAR*)APP_ASCII_60,FA_READ);//打开文件 
		if(res==FR_OK)
		{
			bfbase=(u8*)gui_memex_malloc(f_calendar->fsize);	//为大字体开辟缓存地址
			if(bfbase==0)rval=1;
			else 
			{
				res=f_read(f_calendar,bfbase,f_calendar->fsize,(UINT*)&br);	//一次读取整个文件
 			}
			f_close(f_calendar);
		} 
		if(res)rval=res;
	} 	    
	if(rval==0)//无错误
	{	  
 		LCD_Clear(BLACK);//清黑屏    	  
		second=calendar.sec;//得到此刻的秒钟
		POINT_COLOR=GBLUE;
		Show_Str(48,60,lcddev.width,lcddev.height,(u8*)calendar_loading_str[gui_phy.language][0],16,0x01); //显示进入信息	    
		if(DS18B20_Init())
		{
			Show_Str(48,76,lcddev.width,lcddev.height,(u8*)calendar_loading_str[gui_phy.language][1],16,0x01);  
			delay_ms(500);
			Show_Str(48,92,lcddev.width,lcddev.height,(u8*)calendar_loading_str[gui_phy.language][2],16,0x01);  
			Adc_Init();//初始化内部温度传感器	  
			TEMP_SEN_TYPE=1; 
		}   	    
		delay_ms(1100);//等待1.1s 
		BACK_COLOR= BLACK;
		LCD_Clear(BLACK);//清黑屏 
		
		TIME_TOPY=(lcddev.width-20)/2+20;
		OTHER_TOPY=TIME_TOPY+60+10;
		xoff=(lcddev.width-240)/2;
		
		calendar_date_refresh();  //加载日历
		tempdate=calendar.w_date;//天数暂存器 		
		app_showbigchar(bfbase,xoff+70-4,TIME_TOPY,':',60,GBLUE,BLACK); 	//":"
		app_showbigchar(bfbase,xoff+150-4,TIME_TOPY,':',60,GBLUE,BLACK); 	//":"	 
	}
  	while(rval==0)
	{	
		if(system_task_return)break;	//需要返回	  
 		if(second!=calendar.sec) //秒钟改变了
		{ 	
  			second=calendar.sec;  
			app_showbig2num(bfbase,xoff+10,TIME_TOPY,calendar.hour,60,GBLUE,BLACK);	//显示时
 			app_showbig2num(bfbase,xoff+90,TIME_TOPY,calendar.min,60,GBLUE,BLACK);	//显示分	 
 			app_showbig2num(bfbase,xoff+170,TIME_TOPY,calendar.sec,60,GBLUE,BLACK);	//显示秒	 					   
			if(t%2==0)//等待2秒钟
			{		 
  				if(TEMP_SEN_TYPE)temperate=Get_Temp();//得到内部温度
				else temperate=DS18B20_Get_Temp();//得到18b20温度
				if(temperate<0)//温度为负数的时候，红色显示
				{
					POINT_COLOR=RED;
					temperate=-temperate;//改为正温度
				}
				else POINT_COLOR=BRRED;		   //正常为棕红色字体显示		 
				app_showbig2num(bfbase,xoff+90,OTHER_TOPY,temperate/10,60,GBLUE,BLACK);			//XX
				app_showbigchar(bfbase,xoff+150,OTHER_TOPY,'.',60,GBLUE,BLACK); 				//"."
				app_showbigchar(bfbase,xoff+180-15,OTHER_TOPY,temperate%10+'0',60,GBLUE,BLACK);	//显示小数
				app_showbigchar(bfbase,xoff+210-10,OTHER_TOPY,'C',60,GBLUE,BLACK);				//"℃"
				if(t>0)t=0;			 
			}  
			if(calendar.w_date!=tempdate)
			{
				calendar_date_refresh();//天数变化了,更新日历.  
				tempdate=calendar.w_date;//修改tempdate，防止重复进入
			}
			t++;   
 		} 
		delay_ms(20);
 	};
 	while(tp_dev.sta&TP_PRES_DOWN)tp_dev.scan(0);//等待TP松开.
 	gui_memex_free(bfbase);		//删除申请的内存
	gui_memin_free(f_calendar);	//删除申请的内存
	GPIOA->CRL&=0XFFFFFFF0;		 	  
	GPIOA->CRL|=0X00000008;		//PA0设置成输入	  
	GPIOA->ODR&=~(1<<0);		//PA0下拉
	POINT_COLOR=BLUE;
	BACK_COLOR=WHITE ;	
	return rval;
}




















