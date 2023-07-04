#include <gpio.h>
#include <i2s.h>
#include <adc.h>
#include <indc_ctrl.h>
#include <signal_prcs.h>
hpf_data filter_1;
int32_t left_ch;

void CLOCK_INIT();

int main(void)
{
	CLOCK_INIT();
	GPIO_INIT();
	EXT_INIT();
	ADC_INIT();
	I2S_INIT();
	TIMER_INIT();
	adc_result = READ_ADC();

	filter_1.alpha = FILTER_INIT(300.0);

    while(1)
    {

    }
}
void CLOCK_INIT()
{
	RCC_DeInit();
	RCC_HSICmd(DISABLE); // выключение внутреннего RC HSI 8ћ√ц генератора

	RCC_HSEConfig(RCC_HSE_Bypass);

	//”станавливаем частоту 56 мгц
	RCC_PLLConfig(RCC_PLLSource_PREDIV1,RCC_PLLMul_7);
	//шина AHB
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	//шина APB1
	RCC_PCLK1Config(RCC_HCLK_Div2);
	//шина APB2
	RCC_PCLK2Config(RCC_HCLK_Div1);
	//»сточник I2S
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_SYSCLK);
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); /// Wait till PLL is ready
	//выбор источника PLLCLK
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource() != 0x08);
}



void DMA1_Channel5_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_FLAG_TC5) != RESET)					// for DMA_IT_TC
	{
		left_ch = (int32_t)(((uint32_t)(adc[0]))<<8) | ((uint32_t)(adc[1]>>8));
		if(left_ch & (1<<23)){
			left_ch |= 0xFF000000;
		}

		filter_1.input_now = left_ch;
		filter_1.filtered_now = (int32_t)(filter_1.alpha*(filter_1.filtered_prev +
				filter_1.input_now - filter_1.input_prev));

		filter_1.input_prev = filter_1.input_now;
		filter_1.filtered_prev = filter_1.filtered_now;

		float adc_f = (float)adc_result/4095.0;
		int32_t out;

		//enable pitch shifting
		if (pitch !=0)
		{
			out = (DO_PITCH(filter_1.filtered_now));
			out = (int32_t)(out*adc_f + left_ch*(1-adc_f));
		}
		else
		{
			out = (int32_t)(left_ch);
		}

		dac[2] = (uint16_t)((uint32_t)out>>8);
		dac[3] = (uint16_t)((out<<8)&0xFF00);

		DMA_ClearITPendingBit(DMA1_IT_TC5);
	}
}


void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_FLAG_TC4) != RESET)					// for DMA_IT_TC
	{
		DMA_ClearITPendingBit(DMA1_IT_TC4);
	}
}



