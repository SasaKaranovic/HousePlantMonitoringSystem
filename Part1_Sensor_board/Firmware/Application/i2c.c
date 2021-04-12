#include "main.h"
#include "i2c.h"
#include "usart.h"


#define MIN_LOG_LEVEL_INFO
#define LOGGER_TAG "I2C"
#include "logger.h"


#define I2C_ADDRESS        	(0x1F << 1)
#define I2C_TIMING    		0x20602938 /* 100 kHz with analog Filter ON, Rise Time 400ns, Fall Time 100ns */ 
#define I2C_BUFF_LEN  		30


volatile uint8_t pI2CBuff[I2C_BUFF_LEN] = {0};
volatile uint8_t nBuffPos = 0;

I2C_HandleTypeDef hi2c1;


/* I2C1 init function */
void MX_I2C1_Init(void)
{
  /*##-1- Configure the I2C peripheral ######################################*/
  hi2c1.Instance             = I2Cx;
  hi2c1.Init.Timing          = I2C_TIMING;
  hi2c1.Init.OwnAddress1     = I2C_ADDRESS;
  hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.OwnAddress2     = 0xFF;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_ENABLE;  

  if(HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Enable the Analog I2C Filter */
  HAL_I2CEx_ConfigAnalogFilter(&hi2c1,I2C_ANALOGFILTER_ENABLE);

}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;
  
  /*##-1- Configure the I2C clock source. The clock is derived from the SYSCLK #*/
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2Cx;
  RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2CxCLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

  /*##-2- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  I2Cx_CLK_ENABLE(); 

  /*##-3- Configure peripheral GPIO ##########################################*/  
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
    
  /*##-4- Configure the NVIC for I2C ########################################*/   
  /* NVIC for I2Cx */
  HAL_NVIC_SetPriority(I2Cx_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(I2Cx_IRQn);


}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

	if(i2cHandle->Instance==I2C1)
	{
		I2Cx_FORCE_RESET();
		I2Cx_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks #################################*/
		/* Configure I2C Tx as alternate function  */
		HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
		/* Configure I2C Rx as alternate function  */
		HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);

		/*##-3- Disable the NVIC for I2C ##########################################*/
		HAL_NVIC_DisableIRQ(I2Cx_IRQn);
	}
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *pI2c_Handle)
{
  /** Error_Handler() function is called when error occurs.
    * 1- When Slave don't acknowledge it's address, Master restarts communication.
    * 2- When Master don't acknowledge the last data transferred, Slave don't care in this example.
    */
  if (HAL_I2C_GetError(pI2c_Handle) != HAL_I2C_ERROR_AF)
  {
    // Error_Handler();
  }
}


void I2C_GetBuffer(uint8_t **ret_pBuff, uint8_t *pBufferLen)
{
	if(ret_pBuff == NULL || pBufferLen == NULL)
	{
		Logger_ERROR("Invalid argument!");
		return;
	}
	
	*ret_pBuff = (uint8_t *)pI2CBuff;
	*pBufferLen = I2C_BUFF_LEN;
}

void I2C_ClearBuffer(void)
{
	for(uint8_t i=0; i<I2C_BUFF_LEN; i++)
	{
		pI2CBuff[i] = 0;
	}
}


void I2C_RestartReceive(void)
{
	HAL_I2C_Slave_Receive_IT(&hi2c1, (uint8_t*)pI2CBuff, I2C_BUFF_LEN);
}

bool I2C_DeviceInReadyState(void)
{
	return (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_READY);
}

bool I2C_SendBuffer(uint8_t *pTXBuff, uint8_t nTXBufferLen)
{
	if(pTXBuff == NULL)
	{
		Logger_ERROR("Invalid pointer!");
		return false;
	}

	if (nTXBufferLen >= I2C_BUFF_LEN)
	{
		Logger_ERROR("nTXBufferLen too big");
		return false;
	}

	if(HAL_I2C_Slave_Transmit(&hi2c1, pTXBuff, nTXBufferLen, 0xFFFFF) != HAL_OK)
	{
		return false;
	}
	return true;
}

