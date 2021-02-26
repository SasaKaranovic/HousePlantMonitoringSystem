#ifndef __MAIN_H
#define __MAIN_H

#include "stm32l0xx_hal.h"
#include <stdbool.h>

void Error_Handler(void);

#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_15
#define VCP_RX_GPIO_Port GPIOA
#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOB
#define TEMP_ALERT_Pin GPIO_PIN_6
#define TEMP_ALERT_GPIO_Port GPIOB



#endif
