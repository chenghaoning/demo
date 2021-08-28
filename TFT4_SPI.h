//��������Ǹ��ݹٷ��������ļ���ֲΪHAL������  �õ������IIC

#ifndef __TFT4_SPI_H
#define __TFT4_SPI_H		
#include "main.h"	 
#include "stdlib.h"

/* ************************************��õļ�������**************************************** */
//  LCD_Init();  //��Ļ��ʼ��
//	LCD_direction(2);    //���淽������  0-0����ת��1-90����ת��2-180����ת��3-270����ת
//	LCD_ShowString(0,0,12,"HELLO WORLD",1);  //��ʾ12�������ַ���
//	LCD_ShowString(0,20,16,"HELLO WORLD",1);  //��ʾ24�������ַ���
//	Gui_StrCenter(0,40,RED,WHITE,"ȫ������",16,1);  //������ʾ
//	Show_Str(0, 60, RED, WHITE,  "ʮ����������Ļ����",16,1);  //��ʾ�����ַ��������������ֵ������ֵ�����ֻ��16�ĺ�12��
//	Show_Str(0, 100, RED, WHITE, "��ʮ�ĺ�������Ļ����",24,1);  //��ʾ�����ַ��������������ֵ������ֵ�����ֻ��16�ĺ�12��
//	Show_Str(0, 140, RED, WHITE, "��ʮ����������Ļ����",32,1);  //��ʾ�����ַ��������������ֵ������ֵ�����ֻ��16�ĺ�12��
//	showimage(gImage_PDD,240,320,0,239);
/* ************************************��õļ�������**************************************** */


#define u16 unsigned short
#define u8 unsigned char

//LCD��Ҫ������
typedef struct  
{										    
	u16 width;			//LCD ���
	u16 height;			//LCD �߶�
	u16 id;				  //LCD ID
	u8  dir;			  //���������������ƣ�0��������1��������	
	u16	 wramcmd;		//��ʼдgramָ��
	u16  setxcmd;		//����x����ָ��
	u16  setycmd;		//����y����ָ��	 
}_lcd_dev; 	

//LCD����
extern _lcd_dev lcddev;	//����LCD��Ҫ����
/////////////////////////////////////�û�������///////////////////////////////////	 
#define USE_HORIZONTAL  	 0//����Һ����˳ʱ����ת���� 	0-0����ת��1-90����ת��2-180����ת��3-270����ת

//////////////////////////////////////////////////////////////////////////////////	  
//����LCD�ĳߴ�
#define LCD_W 240
#define LCD_H 320

//TFTLCD������Ҫ���õĺ���		   
extern u16  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern u16  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ


//-----------------  LCD�˿ڶ���ֱ�Ӳ���Ҫ�ˣ�ֻ��Ҫ��STM32CubeMX�����ú�IO�ڲ��ұ�ǩ��ã�ֱ�����Ӿ�����  ---------------- 
#define	LCD_BLK_SET	HAL_GPIO_WritePin(TFT4_SPI_BLK_GPIO_Port, TFT4_SPI_BLK_Pin, GPIO_PIN_SET); //LCD����򿪣�QDtechȫϵ��ģ������������ܿ��Ʊ��������û�Ҳ���Խ�PWM���ڱ�������
//���ʹ�ùٷ��⺯���������еײ㣬�ٶȽ����½���14֡ÿ�룬����������ǹ�˾�Ƽ�����
//����IO����ֱ�Ӳ����Ĵ���������IO������ˢ�����ʿ��Դﵽ28֡ÿ�룡 


//GPIO���ܿ���λ�����ߣ�
#define	LCD_CS_SET  HAL_GPIO_WritePin(TFT4_SPI_CS_GPIO_Port, TFT4_SPI_CS_Pin, GPIO_PIN_SET);  
#define	LCD_RS_SET	HAL_GPIO_WritePin(TFT4_SPI_DC_GPIO_Port, TFT4_SPI_DC_Pin, GPIO_PIN_SET);     //����/����     (DC RS ָ����ͬһ�����Ź�) 
#define	LCD_RST_SET	HAL_GPIO_WritePin(TFT4_SPI_RST_GPIO_Port, TFT4_SPI_RST_Pin, GPIO_PIN_SET);    //��λ			 
//GPIO���ܿڸ�λ�����ͣ�							    
#define	LCD_CS_CLR  HAL_GPIO_WritePin(TFT4_SPI_CS_GPIO_Port, TFT4_SPI_CS_Pin, GPIO_PIN_RESET);     
#define	LCD_RS_CLR	HAL_GPIO_WritePin(TFT4_SPI_DC_GPIO_Port, TFT4_SPI_DC_Pin, GPIO_PIN_RESET);      //����/����  
#define	LCD_RST_CLR	HAL_GPIO_WritePin(TFT4_SPI_RST_GPIO_Port, TFT4_SPI_RST_Pin, GPIO_PIN_RESET);     //��λ			

