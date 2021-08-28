//大多数都是根据官方的驱动文件移植为HAL库驱动  用的是软件IIC

#ifndef __TFT4_SPI_H
#define __TFT4_SPI_H		
#include "main.h"	 
#include "stdlib.h"

/* ************************************最常用的几个函数**************************************** */
//  LCD_Init();  //屏幕初始化
//	LCD_direction(2);    //画面方向设置  0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转
//	LCD_ShowString(0,0,12,"HELLO WORLD",1);  //显示12号字体字符串
//	LCD_ShowString(0,20,16,"HELLO WORLD",1);  //显示24号字体字符串
//	Gui_StrCenter(0,40,RED,WHITE,"全动电子",16,1);  //居中显示
//	Show_Str(0, 60, RED, WHITE,  "十六号字体屏幕测试",16,1);  //显示中文字符串，允许有数字但是数字的字体只有16的和12的
//	Show_Str(0, 100, RED, WHITE, "二十四号字体屏幕测试",24,1);  //显示中文字符串，允许有数字但是数字的字体只有16的和12的
//	Show_Str(0, 140, RED, WHITE, "三十二号字体屏幕测试",32,1);  //显示中文字符串，允许有数字但是数字的字体只有16的和12的
//	showimage(gImage_PDD,240,320,0,239);
/* ************************************最常用的几个函数**************************************** */


#define u16 unsigned short
#define u8 unsigned char

//LCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				  //LCD ID
	u8  dir;			  //横屏还是竖屏控制：0，竖屏；1，横屏。	
	u16	 wramcmd;		//开始写gram指令
	u16  setxcmd;		//设置x坐标指令
	u16  setycmd;		//设置y坐标指令	 
}_lcd_dev; 	

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
/////////////////////////////////////用户配置区///////////////////////////////////	 
#define USE_HORIZONTAL  	 0//定义液晶屏顺时针旋转方向 	0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转

//////////////////////////////////////////////////////////////////////////////////	  
//定义LCD的尺寸
#define LCD_W 240
#define LCD_H 320

//TFTLCD部分外要调用的函数		   
extern u16  POINT_COLOR;//默认红色    
extern u16  BACK_COLOR; //背景颜色.默认为白色


//-----------------  LCD端口定义直接不需要了，只需要在STM32CubeMX上设置好IO口并且标签打好，直接连接就能用  ---------------- 
#define	LCD_BLK_SET	HAL_GPIO_WritePin(TFT4_SPI_BLK_GPIO_Port, TFT4_SPI_BLK_Pin, GPIO_PIN_SET); //LCD背光打开，QDtech全系列模块采用了三极管控制背光亮灭，用户也可以接PWM调节背光亮度
//如果使用官方库函数定义下列底层，速度将会下降到14帧每秒，建议采用他们公司推荐方法
//以下IO定义直接操作寄存器，快速IO操作，刷屏速率可以达到28帧每秒！ 


//GPIO功能口置位（拉高）
#define	LCD_CS_SET  HAL_GPIO_WritePin(TFT4_SPI_CS_GPIO_Port, TFT4_SPI_CS_Pin, GPIO_PIN_SET);  
#define	LCD_RS_SET	HAL_GPIO_WritePin(TFT4_SPI_DC_GPIO_Port, TFT4_SPI_DC_Pin, GPIO_PIN_SET);     //数据/命令     (DC RS 指的是同一个引脚哈) 
#define	LCD_RST_SET	HAL_GPIO_WritePin(TFT4_SPI_RST_GPIO_Port, TFT4_SPI_RST_Pin, GPIO_PIN_SET);    //复位			 
//GPIO功能口复位（拉低）							    
#define	LCD_CS_CLR  HAL_GPIO_WritePin(TFT4_SPI_CS_GPIO_Port, TFT4_SPI_CS_Pin, GPIO_PIN_RESET);     
#define	LCD_RS_CLR	HAL_GPIO_WritePin(TFT4_SPI_DC_GPIO_Port, TFT4_SPI_DC_Pin, GPIO_PIN_RESET);      //数据/命令  
#define	LCD_RST_CLR	HAL_GPIO_WritePin(TFT4_SPI_RST_GPIO_Port, TFT4_SPI_RST_Pin, GPIO_PIN_RESET);     //复位			

//GPIO通信口置位（拉高）
#define	SPI_MOSI_SET  	HAL_GPIO_WritePin(TFT4_SPI_MOSI_GPIO_Port, TFT4_SPI_MOSI_Pin, GPIO_PIN_SET); 
#define	SPI_SCLK_SET  	HAL_GPIO_WritePin(TFT4_SPI_SCK_GPIO_Port, TFT4_SPI_SCK_Pin, GPIO_PIN_SET); 
//GPIO通信口复位（拉低）
#define	SPI_MOSI_CLR  	HAL_GPIO_WritePin(TFT4_SPI_MOSI_GPIO_Port, TFT4_SPI_MOSI_Pin, GPIO_PIN_RESET); 
#define	SPI_SCLK_CLR  	HAL_GPIO_WritePin(TFT4_SPI_SCK_GPIO_Port, TFT4_SPI_SCK_Pin, GPIO_PIN_RESET); 

//画笔颜色
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
#define BROWN 			0XBC40 //棕色
#define BRRED 			0XFC07 //棕红色
#define GRAY  			0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	0X841F //浅绿色
#define LIGHTGRAY     0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 		0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE      	0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE          0X2B12 //浅棕蓝色(选择条目的反色)
	    															  
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(u16 Color);	 
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);//画点
u16  LCD_ReadPoint(u16 x,u16 y); //读点
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd);


u16 LCD_RD_DATA(void);//读取LCD数据									    
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


//void Gui_Drawbmp16(u16 x,u16 y,const unsigned char *p); //显示40*40 QQ图片
void gui_circle(int xc, int yc,u16 c,int r, int fill);
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode);
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2);

void showimage(const unsigned char *p,unsigned short length,unsigned short width,unsigned short x,unsigned short y);

//以下这一段我也没有研究过有没有用，官方的源码里面有这一段而且本来就是注释了的

//如果仍然觉得速度不够快，可以使用下面的宏定义,提高速度.
//注意要去掉lcd.c中void LCD_WR_DATA(u16 data)函数定义哦
/*
#if LCD_USE8BIT_MODEL==1//使用8位并行数据总线模式
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
	#else//使用16位并行数据总线模式
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
	 
	 



