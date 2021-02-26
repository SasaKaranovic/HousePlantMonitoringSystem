#ifndef __STM32_SW_I2C_H
#define __STM32_SW_I2C_H

#include <stdbool.h>
#include "main.h"
#include "dwt_stm32_delay.h"


#define SW_I2C_SCL_GPIO_Port 	GPIOB
#define SW_I2C_SCL_Pin			GPIO_PIN_4
#define SW_I2C_SDA_GPIO_Port 	GPIOB
#define SW_I2C_SDA_Pin			GPIO_PIN_5



#define I2C_CLEAR_SDA HAL_GPIO_WritePin(SW_I2C_SDA_GPIO_Port, SW_I2C_SDA_Pin, GPIO_PIN_RESET);
#define I2C_SET_SDA HAL_GPIO_WritePin(SW_I2C_SDA_GPIO_Port, SW_I2C_SDA_Pin, GPIO_PIN_SET);
//#define I2C_READ_SDA {if (HAL_GPIO_ReadPin(SW_I2C_SDA_GPIO_Port, SW_I2C_SDA_Pin)) == GPIO_PIN_SET) return 1; else return 0; return 0;};
#define I2C_CLEAR_SCL HAL_GPIO_WritePin(SW_I2C_SCL_GPIO_Port, SW_I2C_SCL_Pin, GPIO_PIN_RESET);
#define I2C_SET_SCL HAL_GPIO_WritePin(SW_I2C_SCL_GPIO_Port, SW_I2C_SCL_Pin, GPIO_PIN_SET);
#define I2C_DELAY DWT_Delay_us(5); // 5 microsecond delay

//void I2C_bus_init(uint8_t scl_pin, uint8_t sda_pin, uint8_t port);

void I2C_init(void);
bool I2C_TXBuffer(uint8_t address, uint8_t *pData, uint8_t nLen);
bool I2C_RXBuffer(uint8_t address, uint8_t *pData, uint8_t nLen);


#endif /*__STM32_SW_I2C_H */