//GPIOͨ�ſ���λ�����ߣ�
#define	SPI_MOSI_SET  	HAL_GPIO_WritePin(TFT4_SPI_MOSI_GPIO_Port, TFT4_SPI_MOSI_Pin, GPIO_PIN_SET); 
#define	SPI_SCLK_SET  	HAL_GPIO_WritePin(TFT4_SPI_SCK_GPIO_Port, TFT4_SPI_SCK_Pin, GPIO_PIN_SET); 
//GPIOͨ�ſڸ�λ�����ͣ�
#define	SPI_MOSI_CLR  	HAL_GPIO_WritePin(TFT4_SPI_MOSI_GPIO_Port, TFT4_SPI_MOSI_Pin, GPIO_PIN_RESET); 
#define	SPI_SCLK_CLR  	HAL_GPIO_WritePin(TFT4_SPI_SCK_GPIO_Port, TFT4_SPI_SCK_Pin, GPIO_PIN_RESET); 

//������ɫ
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 //��ɫ
#define BRRED 			0XFC07 //�غ�ɫ
#define GRAY  			0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	0X841F //ǳ��ɫ
#define LIGHTGRAY     0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			 		0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE      	0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE          0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)
	    															  
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(u16 Color);	 
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);//����
u16  LCD_ReadPoint(u16 x,u16 y); //����
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);


u16 LCD_RD_DATA(void);//��ȡLCD����									    
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
void LCD_WR_DATA(u8 data);
u16 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
u16 LCD_ReadRAM(void);		   
u16 LCD_BGR2RGB(u16 c);
void LCD_SetParam(void);
void Lcd_WriteData_16Bit(u16 Data);
void LCD_direction(u8 direction );


void GUI_DrawPoint(u16 x,u16 y,u16 color);
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);


void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void Draw_Circle(u16 x0,u16 y0,u16 fc,u8 r);
void Draw_Triangel(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2);
void Fill_Triangel(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2);


void LCD_ShowChar(u16 x,u16 y,u16 fc, u16 bc, u8 num,u8 size,u8 mode);
void LCD_ShowNum(u16 x,u16 y,unsigned int num,u8 len,u8 size);
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len,u8 size,u8 mode);
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode);
void GUI_DrawFont16(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode);
void GUI_DrawFont24(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode);
void GUI_DrawFont32(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode);
void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);


//void Gui_Drawbmp16(u16 x,u16 y,const unsigned char *p); //��ʾ40*40 QQͼƬ
void gui_circle(int xc, int yc,u16 c,int r, int fill);
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2);

void showimage(const unsigned char *p,unsigned short length,unsigned short width,unsigned short x,unsigned short y);

//������һ����Ҳû���о�����û���ã��ٷ���Դ����������һ�ζ��ұ�������ע���˵�

//�����Ȼ�����ٶȲ����죬����ʹ������ĺ궨��,����ٶ�.
//ע��Ҫȥ��lcd.c��void LCD_WR_DATA(u16 data)��������Ŷ
/*
#if LCD_USE8BIT_MODEL==1//ʹ��8λ������������ģʽ
	#define LCD_WR_DATA(data){\
	LCD_RS_SET;\
	LCD_CS_CLR;\
	DATAOUT(data);\
	LCD_WR_CLR;\
	LCD_WR_SET;\
	DATAOUT(data<<8);\
	LCD_WR_CLR;\
	LCD_WR_SET;\
	LCD_CS_SET;\
	}
	#else//ʹ��16λ������������ģʽ
	#define LCD_WR_DATA(data){\
	LCD_RS_SET;\
	LCD_CS_CLR;\
	DATAOUT(data);\
	LCD_WR_CLR;\
	LCD_WR_SET;\
	LCD_CS_SET;\
	
	
	} 	
#endif
*/	  		 
#endif  
	 
	 



