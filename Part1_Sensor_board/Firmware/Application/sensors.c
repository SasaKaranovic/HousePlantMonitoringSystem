#include "sensors.h"
#include "adc.h"
#include "i2c.h"
#include "stm32_sw_i2c.h"


#define MIN_LOG_LEVEL_INFO
#define LOGGER_TAG "SENS"
#include "logger.h"

#define TEMP_SENS_ADDRESS 	0x90
#define ADC_SOIL_CHANNEL 	ADC_CHANNEL_0
#define ADC_ALS_CHANNEL 	ADC_CHANNEL_9

uint32_t nADC_soilMoisture = 0;
uint32_t nADC_ambientLight = 0;
int8_t temperatureDeg = 0;
uint8_t temperatureFrac = 0;


// Take temperature, soil moisture and ambient light reading
void Sensor_TakeMeasurement(void)
{
    MCP9800_readTemp(&temperatureDeg, &temperatureFrac);
    nADC_soilMoisture = ADC_Measure(ADC_SOIL_CHANNEL);
    nADC_ambientLight = ADC_Measure(ADC_ALS_CHANNEL);
}

void Sensors_PrintDebug(void)
{
	Logger_INFO("T:%d.%dC - M: %d - L: %d", temperatureDeg, temperatureFrac, nADC_soilMoisture, nADC_ambientLight);
}

// Read temperature from MCP9800 temperature sensor
void MCP9800_readTemp(int8_t *deg, uint8_t *frac)
{
	if( (deg==NULL) || frac==NULL)
	{
		return;
	}

	uint8_t _txBuffer[3] = {0};
	uint8_t _rxBuffer[3] = {0};
	I2C_TXBuffer(TEMP_SENS_ADDRESS, _txBuffer, 1);
	I2C_RXBuffer(TEMP_SENS_ADDRESS, _rxBuffer, 2);

	*deg  = _rxBuffer[0];
	*frac = (_rxBuffer[1] >> 7) * 5;
}

// Get last temperature reading
void Sensors_GetTemperatureReading(uint8_t *deg, uint8_t *frac)
{
	if(deg==NULL || frac==NULL)
	{
		return;
	}

	*deg = temperatureDeg;
	*frac = temperatureFrac;
}


// Get last soil moisture reading
// returns uin32_t soil moisture value as 4 individual uint8_t bytes
void Sensors_GetSoilMoistureReading(uint8_t *b3, uint8_t *b2, uint8_t *b1, uint8_t *b0)
{
	if( (b3==NULL) || (b2==NULL) || (b1==NULL) || (b0==NULL))
	{
		return;
	}

	*b0 = (uint8_t)(nADC_soilMoisture 			& 0xFF);
	*b1 = (uint8_t)((nADC_soilMoisture >> 8) 	& 0xFF);
	*b2 = (uint8_t)((nADC_soilMoisture >> 16) 	& 0xFF);
	*b3 = (uint8_t)((nADC_soilMoisture >> 24) 	& 0xFF);
}


// Get last ambient light reading
// returns uin32_t ambient light value as 4 individual uint8_t bytes
void Sensors_GetAmbientLightReading(uint8_t *b3, uint8_t *b2, uint8_t *b1, uint8_t *b0)
{
	if( (b3==NULL) || (b2==NULL) || (b1==NULL) || (b0==NULL))
	{
		return;
	}

	*b0 = (uint8_t)(nADC_ambientLight 			& 0xFF);
	*b1 = (uint8_t)((nADC_ambientLight >> 8) 	& 0xFF);
	*b2 = (uint8_t)((nADC_ambientLight >> 16) 	& 0xFF);
	*b3 = (uint8_t)((nADC_ambientLight >> 24) 	& 0xFF);
}




