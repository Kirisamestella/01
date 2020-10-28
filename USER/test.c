#include "includes.h"		
#include "common.h"
#include "usart.h"
#include "usart2.h"


/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
#define START_TASK_PRIO      			                   11 //设置任务优先级//开始任务的优先级设置为最低
#define START_STK_SIZE  				                   64 //设置任务堆栈大小	
__align(8) static OS_STK START_TASK_STK[START_STK_SIZE];      //任务堆栈，8字节对齐
void start_task(void *pdata);                                 //任务函数	


//led 任务
#define LED_TASK_PRIO      			                        9 //设置任务优先级
#define LED_STK_SIZE  				                       64 //设置任务堆栈大小	
__align(8) static OS_STK LED_TASK_STK[LED_STK_SIZE];          //任务堆栈，8字节对齐
void led_task(void *pdata);

//key 任务
#define KEY_TASK_PRIO      			                        8 //设置任务优先级
#define KEY_STK_SIZE  				                       64 //设置任务堆栈大小	
__align(8) static OS_STK KEY_TASK_STK[KEY_STK_SIZE];          //任务堆栈，8字节对齐
void key_task(void *pdata);                                   //任务函数


//touch 任务
#define TOUCH_TASK_PRIO      			                    6 //设置任务优先级
#define TOUCH_STK_SIZE  				                  128 //设置任务堆栈大小	64死机，用128
__align(8) static OS_STK TOUCH_TASK_STK[TOUCH_STK_SIZE];      //任务堆栈，8字节对齐
void touch_task(void *pdata);                                 //任务函数


//lcd 任务
#define MAIN_TASK_PRIO      			                    10 //设置任务优先级
#define MAIN_STK_SIZE  				                      512 //设置任务堆栈大小	
__align(8) static OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];        //任务堆栈，8字节对齐
void main_task(void *pdata);                                  //任务函数


//gui 任务
#define GUI_TASK_PRIO      			                       5 //设置任务优先级
#define GUI_STK_SIZE  				                     512 //设置任务堆栈大小	
__align(8) static OS_STK GUI_TASK_STK[GUI_STK_SIZE];         //任务堆栈，8字节对齐
void gui_task(void *pdata);                                  //任务函数



//bar 任务
#define BAR_TASK_PRIO      			                       19 //设置任务优先级
#define BAR_STK_SIZE  				                     128 //设置任务堆栈大小	
__align(8) static OS_STK BAR_TASK_STK[BAR_STK_SIZE];         //任务堆栈，8字节对齐
void bar_task(void *pdata);                                  //任务函数

//GSM 任务
#define GSM_TASK_PRIO      			                       7 //设置任务优先级
#define GSM_STK_SIZE                                      512 //设置任务堆栈大小
__align(8) static OS_STK GSM_TASK_STK[GSM_STK_SIZE];         //任务堆栈，8字节对齐
void gsm_task(void *pdata);                                  //任务函数

OS_EVENT *KEY_Box;
OS_EVENT *LED_Box;
OS_EVENT *TOUCH_Box;
OS_EVENT *GSM_Box;  //GSM信息邮箱

#define ERR_COMMAND      4    //蓝牙主机反馈扫描设备的结果;
#define GET_ALL_RES      5    //蓝牙主机反馈所有设备的温度值;
#define GET_ONE_RES      6    //蓝牙主机反馈指定从机设备的温度值;
#define CONNECTING       7    //GSM建立连接中

#define START_SCAN       1    //开始扫描设备；
#define MSG_QUERY        2    //打开文件;
#define STOP_SCAN        3    //停止扫描设备；
#define LINK             4    //编辑ip信息

#define CHANGE_IP        1    //修改IP
#define CHANGE_PORT      2    //修改端口
#define RE_CONNECT       3    //重新连接
#define RETURN           4    //返回

#define CONFIRM          1    //确认

#define READ_FILE        1    //读取文件
#define PERIPHERAL_STA   2    //从机状态
#define EMPTY            3    //待定状态

#define PRE_PAGE         1
#define NEXT_PAGE        2
#define OPEN_FILE        3

#define BUTTON_ONE       1
#define BUTTON_TWO       2
#define BUTTON_THR       3
#define BUTTON_FOU       4

#define DO_NOTHING       0    //GSM连接成功，可以发送数据
static u8 HOLD;                   //保存上一状态

//任务申明

u8 system_task_return;

u16 point_color[6]={BRED,GRED,GBLUE,DARKBLUE,BRRED,YELLOW};
// #define WHITE         	 0xFFFF
// #define BLACK         	 0x0000	  
// #define BLUE         	 0x001F  
// #define BRED             0XF81F
// #define GRED 			 0XFFE0
// #define GBLUE			 0X07FF
// #define RED           	 0xF800
// #define MAGENTA       	 0xF81F
// #define GREEN         	 0x07E0
// #define CYAN          	 0x7FFF
// #define YELLOW        	 0xFFE0
// #define BROWN 			 0XBC40 //棕色
// #define BRRED 			 0XFC07 //棕红色
// #define GRAY  			 0X8430 //灰色
// //GUI颜色

// #define DARKBLUE      	 0X01CF	//深蓝色
// #define LIGHTBLUE      	 0X7D7C	//浅蓝色  
// #define GRAYBLUE       	 0X5458 //灰蓝色
// //以上三色为PANEL的颜色 
//  
// #define LIGHTGREEN     	 0X841F //浅绿色 
// #define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

// #define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
// #define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

// #define MAX_TEMP       16
// #define MAX_BLE_NUM    12

// typedef struct 
// {
//     u8 whole_num;
//     u8 addr[6];
//     u8 temp[MAX_TEMP][7];
//     
// }BLE_DEVICE;

////static BLE_DEVICE BLE_device[MAX_BLE_NUM]={0};

//static FRESULT res;
static INT32U br, bw;    /* File R/W count文件里读写的字节数*/

#define READ_LEN           200
#define base_address       4916*1024
#define sta_address        4917*1024
#define ip_address         4918*1024
#define port_address       (4918*1024)+5


static u8 SYS_INF=0;
static u8 STA=0;
static u8 connect_sta=0;
////////////////////////////////////////////////////////////////////////////////	

//系统初始化
void system_init(void)
{
	u32 sd_total_size,sd_free_size,flash_total_size,flash_free_size;//SD/FLASH卡总容量，空余容量 /KB
	u8 res,i=0;
    
 	Stm32_Clock_Init(9);	//系统时钟设置
  delay_init(72);			//延时初始化
 	
	uart_init(72,230400); 	//串口1初始化abort
	USART2_Init(36,230400); //初始化串口2，用于蓝牙通信
	usart3_init(36,115200); //初始化串口3，用于GSM
	uart4_init(36,9600);
	
	LCD_Init();				//LCD初始化 
 	LED_Init();	    		//LED初始化
 	KEY_Init();				//按键初始化 	
 	AT24CXX_Init(); 
	SPI_Flash_Init();		//W25Qxx初始化												  
  mem_init();				//内存池初始化
	//Adc_Init(); //ADC初始化
    
	if(!exfuns_init())  SYS_INF |= 1<<0;//exfuns init ok
  if(!RTC_Init())     SYS_INF |= 1<<1;//rtc    init ok
  if(!SD_Initialize())SYS_INF |= 1<<2;//sd     init ok
  if(!font_init())    SYS_INF |= 1<<3;//font   init ok
  if(!TP_Init())      SYS_INF |= 1<<4;//touch  init ok
    
  if(SYS_INF && 1<<0)LCD_ShowString(0,16*(i++),240,16,16,"exfuns OK  ",GREEN,BLACK,1);
  else   LCD_ShowString(0,16*(i++),240,16,16,"exfuns Fail",GREEN,BLACK,1);
  if(SYS_INF && 1<<1)LCD_ShowString(0,16*(i++),240,16,16,"RTC OK  ",GREEN,BLACK,1);
  else   LCD_ShowString(0,16*(i++),240,16,16,"RTC Fail",GREEN,BLACK,1);    //
  if(SYS_INF && 1<<2)LCD_ShowString(0,16*(i++),240,16,16,"SD OK  ",GREEN,BLACK,1);
  else   LCD_ShowString(0,16*(i++),240,16,16,"SD Fail",GREEN,BLACK,1);
  if(SYS_INF && 1<<3)LCD_ShowString(0,16*(i++),240,16,16,"FONT OK  ",GREEN,BLACK,1);
  else   LCD_ShowString(0,16*(i++),240,16,16,"FONT Fail",GREEN,BLACK,1);
  if(SYS_INF && 1<<4)LCD_ShowString(0,16*(i++),240,16,16,"TP OK  ",GREEN,BLACK,1);
  else   LCD_ShowString(0,16*(i++),240,16,16,"TP Fail",GREEN,BLACK,1);
    
	f_mount(fs[0],"0:",1);	//挂载SD卡 
 	f_mount(fs[1],"1:",1); 	//挂载FLASH.
    
 	if(SYS_INF && 1<<2)
  {
		LCD_ShowString(0,16*(i++),240,16,16,"SD:Total:    MB Free:    MB",GREEN,BLACK,1);
    res=exf_getfree("0:",&sd_total_size,&sd_free_size);//得到SD卡剩余容量和总容量
    if(!res)
    {
			LCD_ShowxNum(72,16*(i-1),sd_total_size>>10,4,16,0,GREEN,BLACK,1);
      LCD_ShowxNum(168,16*(i-1),sd_free_size>>10,4,16,0,GREEN,BLACK,1);  
    }            
  }
 	res=exf_getfree("1:",&flash_total_size,&flash_free_size);//得到FLASH剩余容量和总容量
  LCD_ShowString(0,16*(i++),240,16,16,"FLASH:Total:   MB Free:   MB",GREEN,BLACK,1); 
  if(!res)
  {
		LCD_ShowxNum(96,16*(i-1),flash_total_size>>10,3,16,0,GREEN,BLACK,1);
    LCD_ShowxNum(184,16*(i-1),flash_free_size>>10,3,16,0,GREEN,BLACK,1);      
  }
  LCD_ShowString(0,16*i,240,16,16,"System Running...",GREEN,BLACK,2);
  if(KEY_Scan(0)==1)TP_Adjust();
  else
  {
    delay_ms(1500);
		//delay_ms(1500);
  }
    
} 

			 
int main(void)
{		 
  system_init();//初始化系统
	OSInit();//初始化ucos
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();//ucos
}



