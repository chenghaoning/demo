#include "delay.h"
void delay_us(uint32_t n)
{
	while(n--)
	{
		SysTick->CTRL=0;
		SysTick->LOAD=(SystemCoreClock/1000000)-1;
		SysTick->VAL=0;
		SysTick->CTRL=5;
		while((SysTick->CTRL&0x10000)==0);
	}
		SysTick->CTRL=0;
}
