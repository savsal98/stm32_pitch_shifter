#include <indc_ctrl.h>


uint8_t cnt = 0;///button debounce counter
uint8_t flag_button = 0; ///button interrupt flag
uint8_t num_count = 0; ///digit number in indicator
int8_t pitch = 0; ///pitch value
float shift = 1.0;//pitch frequency relation



void show_num(uint8_t num)
{
	/*
	 * cathods
	 *  PA5 - ןונגאÿ צטפנא
	 *  PA4 - געמנאÿ צטפנא
	 *  PA3 - ענועüÿ צטנפא
	 * 	Àםמהû
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

void timer_init()
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
void TIM7_IRQHandler(void)
{
	//pitch absolute value calculation
	int8_t abs_pitch = pitch;
	uint8_t temp = abs_pitch >> 7;
	abs_pitch ^= temp;
	abs_pitch += temp & 1;
	if (abs_pitch < 10)
	{
		if(num_count == 0)
		{
			GPIOA->ODR |= 0x38;
			GPIOA->ODR &= ~ (1<<3);
			show_num((abs_pitch%10));
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

	if (abs_pitch >= 10)
	{
			if(num_count == 0)
			{
				GPIOA->ODR |= 0x38;
				GPIOA->ODR &= ~ (1<<3);
				show_num((abs_pitch%10));
				num_count++;
			}
			else if(num_count == 1)
			{
				GPIOA->ODR |= 0x38;
				GPIOA->ODR &= ~ (1<<4);
				show_num(((abs_pitch/10)%10));
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
	if(flag_button == 1 && cnt <= 10)
	{
		cnt++;
	}
	else if (flag_button == 1 && cnt > 10)
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) != RESET)
		{
			if (pitch <= 11)
			{
			  pitch++;
			  shift = powf(2, ((float)pitch/12.0));
			}
		}
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) != RESET)
		{
			if (pitch >= -11)
			{
			  pitch--;
			  EXTI_ClearITPendingBit(EXTI_Line7);
			  shift = powf(2, ((float)pitch/12.0));
			}
		}
		//button status check
		flag_button = 0;
		cnt = 0;
	}
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
}


void ext_init()
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