//开始任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
    
  KEY_Box = OSMboxCreate ((void*)0);	//创建消息邮箱
  LED_Box = OSMboxCreate ((void*)0);	//创建消息邮箱
  TOUCH_Box = OSMboxCreate ((void*)0);	//创建消息邮箱
	GSM_Box = OSMboxCreate ((void*)0);
    
	pdata = pdata; 	   
	OSStatInit();		//初始化统计任务.这里会延时1秒钟左右	
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)
	
  OSTaskCreate(touch_task,   (void *)0,   (OS_STK *)& TOUCH_TASK_STK[TOUCH_STK_SIZE-1],   TOUCH_TASK_PRIO  );//触摸屏任务  
  OSTaskCreate(gsm_task,    (void *)0,   (OS_STK *)& GSM_TASK_STK[GSM_STK_SIZE-1],     GSM_TASK_PRIO    );//GSM任务
  OSTaskCreate(led_task,     (void *)0,   (OS_STK *)& LED_TASK_STK[LED_STK_SIZE-1],       LED_TASK_PRIO    );//LED灯任务
  OSTaskCreate(main_task,    (void *)0,   (OS_STK *)& MAIN_TASK_STK[MAIN_STK_SIZE-1],     MAIN_TASK_PRIO    );//主任务
  OSTaskCreate(key_task,    (void *)0,   (OS_STK *)& KEY_TASK_STK[KEY_STK_SIZE-1],     KEY_TASK_PRIO    );//按键任务
	
	
    //OSTaskCreate(gui_task,     (void *)0,   (OS_STK *)& GUI_TASK_STK[GUI_STK_SIZE-1],       GUI_TASK_PRIO    );//创建任务
    //OSTaskCreate(bar_task,     (void *)0,   (OS_STK *)& BAR_TASK_STK[BAR_STK_SIZE-1],       BAR_TASK_PRIO    );//创建任务
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
  IWDG_Init(6,1560);   //时间计算(大概):Tout=((4*2^prer)*rlr)/40 (ms).这里约10秒
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断)
}


void led_task(void *pdata)//每隔一段时间亮两次黄色LED灯
{
    while(1)
    {
        YELLOW_LED=0;
        delay_ms(100);
        YELLOW_LED=1;
        delay_ms(50);
        YELLOW_LED=0;
        delay_ms(100);
        YELLOW_LED=1;
        delay_ms(1500);
    }
}

//KEY0_PRES，KEY0按下
//KEY1_PRES，KEY1按下
//WKUP_PRES，WK_UP按下 
//优先级,KEY0>KEY1>WK_UP!!
void key_task(void *pdata)//检测有无按键按下
{
    u8 key_val,p[]="0";
    while(1)
    {
        key_val=KEY_Scan(0);		//按键扫描函数
        if(key_val)
        {
            p[0]=key_val;
            OSMboxPost(KEY_Box,&p); //发送消息
        }
        delay_ms(10);
    }
}

u8 IP[4]={0};//flash中存储的IP地址
u8 PORT[2]={0};//flash中存储的端口
u8 server_address[16]={0};//缓存修改
u8 tmp_server_address[16]={0};
u8 address_pointer=-1;//高亮指针
void GET_IP_PORT()
{
	SPI_Flash_Init();
	
	SPI_Flash_Read(IP,ip_address,4);
	if(IP[0] == 0xFF){
		IP[0]=111;  IP[1]=229;  IP[2]=170;  IP[3]=131;
		SPI_Flash_Write(IP,ip_address,4);
	}
	SPI_Flash_Read(PORT,port_address,2);
	if(PORT[0] == 0xFF){
		PORT[0]=80;  PORT[1]=88;
		SPI_Flash_Write(PORT,port_address,2);
	}
	
	USART2_Init(36,230400);
	uart4_init(36,9600);
}

//与服务器连接
//缺少连接失败原因
void gsm_task(void *pdata)
{
	u8 gsm_err;
	u8 *key=0;
	usart3_init(36,115200);
	//开机自动连接
	GET_IP_PORT();
	HOLD = STA;  STA = CONNECTING;
	if(sim900a_star_connect(1,IP,PORT))
		connect_sta=1;
	STA = HOLD;
	
	while(1)
	{
		key=(u8*)OSMboxPend(GSM_Box,0,&gsm_err);//连接断开时通过邮箱发信号重新连接一次
		HOLD = STA;  STA = CONNECTING;
		if(sim900a_star_connect(0,IP,PORT)) connect_sta=1;//重新连接不显示sim卡信息
		STA = HOLD;
	}
}

void clean_screen()
{
	LCD_Fill(0,29,239,287,BLACK);
	LCD_DrawLine(0,65,239,65,GREEN);
	delay_ms(50);
}

#define BUTTON_STA_ONE  1
#define BUTTON_STA_TWO  2
#define BUTTON_STA_THR  3
#define BUTTON_STA_FOU  4
#define BUTTON_STA_FIV  5
#define BUTTON_STA_SIX  6
static u8 *button_one[4]={"启动采集","信息查询","停止采集","连接"};
static u8 *button_two[4]={"修改 IP ","修改端口","重新连接","返回"};
static u8 *button_thr[2]={"  确认  ","  返回  "};
static u8 *button_fou[4]={"读取文件","从机状态","        ","返回"};
static u8 *button_fiv[4]={" 上一页 "," 下一页 ","  打开  ","返回"};
static u8 *button_six[4]={" 上一页 "," 下一页 ","        ","返回"};
static u8 button_sta = 0;

void button_pattern_one(u8 highlight,u8 *str[])//入口参数为高亮按钮
{
	f_mount(fs[0],"0:",1);//中文要访问字库
	
	
	if(highlight == START_SCAN || highlight == CHANGE_IP || highlight == READ_FILE || highlight == PRE_PAGE){
		LCD_Fill(0,288,63,295,LIGHTBLUE);
		Show_Str(0,296,120,16,str[0],16,0,YELLOW,LIGHTBLUE,0);
		LCD_Fill(0,312,63,319,LIGHTBLUE);
	}
  else if(!highlight){
		LCD_Fill(0,288,63,295,GRAY);
		Show_Str(0,296,120,16,str[0],16,0,YELLOW,GRAY,0);
		LCD_Fill(0,312,63,319,GRAY);
	}
	
	if(highlight == MSG_QUERY || highlight == CHANGE_PORT || highlight == PERIPHERAL_STA || highlight == NEXT_PAGE){
		LCD_Fill(70,288,133,295,LIGHTBLUE);
		Show_Str(70,296,120,16,str[1],16,0,YELLOW,LIGHTBLUE,0);
		LCD_Fill(70,312,133,319,LIGHTBLUE);
	}
  else if(!highlight){
		LCD_Fill(70,288,133,295,GRAY);
		Show_Str(70,296,120,16,str[1],16,0,YELLOW,GRAY,0);
		LCD_Fill(70,312,133,319,GRAY);
	}
	
	if(highlight == STOP_SCAN || highlight == RE_CONNECT || highlight == EMPTY || highlight == OPEN_FILE){
		LCD_Fill(140,288,203,295,LIGHTBLUE);
		Show_Str(140,296,120,16,str[2],16,0,YELLOW,LIGHTBLUE,0);
		LCD_Fill(140,312,203,319,LIGHTBLUE);
	}
  else if(!highlight){
		LCD_Fill(140,288,203,295,GRAY);
		Show_Str(140,296,120,16,str[2],16,0,YELLOW,GRAY,0);
		LCD_Fill(140,312,203,319,GRAY);
	}
	
	if(highlight == LINK || highlight == RETURN){
		LCD_Fill(208,288,239,295,LIGHTBLUE);
		Show_Str(208,296,120,16,str[3],16,0,YELLOW,LIGHTBLUE,0);
		LCD_Fill(208,312,239,319,LIGHTBLUE);
	}
	else if(!highlight){
		LCD_Fill(208,288,239,295,GRAY);
		Show_Str(208,296,120,16,str[3],16,0,YELLOW,GRAY,0);
		LCD_Fill(208,312,239,319,GRAY);
	}
	
	if(str == button_one) button_sta = BUTTON_STA_ONE;
	else if(str == button_two) button_sta = BUTTON_STA_TWO;
	else if(str == button_fou) button_sta = BUTTON_STA_FOU;
	else if(str == button_fiv) button_sta = BUTTON_STA_FIV;
	else if(str == button_six) button_sta = BUTTON_STA_SIX;
	
	
	//用完SD卡要重新初始化串口
	USART2_Init(36,230400);
	uart4_init(36,9600);
	//SPI_Flash_Init();

}

