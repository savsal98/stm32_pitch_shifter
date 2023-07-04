#include <stm32f30x.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_gpio.h>
#include <stm32f30x_spi.h>
#include <stm32f30x_dma.h>
#include <stm32f30x_adc.h>
#include <stm32f30x_misc.h>
#include <stm32f30x_tim.h>
#include <stm32f30x_exti.h>
#include <stm32f30x_syscfg.h>


#include <math.h>

typedef struct filters_data {
	  int32_t filtered_now;
	  int32_t filtered_prev;
	  int32_t input_now;
	  int32_t input_prev;
	  float alpha;

} hpf_data;
hpf_data filter_1;

GPIO_InitTypeDef GPIO_InitStructure;
I2S_InitTypeDef I2S_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
ADC_InitTypeDef ADC_InitStructure;
ADC_CommonInitTypeDef ADC_CommonInitStructure;
TIM_TimeBaseInitTypeDef TIM_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;

uint16_t dac[4] = {0};
uint16_t adc[4] = {0};
uint32_t adc_resut;
//номер цирфы на индикаторе
volatile uint8_t num_count = 0;
volatile int8_t pitch = 0;
volatile int32_t left_ch;

//обработки кнопки
volatile uint8_t cnt = 0;
volatile uint8_t flag_button = 0;

#define BufSize 4000
#define Overlap 500
int Buf[BufSize];
int WtrP;
float Rd_P;
float Shift;
float CrossFade;

void CLOCK_INIT();
void GPIO_INIT();
void DELAY();
void I2S_INIT();
void DMA_INIT();
int DO_PITCH(int Sample);
void ADC_INIT();
unsigned int READ_ADC();
void SHOW_NUM(uint8_t num);
void TIMER_INIT();
void EXT_INIT();

void _delay(uint32_t t){
	t*=7.2;
	for (uint32_t j; j<t; j++){};
}

int main(void)
{
	WtrP = 0;
	Rd_P = 0.0f;
	Shift = 1.0f;
	CrossFade = 1.0f;

	//частота среза фильтра
	float rc = 1.0/(2.0*3.14159*300.0);
	filter_1.alpha = rc / (rc + 1.0/44000.0);

	CLOCK_INIT();
	GPIO_INIT();
	EXT_INIT();
	DMA_INIT();
	ADC_INIT();
	I2S_INIT();
	TIMER_INIT();
	adc_resut = READ_ADC();


    while(1)
    {

    }
}
void CLOCK_INIT()
{
	RCC_DeInit();
	RCC_HSICmd(DISABLE); // выключение внутреннего RC HSI 8МГц генератора

	RCC_HSEConfig(RCC_HSE_Bypass);

	//Устанавливаем частоту 56 мгц
	RCC_PLLConfig(RCC_PLLSource_PREDIV1,RCC_PLLMul_7);
	//шина AHB
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	//шина APB1
	RCC_PCLK1Config(RCC_HCLK_Div2);
	//шина APB2
	RCC_PCLK2Config(RCC_HCLK_Div1);
	//Источник I2S
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_SYSCLK);
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); /// Wait till PLL is ready
	//выбор источника PLLCLK
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while(RCC_GetSYSCLKSource() != 0x08);
}


void GPIO_INIT()
{
	// GPIOB 12 - I2S2_WS, 13 - I2S2_CK, 14 - I2S2ext_SD, 15 - I2S2_SD
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB|
			RCC_AHBPeriph_GPIOC|
			RCC_AHBPeriph_GPIOE|
			RCC_AHBPeriph_GPIOA|
			RCC_AHBPeriph_GPIOD
			, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//GPIOC 6 - I2S2_MCK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Альтернативные функции для I2S
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_6);

	//для индикации
	GPIO_InitStructure.GPIO_Pin = 0xFF00;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//ацп
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//индикация
	//аноды
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = 0x00FF;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//катоды
	GPIO_InitStructure.GPIO_Pin = 0x0038;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//управление
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}



void I2S_INIT()
{
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

void DMA_INIT()
{
	/* Enable the DMA AHB clocks */
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

	// DMA Для АЦП
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&adc_resut;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_BufferSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1,ENABLE);

}

void DMA1_Channel5_IRQHandler(void)
{
	GPIO_SetBits(GPIOE, GPIO_Pin_9);
	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
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

		float adc_f = (float)adc_resut/4095.0;
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
	GPIO_SetBits(GPIOE, GPIO_Pin_8);
	GPIO_ResetBits(GPIOE, GPIO_Pin_8);

	if(DMA_GetITStatus(DMA1_FLAG_TC4) != RESET)					// for DMA_IT_TC
	{
		DMA_ClearITPendingBit(DMA1_IT_TC4);
	}
}

int DO_PITCH(int Sample) {
	int sum = Sample;

	//write to ringbuffer
	Buf[WtrP] = sum;

	//read fractional readpointer and generate 0° and 180° read-pointer in integer
	int RdPtr_Int = roundf(Rd_P);
	int RdPtr_Int2 = 0;
	if (RdPtr_Int >= BufSize/2) RdPtr_Int2 = RdPtr_Int - (BufSize/2);
	else RdPtr_Int2 = RdPtr_Int + (BufSize/2);

	//read the two samples...
	float Rd0 = (float) Buf[RdPtr_Int];
	float Rd1 = (float) Buf[RdPtr_Int2];

	//Check if first readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to second read-pointer
	if (Overlap >= (WtrP-RdPtr_Int) && (WtrP-RdPtr_Int) >= 0 && Shift!=1.0f) {
		int rel = WtrP-RdPtr_Int;
		CrossFade = ((float)rel)/(float)Overlap;
	}
	else if (WtrP-RdPtr_Int == 0) CrossFade = 0.0f;

	//Check if second readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to first read-pointer
	if (Overlap >= (WtrP-RdPtr_Int2) && (WtrP-RdPtr_Int2) >= 0 && Shift!=1.0f) {
			int rel = WtrP-RdPtr_Int2;
			CrossFade = 1.0f - ((float)rel)/(float)Overlap;
		}
	else if (WtrP-RdPtr_Int2 == 0) CrossFade = 1.0f;


	//do cross-fading and sum up
	sum = (Rd0*CrossFade + Rd1*(1.0f-CrossFade));

	//increment fractional read-pointer and write-pointer
	Rd_P += Shift;
	WtrP++;
	if (WtrP == BufSize) WtrP = 0;
	if (roundf(Rd_P) >= BufSize) Rd_P = 0.0f;

	return sum;

}

