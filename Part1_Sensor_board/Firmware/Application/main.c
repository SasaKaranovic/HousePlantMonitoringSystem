#include "main.h"
#include "board.h"
#include "i2c.h"
#include "sensors.h"
#include "PlantSystem.h"

#define MIN_LOG_LEVEL_INFO
#define LOGGER_TAG "MAIN"
#include "logger.h"

#define TX_BUFF_SIZE                    10
static uint8_t pTXBuff[TX_BUFF_SIZE]    = {0xFF};
static uint8_t nTXBufferLen             = 0;
uint8_t *pBuff                          = NULL;
uint8_t nBufferLen                      = 0;
uint32_t nTickTarget_PrintDebug         = 0;
uint32_t nTickTarget_TakeMeasurement    = 0;

static void PlantSystem_ProcessRequest(void);

int main(void)
{
    BoardInit();
    HAL_Delay(500);

    Logger_INFO("PlantSensor v0.1");
    Logger_INFO("Build date: %s %s", __DATE__, __TIME__);

    I2C_RestartReceive();

    while (1)
    {
        // Check if I2C is ready for processing 
        if(I2C_DeviceInReadyState() == true)
        {
            I2C_GetBuffer(&pBuff, &nBufferLen);
            // Check valid command is received
            if(pBuff[0] > CMD_FIRST && pBuff[0] < CMD_LAST)
            {
                PlantSystem_ProcessRequest();
            }
            
            // // Clear RX buffer and reset receive 
            I2C_ClearBuffer();
            while (I2C_DeviceInReadyState() != true)
            {
            }
            I2C_RestartReceive();
        }

        // Take soil moisture, temperature and ambient light measurements
        if(HAL_GetTick() >= nTickTarget_TakeMeasurement)
        {
            Sensor_TakeMeasurement();
            nTickTarget_TakeMeasurement = HAL_GetTick() + 500;
        }


        // Print Sensor debug over uart every 1 sec
        if(HAL_GetTick() >= nTickTarget_PrintDebug)
        {
            Sensors_PrintDebug();
            nTickTarget_PrintDebug = HAL_GetTick() + 1000;
        }

    }

}


static void PlantSystem_ProcessRequest(void)
{
    uint8_t cmd=0;
    uint8_t dataType=0;
    uint8_t dataLength=0;

    I2C_GetBuffer(&pBuff, &nBufferLen);

    if (PlanSystem_ParseBuffer(pBuff, nBufferLen, &cmd, &dataType, &dataLength))
    {
        Logger_INFO("Success");

        switch(cmd)
        {
            // Get device information
            case CMD_GET_DEVICE_INFO:
            {
                pTXBuff[0]  = CMD_GET_DEVICE_INFO;
                pTXBuff[1]  = DATATYPE_SINGLE_VALUE;
                pTXBuff[2]  = 0;    // Upper byte of data length
                pTXBuff[3]  = 4;    // Lower byte of data length
                pTXBuff[4]  = DEVICETYPE_SENSOR_V1; 
                pTXBuff[5]  = 'T';  // Optional bytes to indicate capabilities (temp, humid, light)
                pTXBuff[6]  = 'H';  // Optional bytes to indicate capabilities (temp, humid, light)
                pTXBuff[7]  = 'L';  // Optional bytes to indicate capabilities (temp, humid, light)
                nTXBufferLen = 8;
                Logger_INFO("I");
            }
            break;

            // Read temperature
            case CMD_GET_TEMPERATURE:
            {
                pTXBuff[0]  = CMD_GET_TEMPERATURE;
                pTXBuff[1]  = DATATYPE_SINGLE_VALUE;
                pTXBuff[2]  = 2;
                Sensors_GetTemperatureReading(&pTXBuff[3], &pTXBuff[4]);
                nTXBufferLen = 5;
                Logger_INFO("T");
            }
            break;

            // Read soil moisture
            case CMD_GET_SOILMOISTURE:
            {
                pTXBuff[0]  = CMD_GET_SOILMOISTURE;
                pTXBuff[1]  = DATATYPE_SINGLE_VALUE;
                pTXBuff[2]  = 4;
                Sensors_GetSoilMoistureReading(&pTXBuff[3], &pTXBuff[4], &pTXBuff[5], &pTXBuff[6]);
                nTXBufferLen = 7;
                Logger_INFO("M");
            }
            break;

            // Read ambient light
            case CMD_GET_AMBIENTLIGHT:
            {
                pTXBuff[0]  = CMD_GET_AMBIENTLIGHT;
                pTXBuff[1]  = DATATYPE_SINGLE_VALUE;
                pTXBuff[2]  = 4;
                Sensors_GetAmbientLightReading(&pTXBuff[3], &pTXBuff[4], &pTXBuff[5], &pTXBuff[6]);
                nTXBufferLen = 7;
                Logger_INFO("A");
            }
            break;

            default:
                nTXBufferLen=0;
                Logger_INFO("D");
            break;
        }                   
    }
    // PlanSystem_ParseBuffer returned false
    else
    {
        nTXBufferLen=0;
        Logger_INFO("FF");
    }

    // Setup response if necessary
    // -- Check TX length
    if(nTXBufferLen>= TX_BUFF_SIZE-1)
    {
        nTXBufferLen = TX_BUFF_SIZE;
    }
    // -- Send data
    if(nTXBufferLen > 0)
    {
        Logger_DEBUG("TX");
        
        uint32_t timeout = HAL_GetTick() + 5000;
        I2C_SendBuffer(pTXBuff, nTXBufferLen);
        while (I2C_DeviceInReadyState() != true)
        {
            if(HAL_GetTick() > timeout)
            {
                Logger_ERROR("TX timeout");
                return;
            }
        }
        Logger_DEBUG("TX end");
    }
}

