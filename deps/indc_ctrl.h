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

//обработки кнопки
extern uint8_t cnt;
extern uint8_t flag_button;
extern float shift;

//номер цирфы на индикаторе
extern uint8_t num_count;
extern int8_t pitch;

void show_num(uint8_t num);
void timer_init();
void ext_init();
