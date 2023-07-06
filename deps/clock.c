#include <clock.h>


void clock_init()
{
	RCC_DeInit();
	RCC_HSICmd(DISABLE);
	RCC_HSEConfig(RCC_HSE_Bypass);
	//main freq is 56 MHz
	RCC_PLLConfig(RCC_PLLSource_PREDIV1,RCC_PLLMul_7);
	//AHB
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	//APB1
	RCC_PCLK1Config(RCC_HCLK_Div2);
	//APB2
	RCC_PCLK2Config(RCC_HCLK_Div1);
	//I2S source
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_SYSCLK);
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); /// Wait till PLL is ready
	//PLLCLK source
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource() != 0x08);
}
