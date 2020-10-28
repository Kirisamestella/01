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
//sim900a���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim900a_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��sim900a��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 sim900a_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	if((u32)cmd<=0XFF)
	{
		while(DMA1_Channel2->CNDTR!=0);	//�ȴ�ͨ��2�������   
		USART3->DR=(u32)cmd;
	}else u3_printf("%s\r\n",cmd);//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(sim900a_check_cmd(ack))break;//�õ���Ч���� 
				USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	USART3_RX_STA=0;
	return res;
} 


/////////////////////////////////////////////////////////////////////////////////////////////
static u8 sim_sta=0;	//SIM��״̬��1��׼����
static u8 connect=0;//TCP����״̬��1��������

//x,y��ʾ�������Ͻ����꣬flag�Ƿ���ʾ
u8 sim900a_check_sim(u16 x,u16 y,u8 flag)
{
	u8 *p,*p1,*p2;
	u8 res=0;
	p=mymalloc(50);//����50�ֽ��ڴ�
	
	if(flag){//��Ҫ��ʾ������
		LCD_Fill(0,41,239,286,BLACK);
		LCD_DrawLine(0,65,239,65,GREEN);
	}
	if(sim900a_send_cmd("AT+CPIN?","OK",200))//��ѯSIM���Ƿ���λ 
		res|=1<<0;
	printf("%s\n",USART3_RX_BUF);//���������
	
	if(sim900a_send_cmd("AT+COPS?","OK",200)==0)		//��ѯ��Ӫ������
	{
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\"");
		if(p1)//����Ч����
		{
			p2=(u8*)strstr((const char*)(p1+1),"\"");
			p2[0]=0;//���������			
			if(flag) sprintf((char*)p,"CARRIER:%s",p1+1);
			Show_Str(x,y,200,16,p,16,0,WHITE,BLACK,0);
		}
	}else res|=1<<1;
	
	if(sim900a_send_cmd("AT+CSQ","+CSQ:",200)==0)		//��ѯ�ź�����
	{ 
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),":");
		p2=(u8*)strstr((const char*)(p1),",");
		p2[0]=0;//���������
		sprintf((char*)p,"SIGNAL QUALITY:%s",p1+2);
		if(flag) Show_Str(x,y+20,200,16,p,16,0,WHITE,BLACK,0);	
	}else res|=1<<2;
	
	if(sim900a_send_cmd("AT+CBC","+CBC:",200)==0)		//��ѯ��ص���
	{ 
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),",");
		p2=(u8*)strstr((const char*)(p1+1),",");
		p2[0]=0;p2[5]=0;//���������
		sprintf((char*)p,"BATTRY:%s%%  %smV",p1+1,p2+1);
		if(flag) Show_Str(x,y+40,200,16,p,16,0,WHITE,BLACK,0);	
	}else res|=1<<3;
	
	if(sim900a_send_cmd("AT+CCLK?","+CCLK:",200)==0)		//��ѯ����
	{ 
		printf("%s",USART3_RX_BUF);
		p1=(u8*)strstr((const char*)(USART3_RX_BUF),"\"");
		p2=(u8*)strstr((const char*)(p1+1),":");
		p2[3]=0;//���������
		sprintf((char*)p,"DATE:%s",p1+1);
		if(flag) Show_Str(x,y+60,200,16,p,16,0,WHITE,BLACK,0);
	}else res|=1<<4;
	
	myfree(p);
	return res;
}

