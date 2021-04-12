#ifndef __i2c_H
#define __i2c_H

#include "main.h"

extern I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void);
void I2C_GetBuffer(uint8_t **ret_pBuff, uint8_t *pBufferLen);
void I2C_ClearBuffer(void);
void I2C_RestartReceive(void);
bool I2C_DeviceInReadyState(void);
bool I2C_SendBuffer(uint8_t *pTXBuff, uint8_t nTXBufferLen);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle);


#endif /*__ i2c_H */

