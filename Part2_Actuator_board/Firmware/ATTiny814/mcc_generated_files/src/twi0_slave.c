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


#include "../include/twi0_slave.h"
#include "../../mcc_generated_files/utils/atomic.h"
#include "../../debug.h"
#include "../../SolenoidDriver.h"
#include <stdbool.h>

/* I2C Internal API's */
/* Slave */
bool I2C_SlaveIsAddressInterrupt(void);
bool I2C_SlaveIsDataInterrupt(void);
bool I2C_SlaveIsStopInterrupt(void);
void I2C_SlaveOpen(void);
void I2C_SlaveClose(void);
bool I2C_SlaveDIR(void);
char I2C_SlaveReset(void);
uint8_t I2C_SlaveRead(void);
char I2C_SlaveWrite(uint8_t data);
bool I2C_SlaveIsNack(void);
void I2C_SlaveSendAck(void);
void I2C_SlaveSendNack(void);
bool I2C_SlaveIsBusCollision(void);
bool I2C_SlaveIsBusError(void);
bool I2C_SlaveIsTxComplete(void);

// Read Event Interrupt Handlers
void I2C_ReadCallback(void);
void (*I2C_ReadInterruptHandler)(void);

// Write Event Interrupt Handlers
void I2C_WriteCallback(void);
void (*I2C_WriteInterruptHandler)(void);

// Address Event Interrupt Handlers
void I2C_AddressCallback(void);
void (*I2C_AddressInterruptHandler)(void);

// Stop Event Interrupt Handlers
void I2C_StopCallback(void);
void (*I2C_StopInterruptHandler)(void);

// Bus Collision Event Interrupt Handlers
void I2C_CollisionCallback(void);
void (*I2C_CollisionInterruptHandler)(void);

// Bus Error Event Interrupt Handlers
void I2C_BusErrorCallback(void);
void (*I2C_BusErrorInterruptHandler)(void);

// I2C buffers and positional variables
bool processPending = false;
uint8_t TWI_TxBuff[TWI_BUFFER_SIZE] = {0};    // Device TX to host
uint8_t TWI_TxPos = 0;
uint8_t TWI_TxLen = 0;
uint8_t TWI_RxBuff[TWI_BUFFER_SIZE] = {0};    // Device RX from host
uint8_t TWI_RxPos = 0;

// Function prototypes
static void I2C_AddressFn(void);
static void I2C_WriteFn(void);
static void I2C_ReadFn(void);
static void I2C_StopFn(void);
static void I2C_CollisionFn(void);
static void TWI_ISR_Handler(void);

uint8_t I2C_Initialize()
{
    //SDASETUP 4CYC; SDAHOLD OFF; FMPEN disabled; 
    TWI0.CTRLA = 0x00;
    
    //Debug Run
    TWI0.DBGCTRL = 0x00;
    
    //Slave Address
    TWI0.SADDR = DEVICE_I2C_ADDRESS;
    
    //ADDRMASK 0; ADDREN disabled; 
    TWI0.SADDRMASK = 0x00;    
//    DIEN enabled; APIEN enabled; PIEN disabled; PMEN disabled; SMEN disabled; ENABLE enabled; 
    TWI0.SCTRLA = 0xE1;
    
    //ACKACT ACK; SCMD NOACT; 
    TWI0.SCTRLB = 0x00;
    
    //Slave Data
    TWI0.SDATA = 0x00;
    
    //DIF disabled; APIF disabled; COLL disabled; BUSERR disabled; 
    TWI0.SSTATUS = 0x00;
    
//     I2C_SetWriteCallback(NULL);
//     I2C_SetReadCallback(NULL);
//     I2C_SetAddressCallback(NULL);
//     I2C_SetStopCallback(NULL);
//     I2C_SetCollisionCallback(NULL);
//     I2C_SetBusErrorCallback(NULL);
    I2C_SetWriteCallback(I2C_WriteFn);
    I2C_SetReadCallback(I2C_ReadFn);
    I2C_SetAddressCallback(I2C_AddressFn);
    I2C_SetStopCallback(I2C_StopFn);
    I2C_SetCollisionCallback(I2C_CollisionFn);
    I2C_SetBusErrorCallback(I2C_CollisionFn);
  
    //TWI0.SCTRLA = 0xC1;
    return 0;
}

