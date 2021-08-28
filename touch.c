#include "touch.h"
#include "TFT4_SPI.h"
#include "stdlib.h"
#include "math.h"
#include "flash.h"
#include "stdio.h"

//这个是我用F411一款512k flash的地址，用的是很靠后面的一点地址存储，根据你自己的单片机地址找地址使用，用法在我上一篇flash的博文里面讲到了
#define save_addr 0x08007C00 

_m_tp_dev tp_dev=
{
	TP_Init,
	TP_Scan,
	TP_Adjust,
	0,
	0,
 	0,
	0,
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};	

//默认为touchtype=0的数据.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

//以下这一段是在网上找的HAL库的us级别延时，实测还是比较准的
#define CPU_FREQUENCY_MHZ    72		// STM32时钟主频，对应你自己的MCU主频改一下
void delay_us(__IO uint32_t delay)
{
    int last, curr, val;
    int temp;
    while (delay != 0)
    {
        temp = delay > 900 ? 900 : delay;
        last = SysTick->VAL;
        curr = last - CPU_FREQUENCY_MHZ * temp;
        if (curr >= 0)
        {
            do
            {
                val = SysTick->VAL;
            }
            while ((val < last) && (val >= curr));
        }
        else
        {
            curr += CPU_FREQUENCY_MHZ * 1000;
            do
            {
                val = SysTick->VAL;
            }
            while ((val <= last) || (val > curr));
        }
        delay -= temp;
    }
}

//用SPI总线向触摸屏IC写入字节数据
void TP_Write_Byte(u8 num)    
{  
	u8 count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)
		{
			LCD_T_DIN_SET;
		}  
		else 
		{
			LCD_T_DIN_RESET;  
		} 
		num<<=1;    
		LCD_T_CLK_RESET; 	 
		LCD_T_CLK_SET;		//上升沿有效	        
	}		 			    
}

//读取AD值
u16 TP_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	LCD_T_CLK_RESET;		//先拉低时钟 	 
	LCD_T_DIN_RESET ; 	//拉低数据线
	LCD_T_CS_RESET; 		//选中触摸屏IC
	TP_Write_Byte(CMD);//发送命令字
	delay_us(6);//ADS7846的转换时间最长为6us
	LCD_T_CLK_RESET; 	     	    
	delay_us(1);    	   
	LCD_T_CLK_SET;		//给1个时钟，清除BUSY	    	    
	LCD_T_CLK_RESET; 	     	    
	for(count=0;count<16;count++)//读出16位数据,只有高12位有效 
	{ 				  
		Num<<=1; 	 
		LCD_T_CLK_RESET;	//下降沿有效  	    	   
		LCD_T_CLK_SET;
		if(HAL_GPIO_ReadPin(TFT4_SPI_T_DOUT_GPIO_Port , TFT4_SPI_T_DOUT_Pin))Num++; 		 
	}  	
	Num>>=4;   	//只有高12位有效.
	LCD_T_CS_SET;		//释放片选	 
	return(Num);  
//#endif
}

#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
//读取触摸屏坐标（x或y），连续读取次数据并按升序对数据进行排序，然后删除最低和最高的丢失值取平均值
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//排序
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//升序排列
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

//读取触摸屏的x和y坐标，最小值不能小于100
//x： 读取触摸屏的x坐标y： 读取触摸屏的y坐标
//返回值 0：失败   1：成功
u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	//if(xtemp<100||ytemp<100)return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}