void button_pattern_two(u8 highlight,u8 *str[])
{
	f_mount(fs[0],"0:",1);//中文要访问字库
	
	if(highlight == CONFIRM){
		LCD_Fill(30,288,93,295,LIGHTBLUE);
		Show_Str(30,296,120,16,str[0],16,0,YELLOW,LIGHTBLUE,0);
		LCD_Fill(30,312,93,319,LIGHTBLUE);
	}
	else if(!highlight){
		LCD_Fill(30,288,93,295,GRAY);
		Show_Str(30,296,120,16,str[0],16,0,YELLOW,GRAY,0);
		LCD_Fill(30,312,93,319,GRAY);
	}
	
	if(highlight == RETURN){
		LCD_Fill(150,288,213,295,LIGHTBLUE);
		Show_Str(150,296,120,16,str[1],16,0,YELLOW,LIGHTBLUE,0);
		LCD_Fill(150,312,213,319,LIGHTBLUE);
	}
	else if(!highlight){
		LCD_Fill(150,288,213,295,GRAY);
		Show_Str(150,296,120,16,str[1],16,0,YELLOW,GRAY,0);
		LCD_Fill(150,312,213,319,GRAY);
	}
	
	button_sta = BUTTON_STA_THR;
	
	//用完SD卡要重新初始化串口
	USART2_Init(36,230400);
	uart4_init(36,9600);
	//SPI_Flash_Init();

}

void choose_pattern(u8 press)
{
	switch(button_sta){
		case BUTTON_STA_ONE:
			button_pattern_one(press,button_one);
			break;
		case BUTTON_STA_TWO:
			button_pattern_one(press,button_two);
			break;
		case BUTTON_STA_THR:
			button_pattern_two(press,button_thr);
			break;
		case BUTTON_STA_FOU:
			button_pattern_one(press,button_fou);
			break;
		case BUTTON_STA_FIV:
			button_pattern_one(press,button_fiv);
			break;
		case BUTTON_STA_SIX:
			button_pattern_one(press,button_six);
			break;
		default :break;
	}
}

u8 first_press=TRUE,first_loosen=TRUE;
void touch_task(void *pdata)
{
	u8 touch_msg[]="0";

	SPI_Flash_Init();//读flash后要初始化串口
	
	u1_printf("START");//系统刚启动时候，串口发送的第一个字节丢失，估计是上下拉问题，这里采用粗鲁的办法，直接先输出几个无效数据。
	SPI_Flash_Read(&touch_msg[0],base_address,1);
	if(touch_msg[0]!=0x55)
	{
		touch_msg[0]=0x55;
		SPI_Flash_Write(&touch_msg[0],base_address,1);
		touch_msg[0]=STOP_SCAN;
	}
	else SPI_Flash_Read(&touch_msg[0],sta_address,1);
	OSMboxPost(TOUCH_Box,&touch_msg); //发送消息
	STA=touch_msg[0];
	
	USART2_Init(36,230400);
	uart4_init(36,9600);

	while(1)
	{
		tp_dev.scan(0); 		
		if(STA==CONNECTING){ //GSM建立连接中 无法执行其他任务
			touch_msg[0]=0;
			OSMboxPost(TOUCH_Box,&touch_msg); //发送消息
			delay_ms(10);
			continue;
		}
		if(tp_dev.sta&TP_PRES_DOWN && first_press == TRUE)			//触摸屏被按下
		{	
			if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				switch(button_sta){
					case BUTTON_STA_ONE :
					case BUTTON_STA_TWO :
					case BUTTON_STA_FOU :
					case BUTTON_STA_FIV :
					case BUTTON_STA_SIX :{
						if(check_tp_coords(0,288,63,319)==TRUE){
							touch_msg[0]=BUTTON_ONE;
							choose_pattern(BUTTON_ONE);
						}
						else if(check_tp_coords(70,288,133,319)==TRUE){
							touch_msg[0]=BUTTON_TWO;
							choose_pattern(BUTTON_TWO);
						}
						else if(check_tp_coords(140,288,203,319)==TRUE){
							touch_msg[0]=BUTTON_THR;
							choose_pattern(BUTTON_THR);
						}
						else if(check_tp_coords(208,288,239,319)==TRUE){
							touch_msg[0]=BUTTON_FOU;
							choose_pattern(BUTTON_FOU);
						}
						else touch_msg[0]=0;
						OSMboxPost(TOUCH_Box,&touch_msg); //发送消息
						break;
					}
					case BUTTON_STA_THR :{
						if(check_tp_coords(30,288,93,319)==TRUE){
							touch_msg[0]=BUTTON_ONE;
							choose_pattern(BUTTON_ONE);
						}
						else if(check_tp_coords(150,288,213,319)==TRUE){
							touch_msg[0]=BUTTON_FOU;
							choose_pattern(BUTTON_FOU);
						}
						else touch_msg[0]=0;
						OSMboxPost(TOUCH_Box,&touch_msg); //发送消息
						break;
					}
					default: break;
				}
				first_press = FALSE;  first_loosen = TRUE;
			}
		}
		delay_ms(25);
	}
}

void copy_server_address()
{
	u8 i;
	for(i=0;i<16;++i)
		tmp_server_address[i] = server_address[i];
}

void refresh_server_address()
{
	u8 i;
	for(i=0;i<16;++i)
		server_address[i] = tmp_server_address[i];
}

void SHOW_IP(u8 str[])
{
	u8 i,offset;
	
	clean_screen();
	LCD_ShowString(20,70,120,16,16," IP  :",WHITE,BLACK,0);
	LCD_ShowString(20,90,120,16,16,"PORT :",WHITE,BLACK,0);
	
	for(i=0;i<4;++i){
		offset = 70+ i*32;
		if(address_pointer == i*3) LCD_ShowNum(offset,70,str[i*3],1,16,WHITE,LIGHTBLUE,0);
		else LCD_ShowNum(offset,70,str[i*3],1,16,WHITE,BLACK,0);
		
		if(address_pointer == i*3+1) LCD_ShowNum(offset+8,70,str[i*3+1],1,16,WHITE,LIGHTBLUE,0);
		else LCD_ShowNum(offset+8,70,str[i*3+1],1,16,WHITE,BLACK,0);
		
		if(address_pointer == i*3+2) LCD_ShowNum(offset+16,70,str[i*3+2],1,16,WHITE,LIGHTBLUE,0);
		else LCD_ShowNum(offset+16,70,str[i*3+2],1,16,WHITE,BLACK,0);
		
		if(i<3) LCD_ShowString(offset+24,70,8,16,16,".",WHITE,BLACK,0);
	}
	
	for(i=0;i<4;++i){
		offset = 70 + i*8;
		if(address_pointer == i+12) LCD_ShowNum(offset,90,str[i+12],1,16,WHITE,LIGHTBLUE,0);
		else LCD_ShowNum(offset,90,str[i+12],1,16,WHITE,BLACK,0);
	}
}

void ip_message()
{
	u8 i;
	
	for(i=0;i<4;++i){
		server_address[i*3]=IP[i]/100;
		server_address[i*3+1]=IP[i]/10%10;
		server_address[i*3+2]=IP[i]%10;
	}
	server_address[12]=PORT[0]/10;  server_address[13]=PORT[0]%10;
	server_address[14]=PORT[1]/10;  server_address[15]=PORT[1]%10;
	
	copy_server_address();//复制一份用于按键修改
	
	SHOW_IP(server_address);
}

void ip_port_change()
{
	u8 i;
	u16 tmp;
	
	refresh_server_address();
	
	for(i=0;i<4;++i){
		tmp = server_address[i*3]*100 + server_address[i*3+1]*10 +server_address[i*3+2];
		if(tmp < 0 || tmp > 255);
		else IP[i]=tmp;
	}
	tmp = server_address[12]*10 + server_address[13];
	if(tmp >= 0 && tmp < 100) PORT[0] = tmp;
	tmp = server_address[14]*10 + server_address[15];
	if(tmp >= 0 && tmp < 100) PORT[1] = tmp;
	//写入flash
	
	SPI_Flash_Init();
	
	SPI_Flash_Write(IP,ip_address,4);
	SPI_Flash_Write(PORT,port_address,2);
	
	USART2_Init(36,230400);
	uart4_init(36,9600);
}

static FIL fp;
char file_dir[30][20];//文件夹目录
u8 file_dir_num=0;
u8 file_dir_pointer=0;
static char file_path[50]={"0:/Temperature"};//文件地址
u8 page=0,file_dir_max=0;

void print_dir()
{
	u8 i;
	
	LCD_Fill(0,85,239,287,BLACK);
	for(i=0;i < 10 && i+page*10 < file_dir_num;++i)
		if(file_dir_pointer == i) Show_Str(10,85+i*17,240,16,file_dir[i+page*10],16,0,BRED,LIGHTBLUE,0);
		else Show_Str(10,85+i*17,240,16,file_dir[i+page*10],16,0,BRED,BLACK,0);
}

void print_address()
{
	u8 len;
	
	len=strlen(file_path);
	LCD_Fill(0,50,239,61,BLACK);
	
	if(len > 40){
		LCD_ShowString(0,50,144,12,12,file_path+2,WHITE,BLACK,0);
		LCD_ShowString(144,50,30,12,12,".....",WHITE,BLACK,0);
		LCD_ShowString(174,50,240,12,12,file_path+len-11,WHITE,BLACK,0);
	}
	else LCD_ShowString(0,50,240,12,12,file_path+2,WHITE,BLACK,0);
}

void cut_file_path()
{
	u8 i;
	i=strlen(file_path)-1;
	for(;;--i)
		if(file_path[i] == '/'){
			file_path[i]='\0';
			break;
		}
}