static void I2C_AddressFn(void)
{
}

static void I2C_WriteFn(void)
{
    // Reset TX/RX buffer positions
    TWI_RxBuff[TWI_RxPos] = I2C_SlaveRead();
    TWI_RxPos++;
    if(TWI_RxPos >= TWI_BUFFER_SIZE)
    {
        TWI_RxPos = TWI_BUFFER_SIZE-1;
    }
}

static void I2C_ReadFn(void)
{
    I2C_SlaveWrite(TWI_TxBuff[TWI_TxPos]);
    TWI_TxPos++;
    if( TWI_TxPos >=  TWI_TxLen)
    {
        TWI_TxPos = TWI_BUFFER_SIZE-1;
    }
    else if(TWI_TxPos >= TWI_BUFFER_SIZE)
    {
        TWI_TxPos = TWI_TxLen-1;
    }
}

static void I2C_StopFn(void)
{
    if (processPending == true)
    {
        SolenoidDriver_ProcessRequest(TWI_RxBuff, TWI_RxPos);
        processPending = false;
    }
    
    // Reset TX/RX buffer positions
    TWI_TxPos = 0;
    TWI_RxPos = 0;
}

static void I2C_CollisionFn(void)
{
    I2C_GotoUnaddressed();
}

void I2C_SetAddress(uint8_t address)
{
    // Always enable general call
    address = address | 0x01;
    TWI0.SADDR = address;
}

void I2C_SetMask(uint8_t mask)
{
    return;
}

void I2C_Open(void)
{
    I2C_SlaveOpen();
}

void I2C_Close(void)
{
    I2C_SlaveClose();
}


static void TWI_ISR_Handler(void)
{
    if (I2C_SlaveIsBusCollision()) {
        I2C_CollisionFn();
        return;
    }

    if (I2C_SlaveIsBusError()) {
        I2C_CollisionFn();
        return;
    }

    if (I2C_SlaveIsAddressInterrupt()) {
        I2C_AddressFn();
        debugTWIStr("\r\nA");
        if (I2C_SlaveDIR()) {
            // Master wishes to read from slave
//            I2C_ReadFn();
            I2C_SlaveSendAck();
        }
        return;
    }
    if (I2C_SlaveIsDataInterrupt()) {
        if (I2C_SlaveDIR()) 
        {
            // Master wishes to read from slave
//           if (!I2C_SlaveIsNack()) {
                debugTWIStr("R");
                // Received ACK from master
                I2C_ReadFn();
                I2C_SlaveSendAck();
//           } else {
                // Received NACK from master
//               debugTWIStr("RN");
//               I2C_SlaveSendAck();
//               I2C_GotoUnaddressed();
//           }
        } 
        else // Master wishes to write to slave
        {
            debugTWIStr("W");
            I2C_WriteFn();
            processPending = true;
        }
        return;
    }

    // Check if STOP was received
    if (I2C_SlaveIsStopInterrupt()) {
        debugTWIStr("S\r\n");
        I2C_StopFn();
        I2C_SlaveIsTxComplete(); // To check the status of the transaction
        return;
    }
}


ISR(TWI0_TWIS_vect)
{
    TWI_ISR_Handler();
    
    // Reset Interrupt flags
    TWI0.SSTATUS |= (TWI_DIF_bm | TWI_APIF_bm);
}


uint8_t I2C_Read(void)
{
    return I2C_SlaveRead();
}

void I2C_Write(uint8_t data)
{
    I2C_SlaveWrite(data);
}

void I2C_Enable(void)
{
    I2C_SlaveOpen();
}

void I2C_SendAck(void)
{
    I2C_SlaveSendAck();
}

void I2C_SendNack(void)
{
    I2C_SlaveSendNack();
}

