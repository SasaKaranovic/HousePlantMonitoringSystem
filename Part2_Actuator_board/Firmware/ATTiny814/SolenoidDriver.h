#ifndef SOLENOID_DRIVER_H
#define	SOLENOID_DRIVER_H

#include <stdbool.h>
#include "mcc_generated_files/utils/compiler.h"
#include "mcc_generated_files/include/port.h"

#define TWI_BUFFER_SIZE	40
#define DEVICE_I2C_ADDRESS	(0x20 << 1)

#define Solenoid_ActLED_On() (PORTB_set_pin_level(1, true))
#define Solenoid_ActLED_Off() (PORTB_set_pin_level(1, false))


#define Solenoid_GetState() (PORTA_get_pin_level(3))
#define Solenoid_SET_On() (PORTA_set_pin_level(3, true))
#define Solenoid_SET_Off() (PORTA_set_pin_level(3, false))


#define SOLENOID_ON_TIMEOUT                 9000
void SolenoidWDT_Activate(void);

void SolenoidDriver_ProcessRequest(uint8_t *pRxBuff, uint8_t nLen);

#endif	/* SOLENOID_DRIVER_H */