static char lfn[_MAX_LFN];
void open_file()
{
	DIR dir;  FILINFO fileinfo;
	
#if _USE_LFN
fileinfo.lfname = lfn;
fileinfo.lfsize = sizeof(lfn);
#endif
	
	f_mount(fs[0],"0:",1);//挂载SD卡
	
	if( f_opendir(&dir,file_path) == FR_OK){
		//写成res = f_readdir(&dir,&fileinfo)会导致死机，原因未明
		LCD_Fill(0,30,120,45,BLACK);
		LCD_Fill(0,68,240,83,BLACK);
		Show_Str(0,30,120,16,"打开成功",16,0,BRED,BLACK,1);
		Show_Str(0,68,120,16,"文件列表:",16,0,BRED,BLACK,1);
		
		file_dir_num=0;  file_dir_pointer=0;  page=0;
		while( f_readdir(&dir,&fileinfo) == FR_OK ){
			if(fileinfo.fname[0] == 0) break;
			if(strlen(fileinfo.lfname) > 0)
				sprintf(file_dir[file_dir_num++],"%s\0",fileinfo.lfname);
			else sprintf(file_dir[file_dir_num++],"%s\0",fileinfo.fname);
		}
		file_dir_max = (page + 1) * 10 > file_dir_num ? file_dir_num%10 : 10;
		print_address();
		print_dir();
	}
	else{
		LCD_Fill(0,30,120,35,BLACK);
		Show_Str(0,30,120,16,"打开失败1_1",16,0,BRED,BLACK,1);
		cut_file_path();
		SD_Initialize();
	}
	
	//用完SD卡要重新初始化串口
	USART2_Init(36,230400);
	uart4_init(36,9600);
	//SPI_Flash_Init();
	
}

void read_txt_fail(u8 *str)
{
	LCD_Fill(0,30,120,35,BLACK);
	Show_Str(0,30,120,16,str,16,0,BRED,BLACK,1);
	f_close(&fp);
	SD_Initialize();
}

void show_txt_msg()
{
	u8 p[15],p2[25];
	u8 port_num=0,len,i;
	
	len = strlen(file_path);
	while(file_path[len] != '/') len--;
	port_num = (file_path[len+5]-'0')*10 + (file_path[len+6]-'0');
	len--;
	while(file_path[len] != '/') len--;  len++;
	for(i=0;i<14;++i) p[i] = file_path[len+i];  p[14] = 0;
	sprintf(p2,"%s端口%02d:",p,port_num);
	LCD_Fill(0,68,120,83,BLACK);
	Show_Str(0,68,240,16,p2,16,0,BRED,BLACK,0);
}

int txt_size=0,txt_pointer=0;
u8 txt_read_buffer[READ_LEN]={0};
u8 txt_data_buffer[11][20]={0},txt_data_buffer_num=0;
u8 read_buffer_len=0;
u8 data_buffer_size[500]={0};
u16 data_page=0;
void read_txt()
{
	u8 *p1,i;
	
	f_mount(fs[0],"0:",1);//挂载SD卡
	
	if(f_open(&fp,file_path, FA_OPEN_ALWAYS | FA_READ) == FR_OK){
		txt_size = f_size(&fp);
		printf("%d   %d\r\n",txt_size,txt_pointer);
		if(f_lseek(&fp,txt_pointer) == FR_OK){
			if(f_read(&fp,txt_read_buffer,READ_LEN,&br) == FR_OK){
				txt_read_buffer[READ_LEN-1] = 0;
				read_buffer_len = strlen(txt_read_buffer);
				//printf("%s\r\n",txt_read_buffer);
				for(i=0;i<10 && data_buffer_size[data_page] < read_buffer_len ;++i){
					//printf("---%d    %d \r\n",data_buffer_size[data_page],read_buffer_len);
					p1=(u8*)strstr((const char*)(txt_read_buffer + data_buffer_size[data_page]),"\n");
					p1[0]=0;
					sprintf((char*)txt_data_buffer[i],"%s",txt_read_buffer + data_buffer_size[data_page]);
					printf("%s\r\n",txt_data_buffer[i]);
					data_buffer_size[data_page] += p1 - txt_read_buffer - data_buffer_size[data_page] +1;
					
					txt_data_buffer_num = i;
				}
				f_close(&fp);
				for(i=0;i<READ_LEN;++i) txt_read_buffer[i]=0;//清空
				LCD_Fill(0,30,120,45,BLACK);
				Show_Str(0,30,120,16,"打开成功1_2",16,0,BRED,BLACK,0);
				print_address();  show_txt_msg();
			}
			else read_txt_fail("读取失败1_2");
		}
		else read_txt_fail("定位失败1_3");
	}
	else read_txt_fail("打开失败1_4");
	
	//用完SD卡要重新初始化串口
	USART2_Init(36,230400);
	uart4_init(36,9600);
}

void show_txt_data()
{
	u8 i;
	
	LCD_Fill(0,85,239,287,BLACK);
	for(i=0;i<=txt_data_buffer_num;++i)
		Show_Str(10,85+i*17,240,16,txt_data_buffer[i],16,0,BRED,BLACK,0);
}

u8 gsm_msg=1;
void button_management(u8 press)
{
	u8 len;
	switch(button_sta){
		case BUTTON_STA_ONE :{
			switch(press){
				case START_SCAN :
					u2_printf(" ");
					delay_ms(25);//防止与上一条信息冲突
					u2_printf("1#");
					button_pattern_one(0,button_one);
					break;
				case MSG_QUERY :
					button_pattern_one(0,button_fou);
					break;
				case STOP_SCAN :
					u2_printf(" ");
					delay_ms(25);
					u2_printf("2#");
					button_pattern_one(0,button_one);
					break;
				case LINK :
					clean_screen();
					button_pattern_one(0,button_two);
					address_pointer=-1;
					GET_IP_PORT();
					ip_message();
					break;
				default :break;
			}
			break;
		}
		case BUTTON_STA_TWO :{
			switch(press){
				case CHANGE_IP :
					LCD_Fill(0,288,239,319,BLACK);
					button_pattern_two(0,button_thr);
					address_pointer=0;
					copy_server_address();
					SHOW_IP(tmp_server_address);
					break;
				case CHANGE_PORT :
					LCD_Fill(0,288,239,319,BLACK);
					button_pattern_two(0,button_thr);
					address_pointer=12;
					copy_server_address();
					SHOW_IP(tmp_server_address);
					break;
				case RE_CONNECT :
					OSMboxPost(GSM_Box,&gsm_msg);
					button_pattern_one(0,button_two);
					break;
				case RETURN :
					clean_screen();
					button_pattern_one(0,button_one);
					break;
				default: break;
			}
			break;
		}
		case BUTTON_STA_THR :{
			switch(press){
				case CONFIRM :
					ip_port_change();
					LCD_Fill(0,288,239,319,BLACK);
					button_pattern_one(0,button_two);
					address_pointer=-1;
					ip_message();
					break;
				case RETURN :
					LCD_Fill(0,288,239,319,BLACK);
					button_pattern_one(0,button_two);
					address_pointer=-1;
					SHOW_IP(server_address);
					break;
				default: break;
			}
			break;
		}
		case BUTTON_STA_FOU :{
			switch(press){
				case READ_FILE :
					clean_screen();
					button_pattern_one(0,button_fiv);
					delay_ms(50);
					open_file();
					break;
				case PERIPHERAL_STA :
					break;
				case EMPTY:
					button_pattern_one(0,button_fou);
					break;
				case RETURN :
					clean_screen();
					button_pattern_one(0,button_one);
				default: break;
			}
			break;
		}
		case BUTTON_STA_FIV :{
			switch(press){
				case PRE_PAGE:
					page = (page + (file_dir_num+9)/10 - 1)%((file_dir_num+9)/10);
					file_dir_pointer = 0;
					file_dir_max = (page + 1) * 10 > file_dir_num ? file_dir_num%10 : 10;
					print_dir();
					button_pattern_one(0,button_fiv);
					break;
				case NEXT_PAGE:
					page = (page + 1)%((file_dir_num+9)/10);
					file_dir_pointer = 0;
					file_dir_max = (page + 1) * 10 > file_dir_num ? file_dir_num%10 : 10;
					print_dir();
					button_pattern_one(0,button_fiv);
					break;
				case OPEN_FILE:
					strcat(file_path,"/");
					strcat(file_path,file_dir[file_dir_pointer + page * 10]);
					printf("%s\r\n",file_path);
					len = strlen(file_path);
					if(file_path[len-4] == '.'){
						printf("undetermined");
						txt_pointer = 0;  data_page = 0;
						data_buffer_size[data_page] = 0;
						read_txt();
						button_pattern_one(0,button_six);
						show_txt_data();
					}
					else{
						open_file();
						button_pattern_one(0,button_fiv);
					}
					break;
				case RETURN:
					if(strcmp(file_path,"0:/Temperature") == 0)
						button_pattern_one(0,button_fou);
					else{
						cut_file_path();
						open_file();
						button_pattern_one(0,button_fiv);
					}
					break;
				default :break;
			}
			break;
		}
		case BUTTON_STA_SIX :{
			switch(press){
				case PRE_PAGE:
					if(data_page == 0)
						LCD_ShowString(0,272,240,16,16,"First Page",WHITE,BLACK,0);
					else{
						data_page--;
						txt_pointer -= data_buffer_size[data_page];
						data_buffer_size[data_page] = 0;
						read_txt();
						show_txt_data();
					}
					button_pattern_one(0,button_six);
					break;
				case NEXT_PAGE:
					if(txt_pointer + data_buffer_size[data_page] == txt_size)
						LCD_ShowString(0,272,240,16,16,"Last Page",WHITE,BLACK,0);
					else{
						txt_pointer += data_buffer_size[data_page];
						data_buffer_size[ ++data_page ] = 0;
						read_txt();
						show_txt_data();
					}
					button_pattern_one(0,button_six);
					break;
				case EMPTY :
					button_pattern_one(0,button_six);
					break;
				case RETURN:
					cut_file_path();
					button_pattern_one(0,button_fiv);
					open_file();
					break;
				default :break;
			}
			break;
		}
		default: break;
	}
	delay_ms(50);
}

