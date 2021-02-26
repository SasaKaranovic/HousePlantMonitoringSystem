#include "main.h"
#include "stm32l0xx_it.h"
#include "usart.h"
#include "i2c.h"

extern I2C_HandleTypeDef hi2c1;


/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles I2C1 event global interrupt / I2C1 wake-up interrupt through EXTI line 23.
  */
void I2C1_IRQHandler(void)
{
  if (hi2c1.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR)) {
    HAL_I2C_ER_IRQHandler(&hi2c1);

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
  else {
    HAL_I2C_EV_IRQHandler(&hi2c1);
    // I2C_ReloadIT();
  }

  HAL_NVIC_ClearPendingIRQ(I2C1_IRQn);


}


