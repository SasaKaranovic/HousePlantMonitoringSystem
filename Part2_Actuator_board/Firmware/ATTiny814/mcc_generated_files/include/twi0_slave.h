/**
  @Company
    Microchip Technology Inc.

  @Description
    This Source file provides APIs.
    Generation Information :
    Driver Version    :   1.0.0
*/
/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/


#ifndef TWI0_SLAVE_H
#define TWI0_SLAVE_H

#include <stdbool.h>
#include <stdint.h>
#include "../utils/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(TWI0_callback)(void);

/**
 * \brief Initialize I2C interface
 * If module is configured to disabled state, the clock to the I2C is disabled
 * if this is supported by the device's clock system.
 *
 * \return Initialization status.
 * \retval 0 the init was successful
 * \retval 1 the init was not successful
 */
uint8_t I2C_Initialize(void);

/**
 * \brief Open the I2C for communication. Enables the module if disabled.
 *
 * \return Nothing
 */
void I2C_Open(void);

/**
 * \brief Close the I2C for communication. Disables the module if enabled.
 * Disables address recognition.
 *
 * \return Nothing
 */
void I2C_Close(void);

/**
 * \brief The function called by the I2C IRQ handler.
 * Can be called in a polling loop in a polled driver.
 *
 * \return Nothing
 */
void I2C_isr(void);

/**
 * \brief Read one byte from the data register of I2C
 *
 * Function will not block if a character is not available, so should
 * only be called when data is available.
 *
 * \return Data read from the I2C module
 */
uint8_t I2C_Read(void);

/**
 * \brief Write one byte to the data register of I2C
 *
 * Function will not block if data cannot be safely accepted, so should
 * only be called when safe, i.e. in the read callback handler.
 *
 * \param[in] data The character to write to the I2C
 *
 * \return Nothing
 */
void I2C_Write(uint8_t data);

/**
 * \brief Enable address recognition in I2C
 * 1. If supported by the clock system, enables the clock to the module
 * 2. Enables the I2C slave functionality  by setting the enable-bit in the HW's control register
 *
 * \return Nothing
 */
void I2C_Enable(void);

/**
 * \brief Send ACK to received address or data. Should
 * only be called when appropriate, i.e. in the callback handlers.
 *
 * \return Nothing
 */
void I2C_SendAck(void);

/**
 * \brief Send NACK to received address or data. Should
 * only be called when appropriate, i.e. in the callback handlers.
 *
 * \return Nothing
 */
void I2C_SendNack(void);

/**
 * \brief Goto unaddressed state. Used to reset I2C HW that are aware
 * of bus state to an unaddressed state.
 *
 * \return Nothing
 */
void I2C_GotoUnaddressed(void);

/**
 * \brief Callback handler for event where master wishes to read a byte from slave.
 *
 * \return Nothing
 */
void I2C_SetReadCallback(TWI0_callback handler);

/**
 * \brief Callback handler for event where master wishes to write a byte to slave.
 *
 * \return Nothing
 */
void I2C_SetWriteCallback(TWI0_callback handler);

/**
 * \brief Callback handler for event where slave has received its address.
 *
 * \return Nothing
 */
void I2C_SetAddressCallback(TWI0_callback handler);

/**
 * \brief Callback handler for event where slave has received a STOP condition after being addressed.
 *
 * \return Nothing
 */
void I2C_SetStopCallback(TWI0_callback handler);

/**
 * \brief Callback handler for event where slave detects a bus collision.
 *
 * \return Nothing
 */
void I2C_SetCollisionCallback(TWI0_callback handler);

/**
 * \brief Callback handler for event where slave detects a bus error.
 *
 * \return Nothing
 */
void I2C_SetBusErrorCallback(TWI0_callback handler);


void I2C_SetAddress(uint8_t address);


#ifdef __cplusplus
}
#endif

#endif /* TWI0_SLAVE_H */