void key_pattern_one(u8 press)
{
	switch(press){
		case KEY0_PRES ://->
			if(address_pointer < 12) address_pointer = (address_pointer + 1)%12;
			else if(address_pointer >=12 && address_pointer <16) address_pointer = (address_pointer - 11)%4 + 12;
			break;
		case KEY1_PRES ://-1
			tmp_server_address[address_pointer] = (tmp_server_address[address_pointer] + 9)%10;
			break;
		case WKUP_PRES ://+1
			tmp_server_address[address_pointer] = (tmp_server_address[address_pointer] + 1)%10;
			break;
		default: break;
	}
	SHOW_IP(tmp_server_address);
}

u8 tmp_msg[]="0";
void key_pattern_two(u8 press)
{
	
	switch(press){
		case KEY0_PRES :
			tmp_msg[0]=BUTTON_THR;
			choose_pattern(BUTTON_THR);
			OSMboxPost(TOUCH_Box,&tmp_msg); //发送消息
			delay_ms(50);
			break;
		case KEY1_PRES :
			file_dir_pointer = (file_dir_pointer + file_dir_max - 1)%file_dir_max;
			print_dir();
			break;
		case WKUP_PRES :
			file_dir_pointer = (file_dir_pointer + 1)%file_dir_max;
			print_dir();
			break;
		default :break;
	}
}

void key_management(u8 press)
{
	switch(button_sta){
		case BUTTON_STA_THR :
			key_pattern_one(press);
			break;
		case BUTTON_STA_FIV :
			key_pattern_two(press);
			break;
		default: break;
	}
}

void SHOW_RES(u8 str[])
{
	LCD_Fill(0,30,120,35,BLACK);
	Show_Str(0,30,120,16,str,16,0,BRED,BLACK,0);
}

u8 path[50]="";
u8 save_data(u8 hour,u8 minute,u8 second)
{
	DIR dir;
	u8 offset=0,pos=0,len=0,id=1,data_len=0;
	float temperature=0;
	u8 *p = mymalloc(50);
	u8 res=0,i;
	
	for(i=0;i<14;++i)
		if( (USART2_RX_BUF[i] >= 'A' && USART2_RX_BUF[i] <= 'Z') 
			|| (USART2_RX_BUF[i] >= '0' && USART2_RX_BUF[i] <= '9') );
		else return 0;
	
	f_mount(fs[0],"0:",1);//挂载SD卡
	
	len = strlen(USART2_RX_BUF);
	USART2_RX_BUF[14]='\0';
	
	sprintf(path,"0:/Temperature/%04d/%02d/%02d/%s",calendar.w_year,calendar.w_month,calendar.w_date,USART2_RX_BUF);
	if(f_opendir(&dir,path) != FR_OK){//原先没有该文件夹，逐步创建
		sprintf(path,"0:/Temperature/%04d",calendar.w_year);
		f_mkdir(path);
		sprintf(path,"0:/Temperature/%04d/%02d",calendar.w_year,calendar.w_month);
		f_mkdir(path);
		sprintf(path,"0:/Temperature/%04d/%02d/%02d",calendar.w_year,calendar.w_month,calendar.w_date);
		f_mkdir(path);
		sprintf(path,"0:/Temperature/%04d/%02d/%02d/%s",calendar.w_year,calendar.w_month,calendar.w_date,USART2_RX_BUF);
		f_mkdir(path);
	}
	
	sprintf(path,"0:/Temperature/%04d/%02d/%02d/%s",calendar.w_year,calendar.w_month,calendar.w_date,USART2_RX_BUF);
	if(f_opendir(&dir,path) == FR_OK){
		if(USART2_RX_BUF[24] == '2') offset = 8;
		pos=31;
		while(pos < len){
			temperature = 100*(USART2_RX_BUF[pos]-48)+10*(USART2_RX_BUF[pos+1]-48)+(USART2_RX_BUF[pos+2]-48)+(USART2_RX_BUF[pos+4]-48)/10+(USART2_RX_BUF[pos+5]-48)/100;
			//printf("%f\r\n",temperature);
			sprintf(path,"0:/Temperature/%04d/%02d/%02d/%s/port%02d.txt",calendar.w_year,calendar.w_month,calendar.w_date,USART2_RX_BUF,id+offset);
			//printf("%s\n",path);
			if(f_open(&fp, path, FA_OPEN_ALWAYS | FA_WRITE) == FR_OK){
				if(f_lseek(&fp, fp.fsize) == FR_OK){//定位,定位到最后
					sprintf(p,"[%02d:%02d:%02d]%03.2f℃\r\n",hour,minute,second,temperature);
					data_len = strlen(p);
					//printf("%d  %s",data_len,p);
					if(f_write(&fp, p,data_len, &bw) == FR_OK)
						SHOW_RES("保存数据成功");
					else{
						SHOW_RES("保存数据失败");
						res = 8;
						break;
					}
				}
				else{
					SHOW_RES("定位数据失败");
					res = 6;
					break;
				}
			}
			else{
				SHOW_RES("打开失败2");
				res = 5;
				break;
			}
			f_close(&fp);
			pos += 13;  id++;
		}
	}
	
	//用完SD卡要重新初始化串口
	USART2_Init(36,230400);
	uart4_init(36,9600);
	//SPI_Flash_Init();
	
	return res;
}

void USART2_msg_management(u8 hour,u8 minute,u8 second)
{
	sim900a_send(USART2_RX_BUF);
	save_data(hour,minute,second);
}

//翻译数组
u8 Map[]={0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};
//状态机状态
#define head              1 //数据头
#define function          2 //功能位
#define data_length       3 //数据长度
#define read_data         4 //提取数据
#define check_and_end     5 //校验和数据尾

//功能
#define B1                1 //数据采集开关

void implement(u8 fuc,u8 str[])
{
	printf("%d--%s\r\n",fuc,str);
	switch(fuc){
		case B1 :
			u2_printf(" ");
			delay_ms(25);//防止与上一条信息冲突
			u2_printf("1#");
			break;
	}
}

