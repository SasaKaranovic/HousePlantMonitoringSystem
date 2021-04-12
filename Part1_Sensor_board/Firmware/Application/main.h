#ifndef __MAIN_H
#define __MAIN_H

#include "stm32l0xx_hal.h"
#include <stdbool.h>

void Error_Handler(void);

#define VCP_TX_Pin 						GPIO_PIN_2
#define VCP_TX_GPIO_Port 				GPIOA
#define TMS_Pin 						GPIO_PIN_13
#define TMS_GPIO_Port 					GPIOA
#define TCK_Pin 						GPIO_PIN_14
#define TCK_GPIO_Port 					GPIOA
#define VCP_RX_Pin 						GPIO_PIN_15
#define VCP_RX_GPIO_Port 				GPIOA
#define LD3_Pin 						GPIO_PIN_3
#define LD3_GPIO_Port 					GPIOB
#define TEMP_ALERT_Pin 					GPIO_PIN_6
#define TEMP_ALERT_GPIO_Port 			GPIOB


/* Definition for I2Cx */
#define I2Cx                            I2C1
#define RCC_PERIPHCLK_I2Cx              RCC_PERIPHCLK_I2C1
#define RCC_I2CxCLKSOURCE_SYSCLK        RCC_I2C1CLKSOURCE_SYSCLK
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_9
#define I2Cx_SCL_GPIO_PORT              GPIOA
#define I2Cx_SDA_PIN                    GPIO_PIN_10
#define I2Cx_SDA_GPIO_PORT              GPIOA
#define I2Cx_SCL_SDA_AF                 GPIO_AF1_I2C1

/* Definition for I2Cx's NVIC */
#define I2Cx_IRQn                       I2C1_IRQn
#define I2Cx_IRQHandler                 I2C1_IRQHandler


#endif
