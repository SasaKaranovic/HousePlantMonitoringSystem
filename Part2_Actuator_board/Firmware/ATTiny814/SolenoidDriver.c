#include "SolenoidDriver.h"
#include "PlantSystem.h"
#include "debug.h"
#include "mcc_generated_files/include/twi0_slave.h"

// -- Global definitions

// -- Extern variables
extern uint8_t TWI_TxBuff[TWI_BUFFER_SIZE];
extern uint8_t TWI_TxLen;
extern uint8_t TWI_TxPos;

extern uint8_t TWI_RxBuff[TWI_BUFFER_SIZE];    // Device RX from host
extern uint8_t TWI_RxPos;

// Solenoid turn-off timer
extern volatile uint32_t milliseconds;
extern volatile uint32_t solenoidTurnOff_target;
extern volatile bool bWDTRunning;

// CMD (1byte) + DATATYPE (1byte) + DATALEN (2bytes) 
void SolenoidDriver_ProcessRequest(uint8_t *pRxBuff, uint8_t nLen)
{
#if 0
  debugStr("pRxBuff:\r\n");
  for(uint8_t i=0; i<nLen; i++)
  {
      debugHex(pRxBuff[i]);
  }
#endif

    // First byte is always command
    switch(pRxBuff[0])
    {
        // -- Get device information
        case CMD_GET_DEVICE_INFO:
        {
            debugStr("\r\nInfo\r\n");
            TWI_TxBuff[0]  = 0xFF;                  // First byte is 0xFF to allow compatibility with STM and disable clock strech
            TWI_TxBuff[1]  = CMD_GET_DEVICE_INFO;   // Second byte is response to command
            TWI_TxBuff[2]  = DATATYPE_SINGLE_VALUE;
            TWI_TxBuff[3]  = 0;
            TWI_TxBuff[4]  = 2;
            TWI_TxBuff[5]  = DEVICETYPE_SOLENOID_V1;
            TWI_TxBuff[6]  = 1;
            TWI_TxLen = 7;
        }
        break;

        // -- SET solenoid state
        case CMD_SET_SOLENOID_STATE:
        {
            debugStr("\r\nSET State: ");

            // Data Type `DATATYPE_SINGLE_VALUE` and exactly 0 + 1 = 1 bytes
            if(pRxBuff[1] == DATATYPE_SINGLE_VALUE && pRxBuff[2] == 0 && pRxBuff[3] == 1)
            {
                debugHex(pRxBuff[1]);

                if(pRxBuff[4] == 0)
                {
                    debugStr("Turn OFF\r\n");
                    Solenoid_SET_Off();
                    Solenoid_ActLED_Off();
                }
                else
                {
                    debugStr("Turn ON\r\n");
                    Solenoid_SET_On();
                    Solenoid_ActLED_On();
                    SolenoidWDT_Activate();
                }
            }
            else
            {
                debugStr("Bad Data\r\n");
            }

        }
        break;

        // -- GET solenoid state
        case CMD_GET_SOLENOID_STATE:
        {
            debugStr("\r\nGET state\r\n");  

            TWI_TxBuff[0]  = 0xFF;                     // First byte is 0xFF to allow compatibility with STM and disable clock strech
            TWI_TxBuff[1]  = CMD_GET_SOLENOID_STATE;   // Second byte is response to command
            TWI_TxBuff[2]  = DATATYPE_SINGLE_VALUE;
            TWI_TxBuff[3]  = 0;
            TWI_TxBuff[4]  = 1;
            TWI_TxBuff[5]  = Solenoid_GetState();
            TWI_TxLen = 6;
        }
        break;

        // Catch-all
        default:
            debugStr("Uknown CMD received\r\n");
        break;
    }
}


void SolenoidWDT_Activate(void)
{
    solenoidTurnOff_target = milliseconds + SOLENOID_ON_TIMEOUT;
    bWDTRunning = true;
}


