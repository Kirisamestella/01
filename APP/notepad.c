#include "notepad.h"
#include "t9input.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//APP-记事本 代码	   
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

//模式选择
const u8* notepad_mode_tbl[GUI_LANGUAGE_NUM][2]=
{
{"新建文本文件","打开已有文件",},
{"新建文本文件","打開已有文件",},		 
{"Create new text file","Open exist file",},
};  
 
//通过时间获取文件名
//仅限在sd卡保存,不支持flash disk保存
//组合成:形如"0:TEXT/TEXT20120321210633.txt"的文件名
void notepad_new_pathname(u8 *pname)
{	 
	RTC_Get();//更新时间
 	sprintf((char*)pname,"0:TEXT/TEXT%04d%02d%02d%02d%02d%02d.txt",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
} 
 
//记事本功能
//所有新建文本文件,都保存在SD卡的TEXT文件夹内
u8 notepad_play(void)
{
	FIL* f_txt=0;					//文件
	DIR notepaddir;					//notepaddir专用	  
	FILINFO notepadinfo;		 	   
  	_btn_obj* rbtn=0;				//返回按钮控件
   	_filelistbox_obj * flistbox=0;
	_filelistbox_list * filelistx=0;//文件表
	_t9_obj * t9=0;					//输入法  
	_memo_obj * tmemo=0;	    	//memo控件
	u8 editmask=0;					//标志是否进行了编辑
 	u8 *pname=0;
	u8 *fn;	 
	u8 res;
	u8 rval=0;
	u8 mode=0;
   	br=0; 
	
	//先选择模式
	rval=app_items_sel((lcddev.width-180)/2,(lcddev.height-152)/2,180,72+40*2,(u8**)notepad_mode_tbl[gui_phy.language],2,(u8*)&mode,0X90,(u8*)APP_MODESEL_CAPTION_TBL[gui_phy.language]);//2个选择
	f_txt=(FIL *)gui_memin_malloc(sizeof(FIL));	//开辟FIL字节的内存区域 
	if(f_txt==NULL)rval=1;					//申请失败
	rbtn=btn_creat(lcddev.width-41,lcddev.height-APP_FB_BTMBAR_HEIGHT,40,19,0,0x03);//创建文字按钮
	if(rbtn==NULL)rval=1;					//没有足够内存够分配
	else
	{																				
	 	rbtn->caption=(u8*)GUI_BACK_CAPTION_TBL[gui_phy.language];//返回 
	 	rbtn->font=16;			//字体		 
		rbtn->bcfdcolor=WHITE;	//按下时的颜色
		rbtn->bcfucolor=WHITE;	//松开时的颜色
		btn_draw(rbtn);			//重画按钮
	}  
  	if(rval==0)//确认键按下了,同时按键f_txt创建OK,RBTN创建OK
	{	  
		LCD_Clear(0);  	 
   		if(mode==0)				//新建文本文件
		{
 			pname=gui_memin_malloc(40);//申请40个字节内存,类似"0:TEXT/TEXT20120321210633.txt"
			pname[0]='\0';//添加结束符
			notepad_new_pathname(pname);
			f_txt->fsize=0;
			if(pname==NULL)rval=1;
 		}else	//选择一个文本文件打开
		{
			app_filebrower((u8*)notepad_mode_tbl[gui_phy.language][1],0X07);//显示信息
			btn_read_backcolor(rbtn);//重新读取按钮背景色
			btn_draw(rbtn);//画按钮  
		   	flistbox=filelistbox_creat(0,20,lcddev.width,lcddev.height-40,1,12);//创建一个filelistbox
			if(flistbox==NULL)rval=1;							//申请内存失败.
			else  
			{
				flistbox->fliter=FLBOX_FLT_TEXT|FLBOX_FLT_LRC;	//查找TEXT文件
				filelistbox_addlist(flistbox,flistbox->disklabel[0],0);		//磁盘0
				filelistbox_addlist(flistbox,flistbox->disklabel[1],0);		//磁盘1
				filelistbox_draw_listbox(flistbox);
			} 	 
 			//为长文件名申请缓存区
		 	notepadinfo.lfsize = _MAX_LFN * 2 + 1;
			notepadinfo.lfname = gui_memin_malloc(notepadinfo.lfsize);
			if(notepadinfo.lfname==NULL)rval=1;//申请内存失败 
		   	else gui_memset((u8*)notepadinfo.lfname,0,notepadinfo.lfsize);
 		   	while(rval==0)
			{
				tp_dev.scan(0);    
				in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);	//得到按键键值   
				delay_ms(1000/OS_TICKS_PER_SEC);		//延时一个时钟节拍
		 		if(system_task_return)					//需要返回
				{
					rval=1;//强制退出
					break;			
				}
				filelistbox_check(flistbox,&in_obj);	//扫描文件
				res=btn_check(rbtn,&in_obj);
				if(res)
				{
					if(((rbtn->sta&0X80)==0))//按钮状态改变了
					{
						if(flistbox->dbclick!=0X81)
						{
		 					filelistx=filelist_search(flistbox->list,flistbox->selindex);//得到此时选中的list的信息
							if(filelistx->type==FICO_DISK)
							{
								rval=1;
								break;	//已经不能再往上了
						 	}
							else filelistbox_back(flistbox);	//退回上一层目录	 
						} 
		 			}	 
				}
				if(flistbox->dbclick==0X81)//双击文件了
				{	    
 		     		rval=f_opendir(&notepaddir,(const TCHAR*)flistbox->path); //打开选中的目录
					if(rval)break;	 			   
					dir_sdi(&notepaddir,flistbox->findextbl[flistbox->selindex-flistbox->foldercnt]);
			 		rval=f_readdir(&notepaddir,&notepadinfo);//读取文件信息
					if(rval)break;//打开失败  
					fn=(u8*)(*notepadinfo.lfname?notepadinfo.lfname:notepadinfo.fname);
					pname=gui_memin_malloc(strlen((const char*)fn)+strlen((const char*)flistbox->path)+2);//申请内存
					if(pname==NULL)rval=1;//申请失败
					{
						pname=gui_path_name(pname,flistbox->path,fn);	//文件名加入路径  
			 			res=f_open(f_txt,(const TCHAR*)pname,FA_READ);//以读方式打开文件
						if(res)rval=1; 
    					break;
					} 
				}
			}   
		}   
		if(rval==0)//前面操作均无问题
		{
			if(lcddev.id==0X5510)tmemo=memo_creat(0,20,lcddev.width,514,0,1,16,f_txt->fsize+NOTEPAD_EDIT_LEN);								//申请整个txt这么多的内存
			else if(lcddev.id==0X6804||lcddev.id==0X5310)tmemo=memo_creat(0,20,lcddev.width,284,0,1,16,f_txt->fsize+NOTEPAD_EDIT_LEN);	//申请整个txt这么多的内存
			else tmemo=memo_creat(0,20,240,166,0,1,16,f_txt->fsize+NOTEPAD_EDIT_LEN);//申请整个txt这么多的内存	
			if(tmemo==NULL)rval=1;
			else//memo创建成功
			{
	 			if(mode==1)
				{
					res=f_read(f_txt,tmemo->text,f_txt->fsize,(UINT*)&br);//读出txt里面的内容  
					f_close(f_txt);//关闭文件
				} 
				memo_draw_memo(tmemo);
		  		app_filebrower(pname,0X05);	//显示标题栏背景
		  		if(lcddev.id==0X5510)t9=t9_creat(0,534,480,266,0); 
				else if(lcddev.id==0X6804||lcddev.id==0X5310)t9=t9_creat(0,304,320,176,0); 
				else t9=t9_creat(0,186,240,134,0); 
				if(t9==NULL)rval=1;	 
				else t9_draw(t9);	    
	 		}
		} 
	 	while(rval==0) 
		{		 
			tp_dev.scan(0); 		 
			in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);	//得到按键键值 	   
			delay_ms(1000/OS_TICKS_PER_SEC);		//延时一个时钟节拍						  
 			if(system_task_return)break;			//需要返回
			t9_check(t9,&in_obj);		   
			memo_check(tmemo,&in_obj);
 			if(t9->outstr[0]!=NULL)//添加字符
			{
				//printf("%s\r\n",t9->outstr);
 				memo_add_text(tmemo,t9->outstr);
				t9->outstr[0]=NULL;	 			//清空输出字符
				editmask=1;						//标记进行了编辑
 			}
		}
 	}   
 	if(editmask)//有编辑过,需要保存
	{
 		res=window_msg_box((lcddev.width-150)/2,(lcddev.height-60)/2,150,60,"",(u8*)APP_SAVE_CAPTION_TBL[gui_phy.language],12,0,0X03);
		if(res==1)//需要保存
		{
			if(mode==0)res=f_open(f_txt,(const TCHAR*)pname,FA_READ|FA_WRITE|FA_CREATE_NEW);//创建新的文件
			else res=f_open(f_txt,(const TCHAR*)pname,FA_READ|FA_WRITE);//以读写方式打开文件
			if(res==0)//打开成功
			{
				f_lseek(f_txt,0);														//指向文件的开始地址		 
				f_write(f_txt,tmemo->text,strlen((const char*)tmemo->text),(UINT*)&br);	//保存文件
				f_close(f_txt);
			}else//打开文件失败,提示信息
			{
				window_msg_box((lcddev.width-150)/2,(lcddev.height-60)/2,150,60,(u8*)APP_CREAT_ERR_MSG_TBL[gui_phy.language],(u8*)APP_REMIND_CAPTION_TBL[gui_phy.language],12,0,0);//提示SD卡是否存在
				delay_ms(2000);//等待2秒钟
			}
		}
	}
	t9_delete(t9);							  
	filelistbox_delete(flistbox);
 	btn_delete(rbtn);
 	memo_delete(tmemo);
	gui_memin_free(f_txt);
 	gui_memin_free(pname);
	gui_memin_free(notepadinfo.lfname);
	return rval;

}








