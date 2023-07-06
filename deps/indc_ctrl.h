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

extern uint8_t cnt; ///counter for button debounce
extern uint8_t flag_button; ///flag for button debounce it will be setting if button is pushed
extern float shift; ///pitch frequency relation

extern uint8_t digit_cnt; ///position of digit in indicator
extern int8_t pitch; ///pitch value

void show_num(uint8_t num, uint16_t digit_position);
void timer_init();
void ext_init();
void button_handling();
void indicator_handling();