void ADC_INIT()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div10);

	ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
	ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;
	ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);

	ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigInjecEventEdge_None;
	ADC_InitStructure.ADC_NbrOfRegChannel = 1;
	ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;

	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Cmd(ADC1,ENABLE);
}

unsigned int READ_ADC()
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	ADC_StartConversion(ADC1);
	return ADC_GetConversionValue(ADC1);

}

void SHOW_NUM(uint8_t num)
{
	/*
	 * Катоды слева направо
	 *  PA5 - первая цифра
	 *  PA4 - вторая цифра
	 *  PA3 - третья цирфа
	 * 	Аноды
	 * 	a - PD7
	 * 	b - PD0
	 * 	c - PD2
	 * 	d - PD4
	 * 	e - PD5
	 * 	f - PD6
	 * 	g - PD1
	 * 	dot - PD3
	 * 	0000 0000 afed .cgb
	 */

	GPIOD->ODR &= ~(0x00FF);
	switch ((uint8_t)num){
		case 0:
			GPIOD->ODR |= 0xF5;
			break;
		case 1:
			GPIOD->ODR |= 0x0005;
			break;
		case 2:
			GPIOD->ODR |= 0x00B3;
			break;
		case 3:
			GPIOD->ODR |= 0x0097;
			break;
		case 4:
			GPIOD->ODR |= 0x0047;
			break;
		case 5:
			GPIOD->ODR |= 0x00D6;
			break;
		case 6:
			GPIOD->ODR |= 0x00F6;
			break;
		case 7:
			GPIOD->ODR |= 0x0085;
			break;
		case 8:
			GPIOD->ODR |= 0x00F7;
			break;
		case 9:
			GPIOD->ODR |= 0x00D7;
			break;
	}
}

void TIMER_INIT()
{
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStructure.TIM_Period = 100;
	TIM_InitStructure.TIM_Prescaler = 3600;
	TIM_TimeBaseInit(TIM7, &TIM_InitStructure);
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM7, ENABLE);
}
void TIM7_IRQHandler(void){
	//pitch absolute value calculation
	int8_t abs_pitch = pitch;
	uint8_t temp = abs_pitch >> 7;
	abs_pitch ^= temp;
	abs_pitch += temp & 1;
	if (abs_pitch < 10){
		if(num_count == 0)
		{
			GPIOA->ODR |= 0x38;
			GPIOA->ODR &= ~ (1<<3);
			SHOW_NUM((abs_pitch%10));
			num_count++;
		}
		else if(num_count == 1)
		{
			GPIOA->ODR |= 0x38;
			GPIOA->ODR &= ~ (1<<4);
			GPIOD->ODR &= ~(0xFF);
			if (pitch<0)
			{
				GPIOD->ODR |= 0x02;
			}
			num_count++;
		}
		else if(num_count == 2)
		{
			GPIOA->ODR |= 0x38;
			GPIOA->ODR &= ~  (1<<5);
			GPIOD->ODR &= ~(0xFF);
			num_count = 0;
		}
	}

	if (abs_pitch >= 10){
			if(num_count == 0)
			{
				GPIOA->ODR |= 0x38;
				GPIOA->ODR &= ~ (1<<3);
				SHOW_NUM((abs_pitch%10));
				num_count++;
			}
			else if(num_count == 1)
			{
				GPIOA->ODR |= 0x38;
				GPIOA->ODR &= ~ (1<<4);
				SHOW_NUM(((abs_pitch/10)%10));
				num_count++;
			}
			else if(num_count == 2)
			{
				GPIOA->ODR |= 0x38;
				GPIOA->ODR &= ~ (1<<5);
				GPIOD->ODR &= ~(0xFF);
				if (pitch<0)
				{
					GPIOD->ODR |= 0x02;
				}
				num_count = 0;
			}
		}
	//button counter handling
	if(flag_button == 1 && cnt <= 10){
		cnt++;
	}
	else if (flag_button == 1 && cnt > 10)
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) != RESET)
		{
			if (pitch <= 11)
			{
			  pitch++;
			  Shift = powf(2, ((float)pitch/12.0));
			}
		}
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) != RESET)
		{
			if (pitch >= -11)
			{
			  pitch--;
			  EXTI_ClearITPendingBit(EXTI_Line7);
			  Shift = powf(2, ((float)pitch/12.0));
			}
		}
		//button status check
		flag_button = 0;
		cnt = 0;
	}
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
}


void EXT_INIT()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource6);

	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI9_5_IRQHandler(void)
{
  if((EXTI_GetITStatus(EXTI_Line6) != RESET))
  {

	  flag_button = 1;

	  EXTI_ClearITPendingBit(EXTI_Line6);


  }
  else if((EXTI_GetITStatus(EXTI_Line7) != RESET))
  {
	  flag_button = 1;

	  EXTI_ClearITPendingBit(EXTI_Line7);

  }
}
