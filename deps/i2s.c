#include "i2s.h"

uint16_t dac[4] = {0};
uint16_t adc[4] = {0};

void I2S_INIT()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	/* Общая для чтения и зап */
	DMA_InitStructure.DMA_BufferSize = 4;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	/* DMA I2S запись */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dac;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* DMA I2S чтение */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2S2ext->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable SPI APB clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_24b;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_44k;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
	I2S_Init(SPI2, &I2S_InitStructure);
	I2S_FullDuplexConfig(I2S2ext,&I2S_InitStructure);

	/* Enable the DMA_CHANNEL_RX transfer complete */
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);

	/* Enable the DMA_CHANNEL_TX transfer complete */
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);

	/* Enable the DMA channel Rx */
	DMA_Cmd(DMA1_Channel4, ENABLE);

	/* Enable the DMA channel Tx */
	DMA_Cmd(DMA1_Channel5, ENABLE);

	/* Enable the I2Sext RX DMA request */
	SPI_I2S_DMACmd(I2S2ext, SPI_I2S_DMAReq_Rx, ENABLE);

	/* Enable the I2S TX DMA request */
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

	/* Enable the SPI Master peripheral */
	I2S_Cmd(SPI2, ENABLE);

	/* Enable the I2Sext peripheral */
	I2S_Cmd(I2S2ext, ENABLE);
}



