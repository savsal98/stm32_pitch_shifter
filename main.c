#include <gpio.h>
#include <i2s.h>
#include <adc.h>
#include <indc_ctrl.h>
#include <signal_prcs.h>
#include <clock.h>
#include <emul_eeprom.h>

hpf_data FilterStruct;

void signal_proc(uint8_t adc_buff_idx, uint8_t dac_buff_idx);

int main(void)
{
	clock_init();
	gpio_init();
	ext_init();
	adc_init();
	i2s_init();
	timer_init();
	pot_adc_value = read_adc();
	pitch = (int8_t)flash_read(0);
	shift = powf(2, ((float)pitch/12.0));

	// high pass filter init, parameter is a cut off frequency
	FilterStruct.alpha = filter_init(500.0);

    while(1);
}
void DMA1_Channel5_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_FLAG_HT5) != RESET)
	{
		signal_proc(0,2);
		DMA_ClearITPendingBit(DMA1_IT_HT5);
	}

	if(DMA_GetITStatus(DMA1_FLAG_TC5) != RESET)
	{
		signal_proc(4,6);

		DMA_ClearITPendingBit(DMA1_IT_TC5);
	}
}


void DMA1_Channel4_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC4);
}

void signal_proc(uint8_t adc_buff_idx, uint8_t dac_buff_idx)
{
	//read data from buffer and converting to int32 format
	int32_t left_ch_data = (int32_t)(((uint32_t)(adc_buff[adc_buff_idx]))<<8) | ((uint32_t)(adc_buff[adc_buff_idx+1]>>8));
	if(left_ch_data & (1<<23))
	{
		left_ch_data |= 0xFF000000;
	}

	int32_t pitched_value;

	//enable pitch shifting if pitch value is not equal 0
	if (pitch !=0)
	{
		//high pass filter
		FilterStruct.input_now = left_ch_data;
		FilterStruct.filtered_now = (int32_t)(FilterStruct.alpha*(FilterStruct.filtered_prev +
				FilterStruct.input_now - FilterStruct.input_prev));
		FilterStruct.input_prev = FilterStruct.input_now;
		FilterStruct.filtered_prev = FilterStruct.filtered_now;

		float adc_f = (float)pot_adc_value/4095.0;

		pitched_value = (pitch_func(FilterStruct.filtered_now));
		//signal mixing using adc pot value
		pitched_value = (int32_t)(pitched_value*adc_f + left_ch_data*(1-adc_f));
	}
	else
	{
		pitched_value = (int32_t)(left_ch_data);
	}
	//write data to dac buffer
	dac_buff[dac_buff_idx] = (uint16_t)((uint32_t)pitched_value>>8);
	dac_buff[dac_buff_idx+1] = (uint16_t)((pitched_value<<8)&0xFF00);
}





