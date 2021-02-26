#ifndef __i2c_H
#define __i2c_H

#include "main.h"

extern I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void);
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);
void I2C_GetBuffer(uint8_t **ret_pBuff, uint8_t *pBufferLen);
uint8_t I2C_GetBufferLenght(void);
void I2C_ReloadIT(void);
void I2C_ClearBuffer(void);
void I2C_RestartITReceive(void);

#endif /*__ i2c_H */

