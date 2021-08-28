#include "touch.h"
#include "TFT4_SPI.h"
#include "stdlib.h"
#include "math.h"
#include "flash.h"
#include "stdio.h"

//���������F411һ��512k flash�ĵ�ַ���õ��Ǻܿ������һ���ַ�洢���������Լ��ĵ�Ƭ����ַ�ҵ�ַʹ�ã��÷�������һƪflash�Ĳ������潲����
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

//Ĭ��Ϊtouchtype=0������.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

//������һ�����������ҵ�HAL���us������ʱ��ʵ�⻹�ǱȽ�׼��
#define CPU_FREQUENCY_MHZ    72		// STM32ʱ����Ƶ����Ӧ���Լ���MCU��Ƶ��һ��
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

//��SPI����������ICд���ֽ�����
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
		LCD_T_CLK_SET;		//��������Ч	        
	}		 			    
}

//��ȡADֵ
u16 TP_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	LCD_T_CLK_RESET;		//������ʱ�� 	 
	LCD_T_DIN_RESET ; 	//����������
	LCD_T_CS_RESET; 		//ѡ�д�����IC
	TP_Write_Byte(CMD);//����������
	delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
	LCD_T_CLK_RESET; 	     	    
	delay_us(1);    	   
	LCD_T_CLK_SET;		//��1��ʱ�ӣ����BUSY	    	    
	LCD_T_CLK_RESET; 	     	    
	for(count=0;count<16;count++)//����16λ����,ֻ�и�12λ��Ч 
	{ 				  
		Num<<=1; 	 
		LCD_T_CLK_RESET;	//�½�����Ч  	    	   
		LCD_T_CLK_SET;
		if(HAL_GPIO_ReadPin(TFT4_SPI_T_DOUT_GPIO_Port , TFT4_SPI_T_DOUT_Pin))Num++; 		 
	}  	
	Num>>=4;   	//ֻ�и�12λ��Ч.
	LCD_T_CS_SET;		//�ͷ�Ƭѡ	 
	return(Num);  
//#endif
}

#define READ_TIMES 5 	//��ȡ����
#define LOST_VAL 1	  	//����ֵ
//��ȡ���������꣨x��y����������ȡ�����ݲ�����������ݽ�������Ȼ��ɾ����ͺ���ߵĶ�ʧֵȡƽ��ֵ
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//����
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//��������
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

//��ȡ��������x��y���꣬��Сֵ����С��100
//x�� ��ȡ��������x����y�� ��ȡ��������y����
//����ֵ 0��ʧ��   1���ɹ�
u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	//if(xtemp<100||ytemp<100)return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}