#define ERR_RANGE 50 //误差范围 
//连续两次读取触摸屏坐标，这两次的偏差不能超过误差范围，满足条件，然后认为读数正确，否则读数是错误的。此功能可大大提高精度。
u8 TP_Read_XY2(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 

//查看是否发生触摸
//tp:0-屏坐标  1-物理坐标（用于特殊场合，如校准）
u8 TP_Scan(u8 tp)
{			   
	if(HAL_GPIO_ReadPin(TFT4_SPI_T_IRQ_GPIO_Port , TFT4_SPI_T_IRQ_Pin) == 0)//有按键按下
	{
		if(tp)TP_Read_XY2(&tp_dev.x,&tp_dev.y);//读取物理坐标
		else if(TP_Read_XY2(&tp_dev.x,&tp_dev.y))//读取屏幕坐标
		{
	 		tp_dev.x=tp_dev.xfac*tp_dev.x+tp_dev.xoff;//将结果转换为屏幕坐标
			tp_dev.y=tp_dev.yfac*tp_dev.y+tp_dev.yoff;  
	 	} 
		if((tp_dev.sta&TP_PRES_DOWN)==0)//之前没有被按下
		{		 
			tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//按键按下  
			tp_dev.x0=tp_dev.x;//记录第一次按下时的坐标
			tp_dev.y0=tp_dev.y;  	   			 
		}			   
	}else
	{
		if(tp_dev.sta&TP_PRES_DOWN)//之前是被按下的
		{
			tp_dev.sta&=~(1<<7);//标记按键松开	
		}else//之前就没有被按下
		{
			tp_dev.x0=0;
			tp_dev.y0=0;
			tp_dev.x=0xffff;
			tp_dev.y=0xffff;
		}	    
	}
	return tp_dev.sta&TP_PRES_DOWN;//返回当前的触屏状态
}

//获取校准参数
u8 TP_Get_Adjdata(void)
{		
  unsigned int tempfac;
	unsigned short pBuffer;
  u16	temp_h;
	u16	temp_l;
	STMFLASH_Read ( save_addr + 14, &pBuffer, 1 );  //读取的内容放在pBuffer上
	tempfac =pBuffer;
	if(tempfac==0X0A)//触摸屏已经校准过了	
	{
		//得到x校准参数
		STMFLASH_Read ( save_addr, &pBuffer, 1 );  //读取的内容放在pBuffer上
		temp_h = pBuffer;
		HAL_Delay(10);
		STMFLASH_Read ( save_addr+2, &pBuffer, 1 );
    HAL_Delay(10);		
		temp_l = pBuffer;
		tp_dev.xfac = (float)(temp_h*65536 + temp_l)/100000000;
		
		//得到y校准参数
		STMFLASH_Read ( save_addr + 4, &pBuffer, 1 ); 
		HAL_Delay(10);
		temp_h = pBuffer;
		STMFLASH_Read ( save_addr + 6, &pBuffer, 1 );  
		HAL_Delay(10);
		temp_l = pBuffer;
		tp_dev.yfac = (float)(temp_h*65536 + temp_l)/100000000;
		
		//得到x偏移量
		STMFLASH_Read ( save_addr + 8, &pBuffer, 1 );
		HAL_Delay(10);
		tp_dev.xoff=pBuffer;			 
		
 	  //得到y偏移量
		STMFLASH_Read ( save_addr + 10, &pBuffer, 1 );
		HAL_Delay(10);
		tp_dev.yoff=pBuffer;
		
		//读取触屏类型标记
		STMFLASH_Read ( save_addr + 12, &pBuffer, 1 );
		HAL_Delay(10);
		tp_dev.touchtype = pBuffer;
		if(tp_dev.touchtype)//X,Y方向与屏幕相反
		{
			CMD_RDX=0X90;
			CMD_RDY=0XD0;	 
		}else				   //X,Y方向与屏幕相同
		{
			CMD_RDX=0XD0;
			CMD_RDY=0X90;	 
		}		
		return 1;	 
	}	
	return 0;
}	

//存下校准参数
void TP_Save_Adjdata(void)
{
	int32_t temp;			
  u16	temp_h;
	u16	temp_l;
	u16	t_xoff;
	u16	t_yoff;
	u16 sign;
	u16 touch_type;
	uint16_t pBuffer_send[8];
	//保存校正结果!		   							  
	temp=tp_dev.xfac*100000000;//保存x校正因素    
  temp_h = temp/65536;
	temp_l = temp%65536;
	pBuffer_send[0] = temp_h;
	pBuffer_send[1] = temp_l;
	
	temp=tp_dev.yfac*100000000;//保存y校正因素 
  temp_h = temp/65536;
	temp_l = temp%65536;
	pBuffer_send[2] = temp_h;
	pBuffer_send[3] = temp_l;
	
	//保存x偏移量
	t_xoff = tp_dev.xoff;
	pBuffer_send[4] = t_xoff;
	
	//保存y偏移量
	t_yoff = tp_dev.yoff;
	pBuffer_send[5] = t_yoff;
	
	//保存触屏类型
	touch_type = tp_dev.touchtype;
	pBuffer_send[6] = touch_type;
	
	sign = 0X0A;//标记校准过了
	pBuffer_send[7] = sign;
	
	STMFLASH_Write ( save_addr, &pBuffer_send[0], 8 ) ; 
	
}

void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
	POINT_COLOR=color;
	LCD_DrawLine(x-12,y,x+13,y);//横线
	LCD_DrawLine(x,y-12,x,y+13);//竖线
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	gui_circle(x,y,POINT_COLOR,6,0);//画中心圈
}	