//���ú�����ǰ��ATָ��
//����ֵ��0�����óɹ�
//				else������ʧ��
u8 sim900a_set_AT(void)
{
	
	sim900a_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100);	//�ر�����
	printf("%s",USART3_RX_BUF);
	sim900a_send_cmd("AT+CIPSHUT","SHUT OK",100);		//�ر��ƶ����� 
	printf("%s",USART3_RX_BUF);
	
	if(sim900a_send_cmd("AT+CGCLASS=\"B\"","OK",1000)){
		printf("%s",USART3_RX_BUF);
		return 2;//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
	}
	if(sim900a_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",1000)){
		printf("%s",USART3_RX_BUF);
		return 3;//����PDP������,��������Э��,��������Ϣ
	}
	if(sim900a_send_cmd("AT+CGATT=1","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 4;//����GPRSҵ��
	}
	if(sim900a_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 5;//����ΪGPRS����ģʽ
	}
	if(sim900a_send_cmd("AT+CIPHEAD=1","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 6;//���ý���������ʾIPͷ(�����ж�������Դ)
	}
	//��������͸��ģʽ
	if(sim900a_send_cmd("AT+IFC=2,2","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 7;
	}
	if(sim900a_send_cmd("AT+CIPMODE=1","OK",500)){
		printf("%s",USART3_RX_BUF);
		return 8;
	}
	if(sim900a_send_cmd("AT+CIPCCFG=5,2,1024,1","OK",500)){//1024�涨ÿ�δ�����󲻳���1024
		printf("%s",USART3_RX_BUF);
		return 9;
	}
	
	return 0;
}

static u8 ip[16]="111.229.170.131";//������ip
static u8 port[6]="8088";//�˿�
static u8 ip_pointer=0;
//�����������
//����ֵ��0�����ӳɹ�
//				1������ʧ��
u8 sim900a_connect(u8 flag)
{
	u16 fail=0;
	u8 *p=mymalloc(50);
	
	//�˳�͸��ģʽ
	delay_ms(1500);
	u3_printf("+++");
	delay_ms(1500);
	
	while(sim_sta == 0 && fail < 3){//��ѯ����ʾsim����Ϣ������3��ʧ��
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
	while(sim900a_set_AT() && fail < 3){//��������ǰ��ATָ�����3��ʧ��
		++fail;
		delay_ms(1000);
	}
	if(fail >= 3){
		LCD_ShowString(20,150,200,16,16,"SET AT FAIL",WHITE,BLACK,0);
		return 1;
	}
	
	//��ʼ����
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",ip,port);
	while(connect == 0 && fail < 5)//����5��ʧ��
	{
		if(sim900a_send_cmd(p,"OK",500))//��������
			connect=0,printf("%s",USART3_RX_BUF);
		else connect=1,printf("%s",USART3_RX_BUF);
		delay_ms(500);
	}
	fail=300;
	while(fail){
		if(USART3_RX_STA & 0X8000){//����CONNECT�����ӳɹ���60sû�յ���Ϊ����ʧ��
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
		printf("--%d\n",fail);//���������
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
	while(connect==0 && ++fail<=3)//3�����Ӳ��Ͼͷ���
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
u8 Hash[]={0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15}; //�ֵ�
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
	u3_printf("%2X%2X",0xAA,0xB4);//����ͷAA��������������B4
	u3_printf("%02X",len);//���ݳ���
	for(i=0;i<len;++i){
		u3_printf("%1X%1X",Hash[(p[i]>>4)&15],Hash[p[i]&15]);//���ݣ�����
		check^=p[i];
	}
	u3_printf("%02X%3X",check,0xFFF);
	return 0;
}

/*
//����0���ɹ�
//����1��ʧ��
u8 work()
{
	u8 read_buf[READ_LEN]={0};
	static FIL file;
	u8 *p=mymalloc(450);
	static u8 file_path[]="0:/Temperature/2016-01-18.txt";  										//�ļ���
	static u8 *file_dir="0:/Temperature";
	u8 *p1=mymalloc(100);	//����100�ֽ��ڴ�;
	u8 *p2=mymalloc(100);	//����100�ֽ��ڴ�;
	u8 fail=0,done=0;
	u16 size=0,i=0,len=0;
	
	u8 send_err;
	u8 *key=0;

	f_mkdir (file_dir);//�����ļ���
	
	while(done==0)
	{
		//�����ļ�
		f_mount(fs[0],"0:",1);	//����SD��
		if( f_open(&file, (const TCHAR*)file_path, FA_OPEN_ALWAYS | FA_READ) ==FR_OK)
		{
			size=f_size(&file);  f_close(&file);
			LCD_ShowString(10,170,100,16,16,"Total size:",BLACK,WHITE,0);
			LCD_ShowString(10,190,100,16,16,"Sended size:",BLACK,WHITE,0);
			LCD_ShowNum(140,170,size,5,16,BLACK,WHITE,0);
			LCD_ShowNum(140,190,i,5,16,BLACK,WHITE,0);
			//����ͷAA��len���ݳ��ȣ�read_buf��Ч���ݣ�FF����β��01У��λ
			sprintf((char*)p,"%04d20160108",size);
			sim900a_send(p);
//			String_to_16Num(p);
//			printf("%s\n",p);
			while(1)
			{
				if(i==size) break;  //�ļ��������
				if(f_open(&file, (const TCHAR*)file_path, FA_OPEN_ALWAYS | FA_READ) !=FR_OK){//�ļ���ʧ��
					LCD_ShowString(60,180,100,16,16,"OPEN FAIL",BLACK,WHITE,0);
					return 1;
				}
				if(f_lseek(&file,i) != FR_OK){
					LCD_ShowString(60,180,100,16,16,"LOCATE FAIL",BLACK,WHITE,0);//�ļ���λʧ��
					return 1;
				}
				if(f_read(&file,read_buf,READ_LEN,&br) != FR_OK){
					LCD_ShowString(60,180,100,16,16,"READ FAIL",BLACK,WHITE,0);//�ļ���ȡʧ��
					return 1;
				}
				
				p1=(u8*)strstr((const char*)read_buf,"\n");//ÿ�ν�ȡһ�з���
				p1[0]=0;
				len=strlen(read_buf);
				sprintf((char*)p,"[2016-01-08]%s",read_buf);
				i+=(p1-read_buf)+1;//�����ļ�ָ��ƫ����
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
			Show_Str(60,180,100,16,"OPEN FAIL",16,0,WHITE,BLACK,0);//�ļ���ʧ��
			return 1;
		}
		
		//ÿ�ζ�ȡ�ļ���Ҫ��ʼ������2
		USART2_Init(36,230400);
		delay_ms(50);
	}
	//Show_Str(150,180,100,16,"DONE",16,0,WHITE,BLACK,0);
	return 0;//�������
}


//�ַ���ת����16������
void String_to_16Num(u8 *p)
{
	u8 *str=mymalloc(300),check=0;
	u16 tmp=0,i=0;
	u16 len=strlen(p);
	sprintf((char*)str,"%s",p);
	//printf("%s\n",str);
	//0XAA  ����ͷ
	p[tmp]=10;  check^=p[tmp++];
	p[tmp]=10;  check^=p[tmp++];
	//0X01  ��������
	p[tmp]=0;  check^=p[tmp++];
	p[tmp]=1;  check^=p[tmp++];
	//���ݳ��� 4λ
	p[tmp]=(len>>12)&7;  check^=p[tmp++];
	p[tmp]=(len>>8)&15;  check^=p[tmp++];
	p[tmp]=(len>>4)&15;  check^=p[tmp++];
	p[tmp]=len&15;  check^=p[tmp++];
	for(i=0;i<len;++i){
		p[tmp]=(str[i]>>4)&15;  check^=p[tmp++];
		p[tmp]=str[i]&15;  check^=p[tmp++];
	}
	//����β
	p[tmp]=15;  check^=p[tmp++];
	p[tmp]=15;  check^=p[tmp++];
	p[tmp++]=check;  //У��λ
	//p[tmp++]=check;  //У��λ
	p[tmp]=0;
	encryption(p,tmp);
	Num_to_String(p,tmp);
	return;
}


//16������ת����16�����ַ���
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