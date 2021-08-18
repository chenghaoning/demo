/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "u8g2.h"
#include "dht11.h"
/* #define CPU_FREQUENCY_MHZ    72	
 void hdelay_us(__IO uint32_t delay)
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
}  */
void delay_us(uint32_t us)
{
    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
    while (delay--)
	{
		;
	}
}
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* void delay_us(uint32_t time)
{
  uint32_t i=8*time;
  while(i--);
}
 */
uint8_t STM32_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
  case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
    __NOP();
    break;
  case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
	for (uint16_t n = 0; n < 320; n++)
    {
       __NOP();
    }
    break;
    case U8X8_MSG_DELAY_MILLI:			                // delay arg_int * 1 milli second
    	HAL_Delay(1);
        break;
    case U8X8_MSG_DELAY_I2C:				            // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
    	delay_us(5);
    	break;							                // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_I2C_CLOCK:		                // arg_int=0: Output low at I2C clock pin
        if(arg_int == 1)                                     // arg_int=1: Input dir with pullup high for I2C clock pin
        	HAL_GPIO_WritePin(GPIOB, SCL_Pin, GPIO_PIN_SET);
        else if(arg_int == 0)
        	HAL_GPIO_WritePin(GPIOB, SCL_Pin, GPIO_PIN_RESET);
    	break;
    case U8X8_MSG_GPIO_I2C_DATA:			            // arg_int=0: Output low at I2C data pin
        if(arg_int == 1)                                     // arg_int=1: Input dir with pullup high for I2C data pin
        	HAL_GPIO_WritePin(GPIOB, SDA_Pin, GPIO_PIN_SET);
        else if(arg_int == 0)
        	HAL_GPIO_WritePin(GPIOB, SDA_Pin, GPIO_PIN_RESET);
    	break;
    case U8X8_MSG_GPIO_MENU_SELECT:
        u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_NEXT:
        u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_PREV:
        u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
        break;
    case U8X8_MSG_GPIO_MENU_HOME:
        u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 1);			         // default return value
        break;
  }
  return 1;
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  
  
  //OLED_Init();
  //OLED_CLS();
  //OLED_ShowStr(0,0,"aaa",2);
  //OLED_ON();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  u8g2_t u8g2;                                                                                     // a structure which will contain all the data for one display
  u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, STM32_gpio_and_delay);  // init u8g2 structure
  u8g2_InitDisplay(&u8g2);                                                                         // send init sequence to the display, display is in sleep mode after this,
  u8g2_SetPowerSave(&u8g2, 0);    
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    char temp[10],humi[10];
  DHT11_Data_TypeDef Data;
  if (DHT11_ReadData(&Data))
    {
      // DEBUG_INFO("\r\n读取DHT11成功!\r\n\r\n湿度�?%d.%d ％RH ，温度为 %d.%d�? \r\n",					\
      DHT11_Data.humi_int,DHT11_Data.humi_deci,DHT11_Data.temp_int,DHT11_Data.temp_deci);
      sprintf(temp,"%s:%d.%d","temp",Data.temp_int,Data.temp_deci);
      sprintf(humi,"%s:%d.%d%s","humi",Data.humi_int,Data.humi_deci,"%");
      // sprintf(temp,"%s","hello");
      // sprintf(humi,"%s","world");
      HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,RESET);
      u8g2_ClearBuffer(&u8g2);
      u8g2_SetFont(&u8g2, u8g2_font_10x20_mf);
      u8g2_DrawStr(&u8g2, 1,13,temp);
      u8g2_DrawCircle(&u8g2,97,3,2,U8G2_DRAW_ALL);
      u8g2_DrawStr(&u8g2,101,13,"C");
      u8g2_DrawStr(&u8g2, 1,32,humi);
      u8g2_DrawStr(&u8g2, 0,52,"Love hwh");
      u8g2_SetFont(&u8g2, u8g2_font_unifont_t_0_78_79);
      u8g2_DrawGlyph(&u8g2,80,50,0x2764);
      u8g2_SetFont(&u8g2, u8g2_font_open_iconic_weather_4x_t);
      u8g2_DrawGlyph(&u8g2,100,50,0x45);
      u8g2_SendBuffer(&u8g2);
    }
    else
    {
      // DEBUG_INFO("Read DHT11 ERROR!\r\n");
      HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,SET);
      u8g2_ClearBuffer(&u8g2);
      u8g2_SetFont(&u8g2, u8g2_font_10x20_mf);
      u8g2_DrawStr(&u8g2, 64,32,"error");
      u8g2_SendBuffer(&u8g2);
    }
    HAL_Delay(3000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