void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{	    
	POINT_COLOR=color;
	LCD_DrawPoint(x,y);//中心点 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}	

//屏幕校准
void TP_Adjust(void)
{								 
	u16 pos_temp[4][2];//坐标缓存值
	u8  cnt=0;	
	u16 d1,d2;
	unsigned int tem1,tem2;
	double fac; 	
	//float fac; 
	u16 outtime=0;
 	cnt=0;				
	POINT_COLOR=BLUE;
	BACK_COLOR =WHITE;
	LCD_Clear(WHITE);//清屏   
	POINT_COLOR=RED;//红色 
	LCD_Clear(WHITE);//清屏 	   
	POINT_COLOR=BLACK;
	LCD_ShowString(10,40,16,"Please use the stylus click the",1);//显示提示信息
	LCD_ShowString(10,56,16,"cross on the screen.The cross will",1);//显示提示信息
	LCD_ShowString(10,72,16,"always move until the screen ",1);//显示提示信息
	LCD_ShowString(10,88,16,"adjustment is completed.",1);//显示提示信息

	TP_Drow_Touch_Point(20,20,RED);//画点1 
	tp_dev.sta=0;//消除触发信号 
	tp_dev.xfac=0;//xfac用来标记是否校准过,所以校准之前必须清掉!以免错误	 
	while(1)//如果连续10秒钟没有按下,则自动退出
	{
		tp_dev.scan(1);//扫描物理坐标
		if((tp_dev.sta&0xc0)==TP_CATH_PRES)//按键按下了一次(此时按键松开了.)
		{	
			outtime=0;		
			tp_dev.sta&=~(1<<6);//标记按键已经被处理过了.
						   			   
			pos_temp[cnt][0]=tp_dev.x;
			pos_temp[cnt][1]=tp_dev.y;
			cnt++;	  
			switch(cnt)
			{			   
				case 1:						 
					TP_Drow_Touch_Point(20,20,WHITE);				//清除点1 
					TP_Drow_Touch_Point(lcddev.width-20,20,RED);	//画点2
					break;
				case 2:
 					TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);	//清除点2
					TP_Drow_Touch_Point(20,lcddev.height-20,RED);	//画点3
					break;
				case 3:
 					TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);			//清除点3
 					TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);	//画点4
					break;
				case 4:	 //全部四个点已经得到
	    		    //对边相等
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,2的距离
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到3,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//不合格
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,3的距离
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
								   
					//对角线相等
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,4的距离
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,3的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
					//计算结果
					tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//得到xfac		 
					tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//得到xoff
						  
					tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//得到yfac
					tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//得到yoff  
					if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//触屏和预设的相反了.
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
						LCD_ShowString(40,26, 16,"TP Need readjust!",1);
						tp_dev.touchtype=!tp_dev.touchtype;//修改触屏类型.
						if(tp_dev.touchtype)//X,Y方向与屏幕相反
						{
							CMD_RDX=0X90;
							CMD_RDY=0XD0;	 
						}else				   //X,Y方向与屏幕相同
						{
							CMD_RDX=0XD0;
							CMD_RDY=0X90;	 
						}			    
						continue;
					}		
					POINT_COLOR=BLUE;
					LCD_Clear(WHITE);//清屏
					LCD_ShowString(35,110, 16,"Touch Screen Adjust OK!",1);//校正完成
					HAL_Delay(300);
					TP_Save_Adjdata(); //存下校准参数 
 					LCD_Clear(WHITE);//清屏   
					return;//校正完成				 
			}
		}
		HAL_Delay(20);
		outtime++;
		if(outtime>1000)
		{
			TP_Get_Adjdata();
			break;
	 	} 
 	}
}	