void I2C_GotoUnaddressed(void)
{
    // Reset module
    I2C_SlaveReset();
}

// Read Event Interrupt Handlers
void I2C_ReadCallback(void)
{
    if (I2C_ReadInterruptHandler) {
        I2C_ReadInterruptHandler();
    }
}

void I2C_SetReadCallback(TWI0_callback handler)
{
    I2C_ReadInterruptHandler = handler;
}

// Write Event Interrupt Handlers
void I2C_WriteCallback(void)
{
    if (I2C_WriteInterruptHandler) {
        I2C_WriteInterruptHandler();
    }
}

void I2C_SetWriteCallback(TWI0_callback handler)
{
    I2C_WriteInterruptHandler = handler;
}

// Address Event Interrupt Handlers
void I2C_AddressCallback(void)
{
    if (I2C_AddressInterruptHandler) {
        I2C_AddressInterruptHandler();
    }
}

void I2C_SetAddressCallback(TWI0_callback handler)
{
    I2C_AddressInterruptHandler = handler;
}

// Stop Event Interrupt Handlers
void I2C_StopCallback(void)
{
    if (I2C_StopInterruptHandler) {
        I2C_StopInterruptHandler();
    }
}

void I2C_SetStopCallback(TWI0_callback handler)
{
    I2C_StopInterruptHandler = handler;
}

// Bus Collision Event Interrupt Handlers
void I2C_CollisionCallback(void)
{
    if (I2C_CollisionInterruptHandler) {
        I2C_CollisionInterruptHandler();
    }
}

void I2C_SetCollisionCallback(TWI0_callback handler)
{
    I2C_CollisionInterruptHandler = handler;
}

// Bus Error Event Interrupt Handlers
void I2C_BusErrorCallback(void)
{
    if (I2C_BusErrorInterruptHandler) {
        I2C_BusErrorInterruptHandler();
    }
}

void I2C_SetBusErrorCallback(TWI0_callback handler)
{
    I2C_BusErrorInterruptHandler = handler;
}


/* Slave Configurations */
void I2C_SlaveOpen(void)
{
    TWI0.SCTRLA |= TWI_ENABLE_bm;
}

void I2C_SlaveClose(void)
{
    TWI0.SCTRLA &= ~TWI_ENABLE_bm;
}

bool I2C_SlaveIsBusCollision(void)
{
    return TWI0.SSTATUS & TWI_COLL_bm;
}

bool I2C_SlaveIsBusError(void)
{
    return TWI0.SSTATUS & TWI_BUSERR_bm;
}

bool I2C_SlaveIsAddressInterrupt(void)
{
    return (TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_AP_bm);
}

bool I2C_SlaveIsDataInterrupt(void)
{
    return TWI0.SSTATUS & TWI_DIF_bm;
}

bool I2C_SlaveIsStopInterrupt(void)
{
    return (TWI0.SSTATUS & TWI_APIF_bm) && (!(TWI0.SSTATUS & TWI_AP_bm));
}

bool I2C_SlaveDIR(void)
{
    return TWI0.SSTATUS & TWI_DIR_bm;
}

void I2C_SlaveSendAck(void)
{
    TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;
}

void I2C_SlaveSendNack(void)
{
    TWI0.SCTRLB = TWI_ACKACT_NACK_gc | TWI_SCMD_COMPTRANS_gc;
}

bool I2C_SlaveIsNack(void)
{
    return TWI0.SSTATUS & TWI_RXACK_bm;
}

bool I2C_SlaveIsTxComplete(void)
{
    TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
    return TWI0.SCTRLB;
}

uint8_t I2C_SlaveRead(void)
{
    return TWI0.SDATA;
}

char I2C_SlaveWrite(uint8_t data)
{
    TWI0.SDATA = data;
    TWI0.SCTRLB |= TWI_SCMD_RESPONSE_gc;
    return TWI0.SDATA;
}

char I2C_SlaveReset(void)
{
    TWI0.SSTATUS |= (TWI_DIF_bm | TWI_APIF_bm);
    TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
    return TWI0.SSTATUS;
}