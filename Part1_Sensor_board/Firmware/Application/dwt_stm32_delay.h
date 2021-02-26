#ifndef DWT_STM32_DELAY_H
#define DWT_STM32_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32l0xx_hal.h"
uint32_t DWT_Delay_Init(void);



/**
 * @brief  This function provides a delay (in microseconds)
 * @param  microseconds: delay in microseconds
 */
__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)
{
	#if 0
	uint32_t clk_cycle_start = DWT->CYCCNT;

	/* Go to number of cycles for system */
	microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);
	HAL_Delay();

	/* Delay till end */
	while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
	#endif
	#if 0
	HAL_Delay(1);
	#endif
	#if 1
	microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);
	
	while(microseconds--)
	{
	    __ASM volatile ("NOP");
	    __ASM volatile ("NOP");
	}
	#endif
}


#ifdef __cplusplus
}
#endif

#endif
