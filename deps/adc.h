#include <stm32f30x_adc.h>
#include <stm32f30x_dma.h>
#include <stm32f30x_rcc.h>

extern uint32_t adc_result;

ADC_InitTypeDef ADC_InitStructure;
ADC_CommonInitTypeDef ADC_CommonInitStructure;
DMA_InitTypeDef DMA_InitStructure;

void ADC_INIT();
unsigned int READ_ADC();
