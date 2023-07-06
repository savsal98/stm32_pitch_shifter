#include <emul_eeprom.h>

uint32_t EraseCounter = 0x00;
uint32_t Address = 0x00;
uint32_t Data = 0x3210ABCD;
uint32_t NbrOfPage = 0x00;


void flash_write(uint32_t addr, uint32_t value)
{
	/* Unlock the Flash to enable the flash control register access *************/
	FLASH_Unlock();

	/* Erase the user Flash area
	(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

	/* Define the number of page to be erased */
	NbrOfPage = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

	/* Erase the FLASH pages */

	if (FLASH_ErasePage(FLASH_USER_START_ADDR)!= FLASH_COMPLETE)
	{
	 /* Error occurred while sector erase.
		 User can add here some code to deal with this error  */
	  while (1)
	  {
	  }
	}
	/* Program the user Flash area word by word
	(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	Address = FLASH_USER_START_ADDR;

	if (FLASH_ProgramWord(Address, value) != FLASH_COMPLETE)
	{
	  while (1)
	  {
	  }
	}

	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) *********/
	FLASH_Lock();
}

uint32_t flash_read(uint32_t addr)

{	Address = FLASH_USER_START_ADDR + addr;
	Data = *(uint32_t*)Address;
	return (Data);
}
