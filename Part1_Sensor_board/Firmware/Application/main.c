#include "main.h"
#include "board.h"
#include "i2c.h"
#include "sensors.h"
#include "PlantSystem.h"

#define MIN_LOG_LEVEL_INFO
#define LOGGER_TAG "MAIN"
#include "logger.h"

uint32_t nTickTarget_PrintDebug		 = 0;
uint32_t nTickTarget_TakeMeasurement = 0;

#define TX_BUFF_SIZE	10
static uint8_t pTXBuff[TX_BUFF_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static uint8_t nTXBufferLen = 0;

int main(void)
{
	BoardInit();

	Logger_INFO("Hello there!");
	HAL_Delay(1000);

	__enable_irq();
	uint8_t *pBuff = NULL;
	uint8_t nBufferLen = 0;
	HAL_I2C_StateTypeDef status;

	while (1)
	{

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

		// Wait for I2C to be IDLE (HAL_I2C_STATE_READY)
		status = HAL_I2C_GetState(&hi2c1);
		// if(status == HAL_I2C_STATE_READY)
		// {

			// Check if we received enough bytes to start processing
			nBufferLen = I2C_GetBufferLenght();
			if(nBufferLen >= PLANTSYSTEM_MIN_HEADER_LENGHT)
			{
				I2C_GetBuffer(&pBuff, &nBufferLen);

				#if 1
				uint8_t i=0;
				for (i=0; i<5; i++)
				{
				  Logger_INFO("0x%02X", pBuff[i]);
				}
				#endif

				uint8_t cmd=0;
				uint8_t dataType=0;
				uint8_t dataLength=0;

				if (PlanSystem_ParseBuffer(pBuff, nBufferLen, &cmd, &dataType, &dataLength))
				{
					Logger_INFO("Success");

					switch(cmd)
					{
						// Get device information
						case CMD_GET_DEVICE_INFO:
						{
							nTXBufferLen=0;
							Logger_INFO("I");
						}
						break;

						// Read temperature
						case CMD_GET_TEMPERATURE:
						{
							pTXBuff[0]	= CMD_GET_TEMPERATURE;
							pTXBuff[1]	= DATATYPE_SINGLE_VALUE;
							pTXBuff[2]	= 2;
							Sensors_GetTemperatureReading(&pTXBuff[3], &pTXBuff[4]);
							nTXBufferLen = 5;
							Logger_INFO("T");
						}
						break;

						// Read soil moisture
						case CMD_GET_SOILMOISTURE:
						{
							pTXBuff[0]	= CMD_GET_SOILMOISTURE;
							pTXBuff[1]	= DATATYPE_SINGLE_VALUE;
							pTXBuff[2]	= 4;
							Sensors_GetSoilMoistureReading(&pTXBuff[3], &pTXBuff[4], &pTXBuff[5], &pTXBuff[6]);
							nTXBufferLen = 7;
							Logger_INFO("M");
						}
						break;

						// Read ambient light
						case CMD_GET_AMBIENTLIGHT:
						{
							pTXBuff[0]	= CMD_GET_AMBIENTLIGHT;
							pTXBuff[1]	= DATATYPE_SINGLE_VALUE;
							pTXBuff[2]	= 4;
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
					while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
					if(HAL_I2C_Slave_Transmit(&hi2c1, pTXBuff, nTXBufferLen, 0xFFFF) != HAL_OK)
					{
						while(1);
					}
					// HAL_I2C_Slave_Transmit_IT(&hi2c1, pTXBuff, nTXBufferLen);
				}

				
				// Reset I2C state machine
				I2C_ClearBuffer();
				I2C_ReloadIT();
			}

			
		// }

	}

}