u8 TP_Init(void)
{			    		   
  //第一步肯定是要进行IO的初始化，但是由于我这里使用的是软件模拟SPI，所以IO不受SPI固定端口限制，同样在CubeMX里面定义标签标好IO就可以了
  TP_Read_XY(&tp_dev.x,&tp_dev.y);//第一次读取初始化	 
 	//这里本来使用EEPROM需要初始化EEPROM，但是我用的是内部FLASH所以不需要初始化
	if(TP_Get_Adjdata())return 0;  //这一步是从存储器里面取出触摸校准参数，如果发现没有校准就需要重新校准
	else			   //未校准?
	{ 										    
		LCD_Clear(WHITE);//清屏
	  TP_Adjust();  //屏幕校准 
		//TP_Save_Adjdata();	 
	}			
	TP_Get_Adjdata();	
	return 1; 									 
}

//提示字符串
const u8* TP_REMIND_MSG_TBL="Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";
/*****************************************************************************
 * @name       :void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
 * @date       :2018-08-09 
 * @function   :Display calibration results	
 * @parameters :x0:the x coordinates of first calibration point
								y0:the y coordinates of first calibration point
								x1:the x coordinates of second calibration point
								y1:the y coordinates of second calibration point
								x2:the x coordinates of third calibration point
								y2:the y coordinates of third calibration point
								x3:the x coordinates of fourth calibration point
								y3:the y coordinates of fourth calibration point
								fac:calibration factor 
 * @retvalue   :None
******************************************************************************/ 	 					  
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{	  
	POINT_COLOR=RED;
	LCD_ShowString(40,160,16,"x1:",1);
 	LCD_ShowString(40+80,160,16,"y1:",1);
 	LCD_ShowString(40,180,16,"x2:",1);
 	LCD_ShowString(40+80,180, 16,"y2:",1);
	LCD_ShowString(40,200, 16,"x3:",1);
 	LCD_ShowString(40+80,200, 16,"y3:",1);
	LCD_ShowString(40,220, 16,"x4:",1);
 	LCD_ShowString(40+80,220, 16,"y4:",1);  
 	LCD_ShowString(40,240, 16,"fac is:",1);     
	LCD_ShowNum(40+24,160,x0,4,16);		//显示数值
	LCD_ShowNum(40+24+80,160,y0,4,16);	//显示数值
	LCD_ShowNum(40+24,180,x1,4,16);		//显示数值
	LCD_ShowNum(40+24+80,180,y1,4,16);	//显示数值
	LCD_ShowNum(40+24,200,x2,4,16);		//显示数值
	LCD_ShowNum(40+24+80,200,y2,4,16);	//显示数值
	LCD_ShowNum(40+24,220,x3,4,16);		//显示数值
	LCD_ShowNum(40+24+80,220,y3,4,16);	//显示数值
 	LCD_ShowNum(40+56,lcddev.width,fac,3,16); 	//显示数值,该数值必须在95~105范围之内.
}

//测试触摸的代码
u16 ColorTab[5]={RED,GREEN,BLUE,YELLOW,BRED};//定义颜色数组
void Touch_Test(void)
{
	//u8 key;
	u16 i=0;
	u16 j=0;
	u16 colorTemp=0;
	TP_Init();
	LCD_Clear(WHITE);
	LCD_ShowString(0,20,16,"TOUCH TEST",1);
	LCD_ShowString(lcddev.width-24,0,16,"RST",1);//显示清屏区域
	POINT_COLOR=RED;
	LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,RED); 
	while(1)
	{
	 	//key=0;
		tp_dev.scan(0); 		 
		//printf("x:%d,y:%d",tp_dev.x,tp_dev.y);
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
		 	if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
			{	
				if(tp_dev.x>(lcddev.width-24)&&tp_dev.y<16)
				{
					LCD_Clear(WHITE);
	        LCD_ShowString(0,20,16,"TOUCH TEST",1);
					LCD_ShowString(lcddev.width-24,0,16,"RST",1);//显示清屏区域
					POINT_COLOR=colorTemp;
					LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,POINT_COLOR); 
				}
				else if((tp_dev.x>(lcddev.width-60)&&tp_dev.x<(lcddev.width-50+20))&&tp_dev.y<20)
				{
				LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,ColorTab[j%5]); 
				POINT_COLOR=ColorTab[(j++)%5];
				colorTemp=POINT_COLOR;
				HAL_Delay(10);
				}

				else TP_Draw_Big_Point(tp_dev.x,tp_dev.y,POINT_COLOR);		//画图	  			   
			}
		}else HAL_Delay(20);	//没有按键按下的时候 	 
    //这里是一个按键，我这里是PA0脚，按下这个键就会重新校正屏幕 		
		
	}   
}





