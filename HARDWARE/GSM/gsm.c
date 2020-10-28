#include "gsm.h"
#include "usart.h"
#include "delay.h"
#include "malloc.h"
#include "string.h"
#include "usart2.h"
#include "common.h"
#include "usart.h"
#include "includes.h"

static INT32U br,bw;
/////////////////////////////////////////////////////////////////////////////////
//sim900a发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向sim900a发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel2->CNDTR!=0);	//等待通道2传输完成   
		USART3->DR=(u32)cmd;
	}else u3_printf("%s\r\n",cmd);//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(sim900a_check_cmd(ack))break;//得到有效数据 
				USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	USART3_RX_STA=0;
	return res;
} 


/////////////////////////////////////////////////////////////////////////////////////////////
static u8 sim_sta=0;	//SIM卡状态：1即准备好
static u8 connect=0;//TCP连接状态：1即已连接

//x,y显示区域左上角坐标，flag是否显示
u8 sim900a_check_sim(u16 x,u16 y,u8 flag)
{
	u8 *p,*p1,*p2;
	u8 res=0;
	p=mymalloc(50);//申请50字节内存
	
	if(flag){//需要显示则清屏
		LCD_Fill(0,41,239,286,BLACK);
		LCD_DrawLine(0,65,239,65,GREEN);
	}
	if(sim900a_send_cmd("AT+CPIN?","OK",200))//查询SIM卡是否在位 
		res|=1<<0;
	printf("%s\n",USART3_RX_BUF);//输出到串口
	
	if(sim900a_send_cmd("AT+COPS?","OK",200)==0)		//查询运营商名字
	{
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\"");
		if(p1)//有有效数据
		{
			p2=(u8*)strstr((const char*)(p1+1),"\"");
			p2[0]=0;//加入结束符			
			if(flag) sprintf((char*)p,"CARRIER:%s",p1+1);
			Show_Str(x,y,200,16,p,16,0,WHITE,BLACK,0);
		}
	}else res|=1<<1;
	
	if(sim900a_send_cmd("AT+CSQ","+CSQ:",200)==0)		//查询信号质量
	{ 
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),":");
		p2=(u8*)strstr((const char*)(p1),",");
		p2[0]=0;//加入结束符
		sprintf((char*)p,"SIGNAL QUALITY:%s",p1+2);
		if(flag) Show_Str(x,y+20,200,16,p,16,0,WHITE,BLACK,0);	
	}else res|=1<<2;
	
	if(sim900a_send_cmd("AT+CBC","+CBC:",200)==0)		//查询电池电量
	{ 
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),",");
		p2=(u8*)strstr((const char*)(p1+1),",");
		p2[0]=0;p2[5]=0;//加入结束符
		sprintf((char*)p,"BATTRY:%s%%  %smV",p1+1,p2+1);
		if(flag) Show_Str(x,y+40,200,16,p,16,0,WHITE,BLACK,0);	
	}else res|=1<<3;
	
	if(sim900a_send_cmd("AT+CCLK?","+CCLK:",200)==0)		//查询日期
	{ 
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\"");
		p2=(u8*)strstr((const char*)(p1+1),":");
		p2[3]=0;//加入结束符
		sprintf((char*)p,"DATE:%s",p1+1);
		if(flag) Show_Str(x,y+60,200,16,p,16,0,WHITE,BLACK,0);
	}else res|=1<<4;
	
	myfree(p);
	return res;
}

