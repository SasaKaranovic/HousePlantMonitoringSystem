#include "main.h"
#include "dwt_stm32_delay.h"
#include "stm32_sw_i2c.h"

//void I2C_bus_init(uint8_t scl_pin, uint8_t sda_pin, uint8_t port){
//	  /*Configure GPIO pins : SW_I2C_SCL_Pin SW_I2C_SDA_Pin */
//	  GPIO_InitStruct.Pin = SW_I2C_SCL_Pin|SW_I2C_SDA_Pin;
//	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//	  GPIO_InitStruct.Pull = GPIO_PULLUP;
//	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//}


#define true 1
#define false 0

static bool read_SCL(void);
static bool read_SDA(void);
static void arbitration_lost(void);
static bool i2c_write_byte(bool send_start, bool send_stop, unsigned char byte);
static unsigned char i2c_read_byte(bool nack, bool send_stop);

void I2C_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

   HAL_GPIO_WritePin(SW_I2C_SCL_GPIO_Port, SW_I2C_SCL_Pin, GPIO_PIN_SET);
   HAL_GPIO_WritePin(SW_I2C_SDA_GPIO_Port, SW_I2C_SDA_Pin, GPIO_PIN_SET);

   /*Configure GPIO pins : SW_I2C_SCL_Pin SW_I2C_SDA_Pin */
   GPIO_InitStruct.Pin = SW_I2C_SCL_Pin|SW_I2C_SDA_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


    I2C_SET_SDA;
    I2C_SET_SCL;
}


static bool read_SCL(void)
{
    if(HAL_GPIO_ReadPin(SW_I2C_SCL_GPIO_Port, SW_I2C_SCL_Pin) == GPIO_PIN_SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static bool read_SDA(void)
{
    if(HAL_GPIO_ReadPin(SW_I2C_SDA_GPIO_Port, SW_I2C_SDA_Pin) == GPIO_PIN_SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}



static void arbitration_lost(void)
{
  return;
}

bool started = false; // global data

void i2c_start_cond(void) {
  if (started) { 
    // if started, do a restart condition
    // set SDA to 1
    I2C_SET_SDA
    I2C_DELAY
    I2C_SET_SCL
    while (read_SCL() == 0) { // Clock stretching
      // You should add timeout to this loop
    }

    // Repeated start setup time, minimum 4.7us
    I2C_DELAY
  }

  if (read_SDA() == 0) {
    arbitration_lost();
  }

  // SCL is high, set SDA from 1 to 0.
  I2C_CLEAR_SDA
  I2C_DELAY
  I2C_CLEAR_SCL
  started = true;
}

void i2c_stop_cond(void) {
  // set SDA to 0
  I2C_CLEAR_SDA
  I2C_DELAY

  I2C_SET_SCL
  // Clock stretching
  while (read_SCL() == 0) {
    // add timeout to this loop.
  }

  // Stop bit setup time, minimum 4us
  I2C_DELAY

  // SCL is high, set SDA from 0 to 1
  I2C_SET_SDA
  I2C_DELAY

  if (read_SDA() == 0) {
    arbitration_lost();
  }

  started = false;
}

// Write a bit to I2C bus
void i2c_write_bit(bool bit) {
  if (bit) {
    I2C_SET_SDA
  } else {
    I2C_CLEAR_SDA
  }

  // SDA change propagation delay
  I2C_DELAY

  // Set SCL high to indicate a new valid SDA value is available
  I2C_SET_SCL

  // Wait for SDA value to be read by slave, minimum of 4us for standard mode
  I2C_DELAY

  while (read_SCL() == 0) { // Clock stretching
    // You should add timeout to this loop
  }

  // SCL is high, now data is valid
  // If SDA is high, check that nobody else is driving SDA
  if (bit && (read_SDA() == 0)) {
    arbitration_lost();
  }

  // Clear the SCL to low in preparation for next change
  I2C_CLEAR_SCL
}

// Read a bit from I2C bus
bool i2c_read_bit(void) {
  bool bit;

  // Let the slave drive data
  I2C_SET_SDA

  // Wait for SDA value to be written by slave, minimum of 4us for standard mode
  I2C_DELAY

  // Set SCL high to indicate a new valid SDA value is available
  I2C_SET_SCL

  while (read_SCL() == 0) { // Clock stretching
    // You should add timeout to this loop
  }

  // Wait for SDA value to be written by slave, minimum of 4us for standard mode
  I2C_DELAY

  // SCL is high, read out bit
  bit = read_SDA();

  // Set SCL low in preparation for next operation
  I2C_CLEAR_SCL

  return bit;
}

// Write a byte to I2C bus. Return 0 if ack by the slave.
static bool i2c_write_byte(bool send_start, bool send_stop, unsigned char byte)
{
  unsigned bit;
  bool     nack;

  if (send_start) {
    i2c_start_cond();
  }

  for (bit = 0; bit < 8; ++bit) {
    i2c_write_bit((byte & 0x80) != 0);
    byte <<= 1;
  }

  nack = i2c_read_bit();

  if (send_stop) {
    i2c_stop_cond();
  }

  return nack;
}

// Read a byte from I2C bus
static unsigned char i2c_read_byte(bool nack, bool send_stop)
{
  unsigned char byte = 0;
  unsigned char bit;

  for (bit = 0; bit < 8; ++bit) {
    byte = (byte << 1) | i2c_read_bit();
  }

  i2c_write_bit(nack);

  if (send_stop) {
    i2c_stop_cond();
  }

  return byte;
}



////////////
// i2c_write_byte(send_start, send_stop, byte)
// i2c_read_byte(nack, send_stop)

bool I2C_TXBuffer(uint8_t address, uint8_t *pData, uint8_t nLen)
{
    i2c_write_byte(true, false, address);

    for(uint8_t i=0; i<nLen-1; i++)
    {
        i2c_write_byte(false, false, pData[i]);
    }

    i2c_write_byte(false, true, pData[nLen-1]);

    return true;
}



bool I2C_RXBuffer(uint8_t address, uint8_t *pData, uint8_t nLen)
{
     i2c_write_byte(true, false, (address | 0x01));

    for(uint8_t i=0; i<nLen-1; i++)
    {
        pData[i] = i2c_read_byte(false, false);
    }

    pData[nLen-1] = i2c_read_byte(true, true);

    return true;
}