void UART3_msg_management(u8 str[])
{
	u8 sta = head;
	u8 fuc,len,read_len,pointer=0;
	u8 i,check,h4,l4;
	u8 *p = mymalloc(50);
	len = strlen(str);
	while(pointer < len){
		switch(sta){
			case head :
				if(str[pointer] == 'A' && str[pointer+1] == 'A') sta=function;
				pointer += 2;
				break;
			case function :
				if(str[pointer] == 'B' && str[pointer+1] == '1') sta=data_length,fuc=B1;
				else sta=head;
				pointer += 2;
				break;
			case data_length :
				if( (str[pointer] >= '0' && str[pointer] <='9'
						|| str[pointer] >= 'A' && str[pointer] <='F')
					|| (str[pointer+1] >= '0' && str[pointer+1] <='9'
						|| str[pointer+1] >= 'A' && str[pointer+1] <='F') )
				{
					sta=read_data;
					h4 = str[pointer] - '0'; if(h4>10) h4-=7;// 'A' = 65,'0' = 48
					l4 = str[pointer+1]-'0'; if(l4>10) l4-=7;
					read_len = h4*16 + l4;
				}
				else sta=head;
				pointer += 2;
				break;
			case read_data :
				check = 0;
				for(i=0;i<read_len;++i){
					if( (str[pointer] >= '0' && str[pointer] <='9'
						|| str[pointer] >= 'A' && str[pointer] <='F')
					|| (str[pointer+1] >= '0' && str[pointer+1] <='9'
						|| str[pointer+1] >= 'A' && str[pointer+1] <='F') )
					{
						h4 = str[pointer] - '0'; if(h4>9) h4-=7;  h4 = Map[h4];//解密
						l4 = str[pointer+1]-'0'; if(l4>9) l4-=7;  l4 = Map[l4];
						p[i] = h4*16 + l4;
						pointer += 2;
						check ^= p[i];
						printf("%d %d\r\n",p[i],check);
					}
					else break;
				}
				if(i == read_len) sta = check_and_end,p[read_len]=0;
				else{
					sta = head;
					for(i=0;i<50;++i) p[i]=0;
				}
				break;
			case check_and_end :
				h4 = (check>>4)&15;  l4 = check&15;
				h4 += '0';  l4 += '0';
				if(h4>'9') h4 +=7;
				if(l4>'9') l4 ==7;
				
				if(str[pointer] == h4 && str[pointer+1] == l4
					&& str[pointer+2] == 'F' && str[pointer+3] == 'F' && str[pointer+4] == 'F')
				{
					implement(fuc,p);
					for(i=0;i<50;++i) p[i]=0; //防止两条指令重合
					sta = head;
				}
				pointer += 5;
				break;
			default :
				sta = head;
				break;
		}
	}
	USART3_RX_STA = 0;
	for(i=0;i<USART3_MAX_RECV_LEN;i++)USART3_RX_BUF[i]=0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 address[10][20]={0};//蓝牙从机地址
u8 num_of_address=0;//蓝牙从机地址数量
u8 now_address=0;//现在输出的从机地址
u8 port=1,pack=1;//制图的端口，哪个包
void match(u8 *tmp)
{
	u8 flag=0,i,j;
	if(strlen(tmp)!=14) return;
	for(i=0;i<14;++i)
		if( !( (tmp[i]>='A' && tmp[i]<='Z') || (tmp[i]>='0' && tmp[i]<='9') ) ) return;
	for(i=1;i<=num_of_address;++i){
		flag=0;
		for(j=0;j<14;++j){
			if(address[i][j]!=tmp[j]){
				flag=1;
				break;
			}
		}
		if(!flag) return;
	}
	num_of_address++;
	for(i=0;i<14;++i){
		address[num_of_address][i]=tmp[i];
	}
	address[num_of_address][14]=0;
	if(now_address==0) now_address=1;
	return;
}

u8 check(u8 *tmp)
{
	u8 i;
	for(i=0;i<14;++i)
		if(tmp[i]!=address[now_address][i]) return 1;
	if(tmp[24]-48!=pack) return 1;
	return 0;
}

u16 year=0,month=0,day=0;



// 模式	                描述
// FA_READ	            指定读访问对象。可以从文件中读取数据。与FA_WRITE结合可以进行读写访问。
// FA_WRITE	            指定写访问对象。可以向文件中写入数据。与FA_READ结合可以进行读写访问。
// FA_OPEN_EXISTING	    打开文件。如果文件不存在，则打开失败。(默认)
// FA_OPEN_ALWAYS	    如果文件存在，则打开；否则，创建一个新文件。此种方式可使用 f_lseek 函数对打开的文件追加数据。
// FA_CREATE_NEW	    创建一个新文件。如果文件已存在，则创建失败。
// FA_CREATE_ALWAYS	    创建一个新文件。如果文件已存在，则它将被截断并覆盖。

void main_task(void *pdata)
{
	u8 *buffer=mymalloc(1200);
	u16 buffer_pointer=0;
    u8 t,len,counter=0;
    u8 touch_err,*touch_msg,touch_command;
    u8 key_val,key_err,*key_msg;

    //u8 GSM_msg[]="0"; 
    u8 GSM_val,GSM_err,*GSM_msg;
    u8 *A = mymalloc(100);
    u8 *USART1_RX_BUF = mymalloc(100);
 	u8 c=0;
    u16 i,j,k,l,x=0,y=0,x0=0,y0=0,y1=0;
    float temp0=0,temp1=0;
     u8 file_path[]="0:/Temperature/2016-01-12.txt";
    static u8 first_open=TRUE;
    u8 p1[300]={0};
	u8 tmp_buff[300],pp[10];
    u8 data_len=0;
    u8 read_buf[READ_LEN]={0};
		u8 pos;
		u16 flag=8;
		float tmp1,tmp2;
		u16 adcx,ph;
		u8 count_down=0;
		u8 touch_val;

    static FIL file;		/* file objects */
//     static DIR dir;
//     const FILINFO* fno;

    static u8 *file_dir="0:/Temperature";
    
    LCD_Clear(BLACK);
    
    f_mkdir (file_dir);//创建文件夹
    //sprintf(file_path,"0:/Temperature/%04d-%02d-%02d.txt",calendar.w_year,calendar.w_month,calendar.w_date);
    //以上一行：更新默认文件名 查看最新的一天的
		year=calendar.w_year;
		month=calendar.w_month;
		day=calendar.w_date;
    f_open(&file, (const TCHAR*)file_path,FA_CREATE_NEW);
    f_close(&file);

	f_mount(fs[0],"0:",1);
     
     
    
    LCD_Fill(0,0,239,27,BLACK);
    Show_Str(0,0,240,16,"====分布式智能大棚调控系统====",16,0,BRED,BLACK,1);  
    //Show_Str(0,16,240,12,"锡马信息科技有限公司",12,0,YELLOW,BLACK);  
    LCD_DrawLine(0, 28, 239, 28,GREEN);	
    Show_Str(192,16,48,12,"CPU:000%",12,0,BRED,BLACK,0);
		
	  button_pattern_one(0,button_one);
		
	//RTC_Set( 2018, 6, 1, 18, 1,1);
    
    if(STA==START_SCAN)
    {
        for(i=0;i<table_x;i++)for(j=0;j<table_y;j++)table[i][j]=0;
        table_DrawLine(0,0,table_x-1,0,BRED);
        table_DrawLine(0,0,0,table_y-1,BRED); 
    }
    
	//每次读取文件后都要初始化串口2
	USART2_Init(36,230400);
		uart4_init(36,9600);
    
	while(1)
    {
        IWDG_Feed();//喂狗
			if(USART2_RX_STA & 0X8000 && STA != CONNECTING){
				if(USART2_RX_BUF[0] != 'U')
					USART2_msg_management(calendar.hour,calendar.min,calendar.sec);
				USART2_RX_STA=0;
			}
		
			touch_msg= (u8*)OSMboxPend(TOUCH_Box,1,&touch_err); //请求消息邮箱
			if(touch_msg !=NULL && touch_msg[0] != 0){
				first_press=FALSE;
            first_loosen=TRUE;
				printf("main------%d\n",touch_msg[0]);
			touch_val=touch_msg[0];
			}
			else if(touch_msg ==NULL && first_loosen==TRUE){
				first_loosen=FALSE;
            first_press=TRUE;
				button_management(touch_val);
			}
			key_msg= (u8*)OSMboxPend(KEY_Box,1,&key_err); //请求消息邮箱 
			if(key_msg!=NULL) key_management(key_msg[0]);
			
			if( STA != CONNECTING && USART3_RX_STA & 0X8000){
			printf("3---%s\n",USART3_RX_BUF);
			UART3_msg_management(USART3_RX_BUF);
			for(l=0;l<USART3_MAX_RECV_LEN;l++)USART3_RX_BUF[l]=0;
			USART3_RX_STA=0;
		}
			
			delay_ms(50);
				//USART2_msg_management();
			
//        if(t!=calendar.sec)
//        {
//            t=calendar.sec;
//            Show_Str(0,16,126,12,rtc_time_transform(),12,0,DARKBLUE,BLACK,0);
//        }
//        LCD_ShowxNum(216,16,OSCPUUsage,3,12,0X80,BRED,BLACK,0);
//        
//				
//				
//        key_msg= (u8*)OSMboxPend(KEY_Box,1,&key_err); //请求消息邮箱 
//        if(key_msg!=NULL)
//        {
//						f_mount(fs[0],"0:",1);
//            key_val=key_msg[0];
//            sprintf((char*)key_msg,"KEY:%d",key_val);
//            LCD_Fill(0,29,125,40,BLACK);
//            Show_Str(0,29,120,16,key_msg,12,0,GREEN,BLACK,0);
//						if(key_val==KEY0_PRES && STA==START_SCAN){
//							if(now_address){
//								now_address=now_address+1>num_of_address?1:num_of_address;
//								port=1,pack=1;
//								x0=0;
//								for(i=0;i<table_x;i++)for(j=0;j<table_y;j++)table[i][j]=0;
//								table_DrawLine(0,0,table_x-1,0,BRED);
//                 table_DrawLine(0,0,0,table_y-1,BRED);
//								LCD_Set_Window(0,280-table_y,table_x,table_y);
//								LCD_WriteRAM_Prepare();
//								for(j=0;j<table_y;j++)//写入数据
//								{
//										for(i=0;i<table_x;i++)
//										{
//												if(table[i][j])
//												{
//														if(i==0||j==table_y-1)LCD_WR_DATA(BRED)
//														else LCD_WR_DATA(GREEN)  
//												}
//												else LCD_WR_DATA(BLACK)
//										}
//								}
//								LCD_Set_Window(0,0,lcddev.width,lcddev.height);	//设置窗口
//								My_Show_Str(1,280-table_y-16,96,12,"---.--",12,0,8,BRED,BLACK,0);
//							}
//						}
//						if(key_val==KEY1_PRES && STA==START_SCAN && now_address){
//							port=port+1>16?1:port+1;
//							pack=port>8?2:1;
//							x0=0;
//							for(i=0;i<table_x;i++)for(j=0;j<table_y;j++)table[i][j]=0;
//							table_DrawLine(0,0,table_x-1,0,BRED);
//              table_DrawLine(0,0,0,table_y-1,BRED);
//							LCD_Set_Window(0,280-table_y,table_x,table_y);
//							LCD_WriteRAM_Prepare();
//							for(j=0;j<table_y;j++)//写入数据
//							{
//									for(i=0;i<table_x;i++)
//									{
//											if(table[i][j])
//											{
//													if(i==0||j==table_y-1)LCD_WR_DATA(BRED)
//													else LCD_WR_DATA(GREEN)  
//											}
//											else LCD_WR_DATA(BLACK)
//									}
//							}
//							LCD_Set_Window(0,0,lcddev.width,lcddev.height);	//设置窗口
//							My_Show_Str(1,280-table_y-16,96,12,"---.--",12,0,8,BRED,BLACK,0);
//						}
//						/*if(key_val==KEY0_PRES && STA==READ_FILE){
//							OSMboxPost(GSM_Box,&key_val);
//							//LCD_ShowString(40,150,100,16,16,"SEND!",WHITE,BLACK,0);
//						}*/
//						if(STA==READ_FILE && key_val==KEY1_PRES){
//							switch(flag){
//								case 0: year=(year+1000)%10000; break;
//								case 1: year=(year/1000*1000)+(year%100)+(year/100%10*100+100)%1000; break;
//								case 2: year=(year/100*100)+(year%10)+(year/10%10*10+10)%100; break;
//								case 3: year=(year/10*10)+(year%10+1)%10; break;
//								case 4: month=(month+10)%20; break;
//								case 5: month=(month/10*10)+(month%10+1)%10; break;
//								case 6: day=(day+10)%40; break;
//								case 7: day=(day/10*10)+(day%10+1)%10; break;
//								case 8:{
//									sprintf(file_path,"0:/Temperature/%04d-%02d-%02d.txt",year,month,day);
//									//touch_msg[0]=READ_FILE;
//									//OSMboxPost(TOUCH_Box,&touch_msg); //发送消息
//									printf("%s\n",file_path);
//									printfile(file_path,read_buf,file);
//									flag=10;
//									break;
//								}
//							}
//						}
//						if(key_val==KEY0_PRES){
//							if(flag<9) flag=(flag+1)%9;
//							else flag=0;
//						}
//						printf("falg is %d\n",flag);
//						if(STA==READ_FILE && flag<9){
//							if(flag==0) LCD_ShowNum(110,41,year/1000,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(110,41,year/1000,1,16,BRED,BLACK,0);
//							if(flag==1) LCD_ShowNum(118,41,year/100%10,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(118,41,year/100%10,1,16,BRED,BLACK,0);
//							if(flag==2) LCD_ShowNum(126,41,year/10%10,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(126,41,year/10%10,1,16,BRED,BLACK,0);
//							if(flag==3) LCD_ShowNum(134,41,year%10,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(134,41,year%10,1,16,BRED,BLACK,0);
//							if(flag==4) LCD_ShowNum(150,41,month/10,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(150,41,month/10,1,16,BRED,BLACK,0);
//							if(flag==5) LCD_ShowNum(158,41,month%10,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(158,41,month%10,1,16,BRED,BLACK,0);
//							if(flag==6) LCD_ShowNum(174,41,day/10,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(174,41,day/10,1,16,BRED,BLACK,0);
//							if(flag==7) LCD_ShowNum(182,41,day%10,1,16,BRED,LIGHTBLUE,0);
//							else LCD_ShowNum(182,41,day%10,1,16,BRED,BLACK,0);
//							if(flag==8) Show_Str(200,41,40,16,"确认",16,0,BRED,LIGHTBLUE,0);
//							else Show_Str(200,41,40,16,"确认",16,0,BRED,BLACK,0);
//						}
//						USART2_Init(36,230400);
//						uart4_init(36,9600);
//        }
//				
//		
//        touch_msg= (u8*)OSMboxPend(TOUCH_Box,1,&touch_err); //请求消息邮箱
//				if(STA==CONNECTING) continue;
//        if(touch_msg!=NULL && first_press==TRUE)
//        {    
//            first_press=FALSE;
//            first_loosen=TRUE;
//            RED_LED=0;
//            touch_command=touch_msg[0];
//            //LCD_Fill(0,29,125,40,BLACK);
//            
//            switch(touch_command)
//            {
//                case START_SCAN:
//                    {
//                        
//						sprintf(pp,"%c#",START_SCAN);
//                        u2_printf(pp);
//						
//						f_mount(fs[0],"0:",1);
//                        Show_Str(0,29,120,16,"采集中...",12,0,BRED,BLACK,0);
//                        STA=START_SCAN;
//                        LCD_Fill(0,288,63,293,LIGHTBLUE);
//                        Show_Str(0,294,120,16,"启动采集",16,0,YELLOW,LIGHTBLUE,0);
//                        LCD_Fill(0,310,63,319,LIGHTBLUE);
//                        LCD_Fill(0,41,239,286,BLACK);
//						
//                        LCD_DrawLine(0,65,239,65,GREEN);
//                        
//                        //先把之前的数据恢复
//                        LCD_Set_Window(0,280-table_y,table_x,table_y);
//                        LCD_WriteRAM_Prepare();
//                        for(j=0;j<table_y;j++)//写入数据
//                        {
//                            for(i=0;i<table_x;i++)
//                            {
//                                if(table[i][j])
//                                {
//                                    if(i==0||j==table_y-1)LCD_WR_DATA(BRED)
//                                    else LCD_WR_DATA(GREEN)  
//                                }
//                                else LCD_WR_DATA(BLACK)
//                            }
//                        }
//                        LCD_Set_Window(0,0,lcddev.width,lcddev.height);	//设置窗口
//                        My_Show_Str(1,280-table_y-16,96,12,"---.--℃",12,0,8,BRED,BLACK,0);
//                        
//						USART2_Init(36,230400);
//						uart4_init(36,9600);
//						break;
//                    }
//                case READ_FILE:
//                    {
//						sprintf(pp,"%c#",STOP_SCAN);
//						u2_printf(pp);
//						f_mount(fs[0],"0:",1);
//                        Show_Str(0,29,120,16,"读取文件",12,0,BRED,BLACK,1);
//                        STA=READ_FILE;
//                        LCD_Fill(88,288,151,293,LIGHTBLUE);
//                        Show_Str(88,294,120,16,"读取文件",16,0,YELLOW,LIGHTBLUE,0);
//                        LCD_Fill(88,310,151,319,LIGHTBLUE);
//                        
//												flag=10;
//                        printfile(file_path,read_buf,file);
//						//每次读取完文件后都要初始化串口2和挂载SD卡
//						USART2_Init(36,230400);
//						uart4_init(36,9600);
//                        break;    
//                    }
//                    
//                case STOP_SCAN:
//                    {
//						sprintf(pp,"%c#",STOP_SCAN);
//                        u2_printf(pp);
//						
//						f_mount(fs[0],"0:",1);
//                        Show_Str(0,29,120,16,"停止采集",12,0,BRED,BLACK,0);
//                        STA=STOP_SCAN;
//                        LCD_Fill(176,288,239,293,LIGHTBLUE);
//                        Show_Str(176,294,120,16,"停止采集",16,0,YELLOW,LIGHTBLUE,0);
//                        LCD_Fill(176,310,239,319,LIGHTBLUE);
//                        //LCD_Fill(0,41,239,286,BLACK);
//                        LCD_DrawLine(0,65,239,65,GREEN);
//						
//						USART2_Init(36,230400);
//						uart4_init(36,9600);
//                        break;
//                    }
//                    
//                default:
//                {
//					f_mount(fs[0],"0:",1);
//                    LCD_Fill(0,288,63,295,GRAY);
//                    Show_Str(0,296,120,16,"启动采集",16,0,YELLOW,GRAY,0);
//                    LCD_Fill(0,312,63,319,GRAY);
//                    
//                    LCD_Fill(88,288,151,295,GRAY);
//                    Show_Str(88,296,120,16,"读取文件",16,0,YELLOW,GRAY,0);
//                    LCD_Fill(88,312,151,319,GRAY);
//                    
//                    LCD_Fill(176,288,239,295,GRAY);
//                    Show_Str(176,296,120,16,"停止采集",16,0,YELLOW,GRAY,0);
//                    LCD_Fill(176,312,239,319,GRAY);
//                    USART2_Init(36,230400);
//									uart4_init(36,9600);
//                    break;
//                }

//            }
//			f_mount(fs[0],"0:",1);
//            SPI_Flash_Write(&STA,sta_address,1);
//			USART2_Init(36,230400);
//			uart4_init(36,9600);
//			
//        }
//        else if(touch_msg==NULL && first_loosen==TRUE)
//        {

//            first_loosen=FALSE;
//            first_press=TRUE;
//            RED_LED=1;
//            
//						f_mount(fs[0],"0:",1);
//            LCD_Fill(0,288,63,295,GRAY);
//            Show_Str(0,296,120,16,"启动采集",16,0,YELLOW,GRAY,0);
//            LCD_Fill(0,312,63,319,GRAY);
//            
//            
//            LCD_Fill(88,288,151,295,GRAY);
//            Show_Str(88,296,120,16,"读取文件",16,0,YELLOW,GRAY,0);
//            LCD_Fill(88,312,151,319,GRAY);
//            
//            LCD_Fill(176,288,239,295,GRAY);
//            Show_Str(176,296,120,16,"停止采集",16,0,YELLOW,GRAY,0);
//            LCD_Fill(176,312,239,319,GRAY);
//            
//            USART2_Init(36,230400);
//						uart4_init(36,9600);
//        }
//        
//		//收到数据
//		if(USART2_RX_STA & 0X8000 && connect_sta==1){
//			for(l=0;l<14;++l)
//				tmp_buff[l]=USART2_RX_BUF[l];
//			tmp_buff[14]=0;
//			//LCD_ShowString(60,140,120,16,16,tmp_buff,BLACK,WHITE,0);
//			match(tmp_buff);
//			f_mount(fs[0],"0:",1);
//			if(STA==START_SCAN){
//			LCD_Fill(0+120*x,66+108*y,114+120*x,66+108*(y+1),BLACK);//清除显示区域
//            My_Show_Str(0+120*x,66+108*y,114,108,USART2_RX_BUF+2,12,0,19,point_color[c],BLACK,0); //显示设备地址和电量信息
//            My_Show_Str(78+120*x,78+108*y,114,108,USART2_RX_BUF+21,12,0,5,point_color[c],BLACK,0);//显示数据包序号
//			
//			k=0;
//            while(k<14)
//            {
//                if(USART2_RX_BUF[26+13*k]=='[')
//                {
//                    My_Show_Str(0+120*x,78+108*y+12*k,114,103,USART2_RX_BUF+26+13*k,12,0,13,point_color[c],BLACK,0);
//                    //以上一行表示逐行显示温度数据
//                    k++;
//                }
//                else break;
//            } 
//            x++;
//            if(x>1)x=0;
//					}
//			
//			REOPEN:sprintf((char*)file_path,"0:/Temperature/%04d-%02d-%02d.txt",calendar.w_year,calendar.w_month,calendar.w_date);
//            if(f_open(&file, (const TCHAR*)file_path, FA_OPEN_ALWAYS | FA_WRITE) == FR_OK)
//            {
//                if(f_lseek(&file, file.fsize) == FR_OK)//定位,定位到最后
//                {
//                    data_len=0;
//                    strcpy(p1,USART2_RX_BUF);
//                    while(data_len<USART2_MAX_RECV_LEN)
//                    {
//                        if(USART2_RX_BUF[data_len]==0)break;
//                        else data_len++;
//                    }
//                    
//                //数据封装和转移
//					sprintf(p1,"[%02d:%02d:%02d]%s\r\n",calendar.hour,calendar.min,calendar.sec,USART2_RX_BUF);
//                 
//					if(f_write(&file, p1,data_len+12, &bw) == FR_OK)//数据保存到SD卡
//					{
//						if(STA==START_SCAN){
//							Show_Str(0,41,240,12,"写入数据成功",12,0,BRED,BLACK,0);  
//						Show_Str(0+80,41,240,12,file_path+15,12,0,BRED,BLACK,0); 
//						Show_Str(0,53,240,12,"数据大小:   字节",12,0,BRED,BLACK,0);                            
//						LCD_ShowxNum(54,53,data_len,3,12,0,BRED,BLACK,1);				//显示 数字
//							}
//					}
//					else
//					{
//						if(STA==START_SCAN)
//						{Show_Str(0,41,240,12,"写入数据失败",12,0,BRED,BLACK,0);  
//						Show_Str(0+80,41,240,12,file_path+15,12,0,BRED,BLACK,0); 
//						Show_Str(0,53,240,12,"               ",12,0,BRED,BLACK,0);
//						}
//					}
//                }
//                else
//                {
//                    if(STA==START_SCAN){
//											Show_Str(0,41,240,12,"定位数据失败",12,0,BRED,BLACK,0);  
//                    Show_Str(0+80,41,240,12,file_path+15,12,0,BRED,BLACK,0); 
//                    Show_Str(0,53,240,12,"               ",12,0,BRED,BLACK,0);
//										}
//								}
//				        
//            }
//            else
//            {
//                if(STA==START_SCAN)
//								{Show_Str(0,42,240,12,"打开文件失败",12,0,BRED,BLACK,0);
//                Show_Str(0+80,42,240,12,file_path+15,12,0,BRED,BLACK,0);  
//                Show_Str(0,53,240,12,"               ",12,0,BRED,BLACK,0);
//                }    
//                f_mount(fs[0],"0:",1); 	//挂载SD卡 
//                SD_Initialize();
//                if(first_open==TRUE)
//                {
//                    first_open=FALSE;
//                    f_close(&file);
//                    goto REOPEN;
//                }
//                    
//            }
//            f_close(&file);
//			
//			//提取部分数据出来制作图表
//					if(STA==START_SCAN){
//            LCD_ShowString(130,215,100,16,12,address[now_address],BRED,BLACK,0);
//						//LCD_ShowNum(200,180,num_of_address,5,16,BLACK,WHITE,0);
//						LCD_ShowNum(215,215,port,3,12,BRED,BLACK,0);
//            if(!check(USART2_RX_BUF))//以蓝牙从设备地址来过滤，现在暂时获取0XD4F51378E987 的第一个温度数据来制作图标
//            {
//								pos=31+(port-(pack-1)*8-1)*12;
//								len=strlen(USART2_RX_BUF);
//								
//								if(pos+5>=len) continue;//没有这个端口的数据
//								
//                temp0=temp1;
//                temp1=100*(USART2_RX_BUF[pos]-48)+10*(USART2_RX_BUF[pos+1]-48)+(USART2_RX_BUF[pos+2]-48)+(USART2_RX_BUF[pos+4]-48)/10+(USART2_RX_BUF[pos+5]-48)/100;//提取温度	
//							
//                if(!x0)
//                {
//                    for(i=0;i<table_x;i++)for(j=0;j<table_y;j++)table[i][j]=0;
//                    table_DrawLine(0,0,table_x-1,0,BRED);
//                    table_DrawLine(0,0,0,table_y-1,BRED);
//                }
//                x0+=5;
//                if(x0>table_x-5)
//                {
//                    x0=5;
//                    for(i=0;i<table_x;i++)for(j=0;j<table_y;j++)table[i][j]=0;
//                    table_DrawLine(0,0,table_x-1,0,BRED);
//                    table_DrawLine(0,0,0,table_y-1,BRED);
//                }
//															
//								
//                if(temp0>50)temp0=50;
//                if(temp1>50)temp1=50;
//                y0=table_y*((temp0-10)/40);
//                y1=table_y*((temp1-10)/40);
//                table_DrawLine(x0-5,y0,x0,y1,GREEN);
//                
//                LCD_Set_Window(0,280-table_y,table_x,table_y);
//                LCD_WriteRAM_Prepare();
//                for(j=0;j<table_y;j++)//写入数据
//                {
//                    for(i=0;i<table_x;i++)
//                    {
//                        if(table[i][j])
//                        {
//                            if(i==0||j==table_y-1)LCD_WR_DATA(BRED)
//                            else LCD_WR_DATA(GREEN)  
//                        }
//                        else LCD_WR_DATA(BLACK)
//                    }
//                }
//                LCD_Set_Window(0,0,lcddev.width,lcddev.height);	//设置窗口
//                My_Show_Str(1,280-table_y-16,96,12,USART2_RX_BUF+pos,12,0,8,BRED,BLACK,0);
//            }
//					}
//			
//			
//			sprintf(tmp_buff,"[%04d-%02d-%02d][%02d:%02d:%02d]%s\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec,USART2_RX_BUF);
//			printf("2-%d-%d--%s\n",connect_sta,buffer_pointer,tmp_buff);
//			len=strlen(tmp_buff);
//			if(buffer_pointer+len>=1024 && connect_sta){
//				sim900a_send(buffer);
//				buffer_pointer=0;
//			}
//			if(connect_sta){
//				buffer[buffer_pointer++]='[';
//				buffer[buffer_pointer++]='+';
//				buffer[buffer_pointer++]='0';
//				buffer[buffer_pointer++]=']';
//				for(l=0;l<len;l++) buffer[buffer_pointer++]=tmp_buff[l];
//			}
//			for(l=0;l<USART2_MAX_RECV_LEN;l++)USART2_RX_BUF[l]=0;
//			USART2_RX_STA=0;
//			
//			USART2_Init(36,230400);
//			uart4_init(36,9600);
//		}
//		
//		if(connect_sta==1 && UART4_RX_STA & 0x8000 && count_down==0){
//			len=UART4_RX_STA&0x3fff;//得到此次接收到的数据长度
//			/*for(l=0;l<len;++l)
//				printf("4---%d ",UART4_RX_BUF[l]);*/
//			if(UART4_RX_BUF[0]+UART4_RX_BUF[1]+UART4_RX_BUF[2]==UART4_RX_BUF[3]){
//				
//				
//				l=UART4_RX_BUF[1]*256+UART4_RX_BUF[2];
//				sprintf(tmp_buff,"AIR_Q = %02d.%1d",l/10,l%10);
//				if(STA==START_SCAN) LCD_ShowString(120,200,80,12,12,tmp_buff,BRED,BLACK,0);
//				sprintf(tmp_buff,"[+%1d][%04d-%02d-%02d][%02d:%02d:%02d]:[%02d][%1d]\r\n",1,calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec,l/10,l%10);
//				//printf("4---%s\n",tmp_buff);
//				sim900a_send(tmp_buff);
//			}
//			for(l=0;l<UART4_REC_LEN;l++)UART4_RX_BUF[l]=0;
//			UART4_RX_STA=0;
//		}
//    
//		if(connect_sta && USART3_RX_STA & 0X8000){
//			printf("3---%s\n",USART3_RX_BUF);
//			for(l=0;l<USART3_MAX_RECV_LEN;l++)USART3_RX_BUF[l]=0;
//			USART3_RX_STA=0;
//		}
//		if(connect_sta==1 && count_down==0){
//			adcx=Get_Adc_Average(ADC_CH1,10);
//			tmp1=(float)adcx*(3.3/4096);
//			tmp2=(float)(-5.9647)*tmp1+22.255;
//			tmp2-=5;
//			//printf("ph=%f\n",tmp2);
//			if(tmp2>=0 && tmp2<=14){
//				adcx=tmp2;
//				ph=tmp2*100;
//				ph=ph%100;
//				sprintf(tmp_buff,"PH = %02d.%02d",adcx,ph);
//				if(STA==START_SCAN) LCD_ShowString(30,200,80,12,12,tmp_buff,BRED,BLACK,0);
//				sprintf(tmp_buff,"[+%1d][%04d-%02d-%02d][%02d:%02d:%02d]:[%02d][%02d]\r\n",2,calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec,adcx,ph);
//				//printf("5---%s\n",tmp_buff);
//				sim900a_send(tmp_buff);
//			}
//		}
//				count_down=(count_down+1)%50;
//        delay_ms(50);
    }


}



void gui_task(void *pdata)
{
    u8 res;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息	
	u8 *fn;	 
    
    _listbox_obj * txt_list_box=NULL;
    gui_init();
    
    		//释放内存   
    while(1)
    {
        //listbox_draw_listbox(txt_list_box);
        
        res=f_opendir(&tdir,"0:/Temperature"); //打开目录
        tfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
        tfileinfo.lfname=mymalloc(tfileinfo.lfsize);	//为长文件缓存区分配内存
        if(res==FR_OK&&tfileinfo.lfname!=NULL)
        {
            txt_list_box=listbox_creat(0,66,239,200,0,12);
            while(1)//查询总的有效文件数
            {
                res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件（打开文件夹后，每读一次读一个文件，直到最后）
                if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
                fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
                res=f_typetell(fn);	
                if(res==T_TEXT)//txt文件	
                {
                    listbox_addlist(txt_list_box,fn);
                    listbox_draw_listbox(txt_list_box);
                    delay_ms(1000);  
                    //listbox_delete(txt_list_box);
                }
                //test_prgb(0,42,160,12,0x61,i++%100);
                              
            }
            listbox_delete(txt_list_box);//
             
        } 
        myfree(tfileinfo.lfname); 	
        
        delay_ms(1000);
        delay_ms(1000);

    }


}

void bar_task(void *pdata)
{
    u8 i=0,flage=1;
    gui_init();
    
    while(1)
    {
        if(flage)
        {
            i++; 
            if(i>180)flage=0;
        }
        else 
        {
            i--;
            if(i==0)flage=1;
        }
        //test_prgb(0,48,160,12,0x61,i);
        //test_prgb(0,70,230,160,0xE0,i);
        draw_progressbar(0,70,9,280,i,16,0X07,RED,GREEN,BLACK,GRAY);
        draw_progressbar(20,70,29,280,i/2,5,0X07,YELLOW,GREEN,BLACK,RED);
        draw_progressbar(40,70,49,280,i/4,5,0X07,RED,GREEN,BLUE,YELLOW);
        delay_ms(20);
    }


}










