#include <stm32f30x.h>
#include <stm32f30x_spi.h>
#include <stm32f30x_dma.h>
#include <stm32f30x_misc.h>
#include <stm32f30x_rcc.h>


extern uint16_t dac[4];
extern uint16_t adc[4];

I2S_InitTypeDef I2S_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;

void I2S_INIT();
