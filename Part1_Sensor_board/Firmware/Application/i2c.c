#include "i2c.h"
#include "usart.h"


#define MIN_LOG_LEVEL_INFO
#define LOGGER_TAG "I2C"
#include "logger.h"

/* USER CODE BEGIN 0 */
#define I2C_BUFF_LEN  30
volatile uint8_t pI2CBuff[I2C_BUFF_LEN] = {0};
volatile uint8_t nBuffPos = 0;

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00707CBB;
	hi2c1.Init.OwnAddress1 = 0x32;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	// hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_ENABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	// hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
	
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Analogue filter
	*/
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure Digital filter
	*/
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
	{
		Error_Handler();
	}

	while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
	HAL_StatusTypeDef status;
	if( (status=HAL_I2C_Slave_Receive_IT(&hi2c1, pI2CBuff, 4)) != HAL_OK)
	{
		while(1);
	}


	/* I2C1 interrupt Init */
	HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(I2C1_IRQn);


}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(i2cHandle->Instance==I2C1)
	{
	/* USER CODE BEGIN I2C1_MspInit 0 */

	/* USER CODE END I2C1_MspInit 0 */

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__DSB();

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9,  GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
		/**I2C1 GPIO Configuration
		PA9     ------> I2C1_SCL
		PA10     ------> I2C1_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* I2C1 clock enable */
		__HAL_RCC_I2C1_CLK_ENABLE();
	}
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

	if(i2cHandle->Instance==I2C1)
	{
		/* Peripheral clock disable */
		__HAL_RCC_I2C1_CLK_DISABLE();

		/**I2C1 GPIO Configuration
		PA9     ------> I2C1_SCL
		PA10     ------> I2C1_SDA
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);

	}
}


void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t msg[] = "Tx\r\n";
	HAL_UART_PutStr(msg, sizeof(msg));
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t msg[] = "Rx\r\n";
	HAL_UART_PutStr(msg, sizeof(msg));
	nBuffPos = 4;

    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_TXE);     // Transmit data register empty
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ADDR);    // Address matched (slave mode)
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_AF);      // Acknowledge failure received flag
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_STOPF);   // STOP detection flag
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_BERR);    // Bus error
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ARLO);    // Arbitration lost
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_OVR);     // Overrun/Underrun
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_PECERR);  // PEC error in reception
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_TIMEOUT); // Timeout or Tlow detection flag
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ALERT);   // SMBus alert
}

void I2C_GetBuffer(uint8_t **ret_pBuff, uint8_t *pBufferLen)
{
	if(ret_pBuff == NULL || pBufferLen == NULL)
	{
		return;
	}
	
	*ret_pBuff = (uint8_t *)pI2CBuff;
	*pBufferLen = nBuffPos;
}

uint8_t I2C_GetBufferLenght(void)
{
	return nBuffPos;
}

void I2C_ClearBuffer(void)
{
	nBuffPos = 0;
}

void I2C_ReloadIT(void)
{
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_TXE);     // Transmit data register empty
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ADDR);    // Address matched (slave mode)
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_AF);      // Acknowledge failure received flag
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_STOPF);   // STOP detection flag
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_BERR);    // Bus error
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ARLO);    // Arbitration lost
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_OVR);     // Overrun/Underrun
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_PECERR);  // PEC error in reception
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_TIMEOUT); // Timeout or Tlow detection flag
    __HAL_I2C_CLEAR_FLAG(&hi2c1, I2C_FLAG_ALERT);   // SMBus alert
	HAL_I2C_Slave_Receive_IT(&hi2c1, pI2CBuff, 4);  //Enable slave interrupt reception
	// nBuffPos++;
	// if (nBuffPos >= I2C_BUFF_LEN-1)
	// {
	// 	nBuffPos = 0;
	// }
}

void I2C_RestartITReceive(void)
{
	// HAL_I2C_DisableListen_IT(&hi2c1);

	// __HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_ERRI);  // Errors interrupt enable
	// __HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_TCI);   // Transfer complete interrupt enable
	// __HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_STOPI); // STOP detection interrupt enable
	// __HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_NACKI); // NACK received interrupt enable
	// __HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_ADDRI); // Address match interrupt enable
	// __HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_RXI);   // RX interrupt enable
	// __HAL_I2C_ENABLE_IT(&hi2c1, I2C_IT_TXI);   // TX interrupt enable

	// HAL_I2C_Slave_Seq_Receive_IT(&hi2c1, (uint8_t *)&pI2CBuff[nBuffPos], 1, I2C_LAST_FRAME_NO_STOP);  //Enable slave interrupt reception
	// I2C_ReloadIT();
	// HAL_I2C_EnableListen_IT(&hi2c1);
}


