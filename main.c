#include <gpio.h>
#include <i2s.h>
#include <adc.h>
#include <indc_ctrl.h>
#include <signal_prcs.h>

hpf_data FilterStruct;
void clock_init();

int main(void)
{
	clock_init();
	gpio_init();
	ext_init();
	adc_init();
	i2s_init();
	timer_init();
	adc_result = read_adc();

	// high pass filter init, parameter is a cut off frequency
	FilterStruct.alpha = filter_init(500.0);

    while(1);
}
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



void DMA1_Channel5_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_FLAG_TC5) != RESET)
	{
		//read data from buffer and converting to int32 format
		int32_t left_ch_data = (int32_t)(((uint32_t)(adc_buff[0]))<<8) | ((uint32_t)(adc_buff[1]>>8));
		if(left_ch_data & (1<<23))
		{
			left_ch_data |= 0xFF000000;
		}

		//high pass filter
		FilterStruct.input_now = left_ch_data;
		FilterStruct.filtered_now = (int32_t)(FilterStruct.alpha*(FilterStruct.filtered_prev +
				FilterStruct.input_now - FilterStruct.input_prev));
		FilterStruct.input_prev = FilterStruct.input_now;
		FilterStruct.filtered_prev = FilterStruct.filtered_now;

		float adc_f = (float)adc_result/4095.0;
		int32_t pitch_value;

		//enable pitch shifting if pitch value is not equal 0
		if (pitch !=0)
		{
			pitch_value = (pitch_func(FilterStruct.filtered_now));
			//signal mixing via adc pot value
			pitch_value = (int32_t)(pitch_value*adc_f + left_ch_data*(1-adc_f));
		}
		else
		{
			pitch_value = (int32_t)(left_ch_data);
		}

		//write data to dac buffer
		dac_buff[2] = (uint16_t)((uint32_t)pitch_value>>8);
		dac_buff[3] = (uint16_t)((pitch_value<<8)&0xFF00);

		DMA_ClearITPendingBit(DMA1_IT_TC5);
	}
}


void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_FLAG_TC4) != RESET)					// for DMA_IT_TC
	{
		//read data from buffer and converting to int32 format
		int32_t left_ch_data = (int32_t)(((uint32_t)(adc_buff[4]))<<8) | ((uint32_t)(adc_buff[5]>>8));
		if(left_ch_data & (1<<23))
		{
			left_ch_data |= 0xFF000000;
		}

		//high pass filter
		FilterStruct.input_now = left_ch_data;
		FilterStruct.filtered_now = (int32_t)(FilterStruct.alpha*(FilterStruct.filtered_prev +
				FilterStruct.input_now - FilterStruct.input_prev));
		FilterStruct.input_prev = FilterStruct.input_now;
		FilterStruct.filtered_prev = FilterStruct.filtered_now;

		float adc_f = (float)adc_result/4095.0;
		int32_t pitch_value;

		//enable pitch shifting if pitch value is not equal 0
		if (pitch !=0)
		{
			pitch_value = (pitch_func(FilterStruct.filtered_now));
			//signal mixing via adc pot value
			pitch_value = (int32_t)(pitch_value*adc_f + left_ch_data*(1-adc_f));
		}
		else
		{
			pitch_value = (int32_t)(left_ch_data);
		}

		//write data to dac buffer
		dac_buff[6] = (uint16_t)((uint32_t)pitch_value>>8);
		dac_buff[7] = (uint16_t)((pitch_value<<8)&0xFF00);

		DMA_ClearITPendingBit(DMA1_IT_TC4);
	}
}



