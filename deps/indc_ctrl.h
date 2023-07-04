#include <stm32f30x.h>
#include <stm32f30x_misc.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_tim.h>
#include <stm32f30x_exti.h>
#include <stm32f30x_gpio.h>
#include <stm32f30x_syscfg.h>
#include <math.h>

NVIC_InitTypeDef NVIC_InitStructure;
TIM_TimeBaseInitTypeDef TIM_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;

//��������� ������
extern uint8_t cnt;
extern uint8_t flag_button;
extern float Shift;

//����� ����� �� ����������
extern uint8_t num_count;
extern int8_t pitch;

void SHOW_NUM(uint8_t num);
void TIMER_INIT();
void EXT_INIT();