#define ERR_RANGE 50 //��Χ 
//�������ζ�ȡ���������꣬�����ε�ƫ��ܳ�����Χ������������Ȼ����Ϊ������ȷ����������Ǵ���ġ��˹��ܿɴ����߾��ȡ�
u8 TP_Read_XY2(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 

//�鿴�Ƿ�������
//tp:0-������  1-�������꣨�������ⳡ�ϣ���У׼��
u8 TP_Scan(u8 tp)
{			   
	if(HAL_GPIO_ReadPin(TFT4_SPI_T_IRQ_GPIO_Port , TFT4_SPI_T_IRQ_Pin) == 0)//�а�������
	{
		if(tp)TP_Read_XY2(&tp_dev.x,&tp_dev.y);//��ȡ��������
		else if(TP_Read_XY2(&tp_dev.x,&tp_dev.y))//��ȡ��Ļ����
		{
	 		tp_dev.x=tp_dev.xfac*tp_dev.x+tp_dev.xoff;//�����ת��Ϊ��Ļ����
			tp_dev.y=tp_dev.yfac*tp_dev.y+tp_dev.yoff;  
	 	} 
		if((tp_dev.sta&TP_PRES_DOWN)==0)//֮ǰû�б�����
		{		 
			tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//��������  
			tp_dev.x0=tp_dev.x;//��¼��һ�ΰ���ʱ������
			tp_dev.y0=tp_dev.y;  	   			 
		}			   
	}else
	{
		if(tp_dev.sta&TP_PRES_DOWN)//֮ǰ�Ǳ����µ�
		{
			tp_dev.sta&=~(1<<7);//��ǰ����ɿ�	
		}else//֮ǰ��û�б�����
		{
			tp_dev.x0=0;
			tp_dev.y0=0;
			tp_dev.x=0xffff;
			tp_dev.y=0xffff;
		}	    
	}
	return tp_dev.sta&TP_PRES_DOWN;//���ص�ǰ�Ĵ���״̬
}

//��ȡУ׼����
u8 TP_Get_Adjdata(void)
{		
  unsigned int tempfac;
	unsigned short pBuffer;
  u16	temp_h;
	u16	temp_l;
	STMFLASH_Read ( save_addr + 14, &pBuffer, 1 );  //��ȡ�����ݷ���pBuffer��
	tempfac =pBuffer;
	if(tempfac==0X0A)//�������Ѿ�У׼����	
	{
		//�õ�xУ׼����
		STMFLASH_Read ( save_addr, &pBuffer, 1 );  //��ȡ�����ݷ���pBuffer��
		temp_h = pBuffer;
		HAL_Delay(10);
		STMFLASH_Read ( save_addr+2, &pBuffer, 1 );
    HAL_Delay(10);		
		temp_l = pBuffer;
		tp_dev.xfac = (float)(temp_h*65536 + temp_l)/100000000;
		
		//�õ�yУ׼����
		STMFLASH_Read ( save_addr + 4, &pBuffer, 1 ); 
		HAL_Delay(10);
		temp_h = pBuffer;
		STMFLASH_Read ( save_addr + 6, &pBuffer, 1 );  
		HAL_Delay(10);
		temp_l = pBuffer;
		tp_dev.yfac = (float)(temp_h*65536 + temp_l)/100000000;
		
		//�õ�xƫ����
		STMFLASH_Read ( save_addr + 8, &pBuffer, 1 );
		HAL_Delay(10);
		tp_dev.xoff=pBuffer;			 
		
 	  //�õ�yƫ����
		STMFLASH_Read ( save_addr + 10, &pBuffer, 1 );
		HAL_Delay(10);
		tp_dev.yoff=pBuffer;
		
		//��ȡ�������ͱ��
		STMFLASH_Read ( save_addr + 12, &pBuffer, 1 );
		HAL_Delay(10);
		tp_dev.touchtype = pBuffer;
		if(tp_dev.touchtype)//X,Y��������Ļ�෴
		{
			CMD_RDX=0X90;
			CMD_RDY=0XD0;	 
		}else				   //X,Y��������Ļ��ͬ
		{
			CMD_RDX=0XD0;
			CMD_RDY=0X90;	 
		}		
		return 1;	 
	}	
	return 0;
}	

//����У׼����
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
	//����У�����!		   							  
	temp=tp_dev.xfac*100000000;//����xУ������    
  temp_h = temp/65536;
	temp_l = temp%65536;
	pBuffer_send[0] = temp_h;
	pBuffer_send[1] = temp_l;
	
	temp=tp_dev.yfac*100000000;//����yУ������ 
  temp_h = temp/65536;
	temp_l = temp%65536;
	pBuffer_send[2] = temp_h;
	pBuffer_send[3] = temp_l;
	
	//����xƫ����
	t_xoff = tp_dev.xoff;
	pBuffer_send[4] = t_xoff;
	
	//����yƫ����
	t_yoff = tp_dev.yoff;
	pBuffer_send[5] = t_yoff;
	
	//���津������
	touch_type = tp_dev.touchtype;
	pBuffer_send[6] = touch_type;
	
	sign = 0X0A;//���У׼����
	pBuffer_send[7] = sign;
	
	STMFLASH_Write ( save_addr, &pBuffer_send[0], 8 ) ; 
	
}

void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
	POINT_COLOR=color;
	LCD_DrawLine(x-12,y,x+13,y);//����
	LCD_DrawLine(x,y-12,x,y+13);//����
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	gui_circle(x,y,POINT_COLOR,6,0);//������Ȧ
}	

