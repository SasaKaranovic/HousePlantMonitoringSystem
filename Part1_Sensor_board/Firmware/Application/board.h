#ifndef __BOARD_H
#define __BOARD_H

#include "main.h"


void BoardInit(void);
void SystemClock_Config(void);
void Error_Handler(void);

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#endif

#endif