#ifndef __usart_H
#define __usart_H


#include "main.h"

extern UART_HandleTypeDef huart2;

void MX_USART2_UART_Init(void);
void UARTLoggerSendBuff(uint8_t *pData, uint32_t nLenght);
void HAL_UART_PutStr(uint8_t *pData, uint32_t nLenght);


#endif /*__ usart_H */