void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{	    
	POINT_COLOR=color;
	LCD_DrawPoint(x,y);//���ĵ� 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}	

//��ĻУ׼
void TP_Adjust(void)
{								 
	u16 pos_temp[4][2];//���껺��ֵ
	u8  cnt=0;	
	u16 d1,d2;
	unsigned int tem1,tem2;
	double fac; 	
	//float fac; 
	u16 outtime=0;
 	cnt=0;				
	POINT_COLOR=BLUE;
	BACK_COLOR =WHITE;
	LCD_Clear(WHITE);//����   
	POINT_COLOR=RED;//��ɫ 
	LCD_Clear(WHITE);//���� 	   
	POINT_COLOR=BLACK;
	LCD_ShowString(10,40,16,"Please use the stylus click the",1);//��ʾ��ʾ��Ϣ
	LCD_ShowString(10,56,16,"cross on the screen.The cross will",1);//��ʾ��ʾ��Ϣ
	LCD_ShowString(10,72,16,"always move until the screen ",1);//��ʾ��ʾ��Ϣ
	LCD_ShowString(10,88,16,"adjustment is completed.",1);//��ʾ��ʾ��Ϣ

	TP_Drow_Touch_Point(20,20,RED);//����1 
	tp_dev.sta=0;//���������ź� 
	tp_dev.xfac=0;//xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������	 
	while(1)//�������10����û�а���,���Զ��˳�
	{
		tp_dev.scan(1);//ɨ����������
		if((tp_dev.sta&0xc0)==TP_CATH_PRES)//����������һ��(��ʱ�����ɿ���.)
		{	
			outtime=0;		
			tp_dev.sta&=~(1<<6);//��ǰ����Ѿ����������.
						   			   
			pos_temp[cnt][0]=tp_dev.x;
			pos_temp[cnt][1]=tp_dev.y;
			cnt++;	  
			switch(cnt)
			{			   
				case 1:						 
					TP_Drow_Touch_Point(20,20,WHITE);				//�����1 
					TP_Drow_Touch_Point(lcddev.width-20,20,RED);	//����2
					break;
				case 2:
 					TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);	//�����2
					TP_Drow_Touch_Point(20,lcddev.height-20,RED);	//����3
					break;
				case 3:
 					TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);			//�����3
 					TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);	//����4
					break;
				case 4:	 //ȫ���ĸ����Ѿ��õ�
	    		    //�Ա����
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,2�ľ���
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�3,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,3�ľ���
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
						continue;
					}//��ȷ��
								   
					//�Խ������
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,4�ľ���
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,3�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
						continue;
					}//��ȷ��
					//������
					tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//�õ�xfac		 
					tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//�õ�xoff
						  
					tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//�õ�yfac
					tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//�õ�yoff  
					if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//������Ԥ����෴��.
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
   	 					TP_Drow_Touch_Point(20,20,RED);								//����1
						LCD_ShowString(40,26, 16,"TP Need readjust!",1);
						tp_dev.touchtype=!tp_dev.touchtype;//�޸Ĵ�������.
						if(tp_dev.touchtype)//X,Y��������Ļ�෴
						{
							CMD_RDX=0X90;
							CMD_RDY=0XD0;	 
						}else				   //X,Y��������Ļ��ͬ
						{
							CMD_RDX=0XD0;
							CMD_RDY=0X90;	 
						}			    
						continue;
					}		
					POINT_COLOR=BLUE;
					LCD_Clear(WHITE);//����
					LCD_ShowString(35,110, 16,"Touch Screen Adjust OK!",1);//У�����
					HAL_Delay(300);
					TP_Save_Adjdata(); //����У׼���� 
 					LCD_Clear(WHITE);//����   
					return;//У�����				 
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
  //��һ���϶���Ҫ����IO�ĳ�ʼ������������������ʹ�õ������ģ��SPI������IO����SPI�̶��˿����ƣ�ͬ����CubeMX���涨���ǩ���IO�Ϳ�����
  TP_Read_XY(&tp_dev.x,&tp_dev.y);//��һ�ζ�ȡ��ʼ��	 
 	//���ﱾ��ʹ��EEPROM��Ҫ��ʼ��EEPROM���������õ����ڲ�FLASH���Բ���Ҫ��ʼ��
	if(TP_Get_Adjdata())return 0;  //��һ���ǴӴ洢������ȡ������У׼�������������û��У׼����Ҫ����У׼
	else			   //δУ׼?
	{ 										    
		LCD_Clear(WHITE);//����
	  TP_Adjust();  //��ĻУ׼ 
		//TP_Save_Adjdata();	 
	}			
	TP_Get_Adjdata();	
	return 1; 									 
}

//��ʾ�ַ���
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
	LCD_ShowNum(40+24,160,x0,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,160,y0,4,16);	//��ʾ��ֵ
	LCD_ShowNum(40+24,180,x1,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,180,y1,4,16);	//��ʾ��ֵ
	LCD_ShowNum(40+24,200,x2,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,200,y2,4,16);	//��ʾ��ֵ
	LCD_ShowNum(40+24,220,x3,4,16);		//��ʾ��ֵ
	LCD_ShowNum(40+24+80,220,y3,4,16);	//��ʾ��ֵ
 	LCD_ShowNum(40+56,lcddev.width,fac,3,16); 	//��ʾ��ֵ,����ֵ������95~105��Χ֮��.
}

//���Դ����Ĵ���
u16 ColorTab[5]={RED,GREEN,BLUE,YELLOW,BRED};//������ɫ����
void Touch_Test(void)
{
	//u8 key;
	u16 i=0;
	u16 j=0;
	u16 colorTemp=0;
	TP_Init();
	LCD_Clear(WHITE);
	LCD_ShowString(0,20,16,"TOUCH TEST",1);
	LCD_ShowString(lcddev.width-24,0,16,"RST",1);//��ʾ��������
	POINT_COLOR=RED;
	LCD_Fill(lcddev.width-52,2,lcddev.width-50+20,18,RED); 
	while(1)
	{
	 	//key=0;
		tp_dev.scan(0); 		 
		//printf("x:%d,y:%d",tp_dev.x,tp_dev.y);
		if(tp_dev.sta&TP_PRES_DOWN)			//������������
		{	
		 	if(tp_dev.x<lcddev.width&&tp_dev.y<lcddev.height)
			{	
				if(tp_dev.x>(lcddev.width-24)&&tp_dev.y<16)
				{
					LCD_Clear(WHITE);
	        LCD_ShowString(0,20,16,"TOUCH TEST",1);
					LCD_ShowString(lcddev.width-24,0,16,"RST",1);//��ʾ��������
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

				else TP_Draw_Big_Point(tp_dev.x,tp_dev.y,POINT_COLOR);		//��ͼ	  			   
			}
		}else HAL_Delay(20);	//û�а������µ�ʱ�� 	 
    //������һ����������������PA0�ţ�����������ͻ�����У����Ļ 		
		
	}   
}