//设置好连接前的AT指令
//返回值：0，设置成功
//				else，设置失败
u8 sim900a_set_AT(void)
{
	
	sim900a_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100);	//关闭连接
	printf("%s",USART3_RX_BUF);
	sim900a_send_cmd("AT+CIPSHUT","SHUT OK",100);		//关闭移动场景 
	printf("%s",USART3_RX_BUF);
	
	if(sim900a_send_cmd("AT+CGCLASS=\"B\"","OK",1000)){
		printf("%s",USART3_RX_BUF);
		return 2;//设置GPRS移动台类别为B,支持包交换和数据交换 
	}
	if(sim900a_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000)){
		printf("%s",USART3_RX_BUF);
		return 3;//设置PDP上下文,互联网接协议,接入点等信息
	}
	if(sim900a_send_cmd("AT+CGATT=1","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 4;//附着GPRS业务
	}
	if(sim900a_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 5;//设置为GPRS连接模式
	}
	if(sim900a_send_cmd("AT+CIPHEAD=1","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 6;//设置接收数据显示IP头(方便判断数据来源)
	}
	//下面设置透传模式
	if(sim900a_send_cmd("AT+IFC=2,2","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 7;
	}
	if(sim900a_send_cmd("AT+CIPMODE=1","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 8;
	}
	if(sim900a_send_cmd("AT+CIPCCFG=5,2,1024,1","OK",500)){//1024规定每次传输最大不超过1024
		printf("%s",USART3_RX_BUF);
		return 9;
	}
	
	return 0;
}

static u8 ip[16]="111.229.170.131";//服务器ip
static u8 port[6]="8088";//端口
static u8 ip_pointer=0;
//与服务器连接
//返回值：0，连接成功
//				1，连接失败
u8 sim900a_connect(u8 flag)
{
	u16 fail=0;
	u8 *p=mymalloc(50);
	
	//退出透传模式
	delay_ms(1500);
	u3_printf("+++");
	delay_ms(1500);
	
	while(sim_sta == 0 && fail < 3){//查询并显示sim卡信息，允许3次失败
		if(sim900a_check_sim(20,70,flag) == 0)
			sim_sta = 1;
		else sim_sta=0,++fail;
		delay_ms(1000);
	}
	if(sim_sta == 0){
		LCD_ShowString(20,150,200,16,16,"CAN'T FIND SIM CARD",WHITE,BLACK,0);
		return 1;
	}
	
	fail=0;
	while(sim900a_set_AT() && fail < 3){//设置连接前的AT指令，允许3次失败
		++fail;
		delay_ms(1000);
	}
	if(fail >= 3){
		LCD_ShowString(20,150,200,16,16,"SET AT FAIL",WHITE,BLACK,0);
		return 1;
	}
	
	//开始连接
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",ip,port);
	while(connect == 0 && fail < 5)//允许5次失败
	{
		if(sim900a_send_cmd(p,"OK",500))//发起连接
			connect=0,printf("%s",USART3_RX_BUF);
		else connect=1,printf("%s",USART3_RX_BUF);
		delay_ms(500);
	}
	fail=300;
	while(fail){
		if(USART3_RX_STA & 0X8000){//返回CONNECT即连接成功，60s没收到视为连接失败
			if(sim900a_check_cmd("CONNECT FAIL")){
				USART3_RX_STA=0;
				return 0;
			}
			else if(sim900a_check_cmd("CONNECT")){
				USART3_RX_STA=0;
				connect=1;
				break;
			}
		}
		delay_ms(200);
		printf("--%d\n",fail);//输出到串口
		--fail;
	}
	if(connect == 0){
		LCD_ShowString(20,150,200,16,16,"CONNECT FAIL",WHITE,BLACK,0);
		return 1;
	}
	else LCD_ShowString(20,150,200,16,16,"CONNECT SUCCEED",WHITE,BLACK,0);
	return 0;
}

u8 sim900a_star_connect(u8 flag,u8 IP[],u8 PORT[])
{
	u8 fail=0;
	sprintf(ip,"%d.%d.%d.%d",IP[0],IP[1],IP[2],IP[3]);
	sprintf(port,"%02d%02d",PORT[0],PORT[1]);
	printf("%s   %s\n",ip,port);
	
	connect = 0;
	while(connect==0 && ++fail<=3)//3次连接不上就返回
		if(sim900a_connect(flag)==0);
		else delay_ms(1000);
	delay_ms(3000);
	if(flag){
		LCD_Fill(0,41,239,286,BLACK);
		LCD_DrawLine(0,65,239,65,GREEN);
	}
	return connect;
}

///////////////////////////////////////////////////////////////////////////////////////////////
u8 Hash[]={0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15}; //字典
/*u8 sim900a_send(u8 *p)
{
	u8 check=0;
	u16 len,i;
	len=strlen(p);
	printf("%s\n",p);
	u3_printf("%1X%1X%1X%1X%1X%1X%1X%1X",Hash[(170>>4)&15],Hash[170&15],Hash[1>>4],Hash[1&15],Hash[(len>>12)&15],Hash[(len>>8)&15],Hash[(len>>4)&15],Hash[len&15]);
	check^=170;  check^=1;  check^=(len>>8)&255;  check^=len&255;
	for(i=0;i<len;++i){
		u3_printf("%1X%1X",Hash[(p[i]>>4)&15],Hash[p[i]&15]);
		check^=p[i];
	}//u3_printf("%s\n",p);
	u3_printf("%1X%1X",Hash[(255>>4)&15],Hash[255&15]);
	check^=255;
	u3_printf("%1X%1X\n",Hash[(check>>4)&15],Hash[check&15]);
	delay_ms(50);
	return 0;
}*/

u8 sim900a_send(u8 *p)
{
	u8 check=0,len,i;
	len=strlen(p);
	printf("send---%s\r\n",p);
	u3_printf("%2X%2X",0xAA,0xB4);//数据头AA，主动传输数据B4
	u3_printf("%02X",len);//数据长度
	for(i=0;i<len;++i){
		u3_printf("%1X%1X",Hash[(p[i]>>4)&15],Hash[p[i]&15]);//数据，加密
		check^=p[i];
	}
	u3_printf("%02X%3X",check,0xFFF);
	return 0;
}

/*
//返回0：成功
//返回1：失败
u8 work()
{
	u8 read_buf[READ_LEN]={0};
	static FIL file;
	u8 *p=mymalloc(450);
	static u8 file_path[]="0:/Temperature/2016-01-18.txt";  										//文件名
	static u8 *file_dir="0:/Temperature";
	u8 *p1=mymalloc(100);	//申请100字节内存;
	u8 *p2=mymalloc(100);	//申请100字节内存;
	u8 fail=0,done=0;
	u16 size=0,i=0,len=0;
	
	u8 send_err;
	u8 *key=0;

	f_mkdir (file_dir);//创建文件夹
	
	while(done==0)
	{
		//发送文件
		f_mount(fs[0],"0:",1);	//挂载SD卡
		if( f_open(&file, (const TCHAR*)file_path, FA_OPEN_ALWAYS | FA_READ) ==FR_OK)
		{
			size=f_size(&file);  f_close(&file);
			LCD_ShowString(10,170,100,16,16,"Total size:",BLACK,WHITE,0);
			LCD_ShowString(10,190,100,16,16,"Sended size:",BLACK,WHITE,0);
			LCD_ShowNum(140,170,size,5,16,BLACK,WHITE,0);
			LCD_ShowNum(140,190,i,5,16,BLACK,WHITE,0);
			//数据头AA；len数据长度；read_buf有效数据；FF数据尾；01校验位
			sprintf((char*)p,"%04d20160108",size);
			sim900a_send(p);
//			String_to_16Num(p);
//			printf("%s\n",p);
			while(1)
			{
				if(i==size) break;  //文件发送完毕
				if(f_open(&file, (const TCHAR*)file_path, FA_OPEN_ALWAYS | FA_READ) !=FR_OK){//文件打开失败
					LCD_ShowString(60,180,100,16,16,"OPEN FAIL",BLACK,WHITE,0);
					return 1;
				}
				if(f_lseek(&file,i) != FR_OK){
					LCD_ShowString(60,180,100,16,16,"LOCATE FAIL",BLACK,WHITE,0);//文件定位失败
					return 1;
				}
				if(f_read(&file,read_buf,READ_LEN,&br) != FR_OK){
					LCD_ShowString(60,180,100,16,16,"READ FAIL",BLACK,WHITE,0);//文件读取失败
					return 1;
				}
				
				p1=(u8*)strstr((const char*)read_buf,"\n");//每次截取一行发送
				p1[0]=0;
				len=strlen(read_buf);
				sprintf((char*)p,"[2016-01-08]%s",read_buf);
				i+=(p1-read_buf)+1;//计算文件指针偏移量
				LCD_ShowNum(140,190,i,5,16,BLACK,WHITE,0);
				f_close(&file);
				delay_ms(10);
				sim900a_send(p);
				
//				String_to_16Num(p);
//				printf("%s\n",p);
			}
			done=1;
		}
		else{
			Show_Str(60,180,100,16,"OPEN FAIL",16,0,WHITE,BLACK,0);//文件打开失败
			return 1;
		}
		
		//每次读取文件后都要初始化串口2
		USART2_Init(36,230400);
		delay_ms(50);
	}
	//Show_Str(150,180,100,16,"DONE",16,0,WHITE,BLACK,0);
	return 0;//发送完成
}


//字符串转化成16进制数
void String_to_16Num(u8 *p)
{
	u8 *str=mymalloc(300),check=0;
	u16 tmp=0,i=0;
	u16 len=strlen(p);
	sprintf((char*)str,"%s",p);
	//printf("%s\n",str);
	//0XAA  数据头
	p[tmp]=10;  check^=p[tmp++];
	p[tmp]=10;  check^=p[tmp++];
	//0X01  发送数据
	p[tmp]=0;  check^=p[tmp++];
	p[tmp]=1;  check^=p[tmp++];
	//数据长度 4位
	p[tmp]=(len>>12)&7;  check^=p[tmp++];
	p[tmp]=(len>>8)&15;  check^=p[tmp++];
	p[tmp]=(len>>4)&15;  check^=p[tmp++];
	p[tmp]=len&15;  check^=p[tmp++];
	for(i=0;i<len;++i){
		p[tmp]=(str[i]>>4)&15;  check^=p[tmp++];
		p[tmp]=str[i]&15;  check^=p[tmp++];
	}
	//数据尾
	p[tmp]=15;  check^=p[tmp++];
	p[tmp]=15;  check^=p[tmp++];
	p[tmp++]=check;  //校验位
	//p[tmp++]=check;  //校验位
	p[tmp]=0;
	encryption(p,tmp);
	Num_to_String(p,tmp);
	return;
}


//16进制数转化成16进制字符串
void Num_to_String(u8 *p,u16 len)
{
	u16 i;
//	for(i=0;i<len;++i)
//	printf("%d ",p[i]);
//	printf("\n");
	for(i=0;i<len;++i){
		if(p[i]>=0 && p[i]<=9)
			p[i]='0'+p[i];
		else p[i]='A'+p[i]-10;
	}
	return;
}

void encryption(u8 *p,u16 len)
{
	u16 i;
	for(i=0;i<len;++i)
	{
		p[i]=Hash[p[i]];
	}
	return;
}*/