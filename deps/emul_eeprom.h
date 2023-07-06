#include "stm32f30x_flash.h"

#define FLASH_PAGE_SIZE         ((uint32_t)0x00000800)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR   ((uint32_t)0x08006000)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t)0x08007000)   /* End @ of user Flash area */
#define DATA_32                 ((uint32_t)4)


extern uint32_t EraseCounter;
extern uint32_t Address;
extern uint32_t Data;
extern uint32_t NbrOfPage;

void flash_write(uint32_t addr, uint32_t value);
uint32_t flash_read(uint32_t addr);
