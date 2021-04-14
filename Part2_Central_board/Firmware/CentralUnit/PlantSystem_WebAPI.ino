extern I2C_Device_TypeDef gDevicesOnBus[250];
extern uint8_t gDevicesOnBusCount;

extern I2C_Sensor_TypeDef gSensorsOnBus[20];
extern uint8_t gnSensorsOnBusCnt;

extern I2C_Solenoid_TypeDef gSolenoidsOnBus[20];
extern uint8_t gnSolenoidsOnBusCnt;

// -- Format char buffer to return data as JSON object
// JSON object will have item count and then each item's
// I2C address and as what type device registered.
bool PlantSystem_WAPI_GetDevices(char *pBuff, uint32_t nMaxLen)
{
	if(pBuff == NULL)
	{
		return false;
	}
	if(nMaxLen <= 2)
	{
		return false;
	}

	uint32_t currentPos = 0;
	int len;

	char str_sensor[] = "sensor";
	char str_solenoid[] = "solenoid";
	char str_unknown[] = "unknown";
 	char *pStrDevTyep = str_unknown;

	// Start JSON string
	len = snprintf(&pBuff[currentPos], (nMaxLen-currentPos), "{\"count\":%d,\"devices\":[", 
																gDevicesOnBusCount);
	if(len<0)
	{
		Serial.println("Buffer too short");
		return false;
	}
	else
	{
		currentPos += len;
	}

	// Construct response based on devices currently registered on the bus
	for(uint8_t i=0; i<gDevicesOnBusCount; i++)
	{
		if(gDevicesOnBus[i].deviceType == DEVICETYPE_SENSOR_V1)
		{
			pStrDevTyep = str_sensor;
		}
		else if (gDevicesOnBus[i].deviceType == DEVICETYPE_SOLENOID_V1)
		{
			pStrDevTyep = str_solenoid;
		}
		else
		{
			pStrDevTyep = str_unknown;
		}


		len = snprintf(&pBuff[currentPos], (nMaxLen-currentPos), "{\"address\":\"0x%02X\",\"type\":\"%s\"},",
							gDevicesOnBus[i].i2cAddress, 
							pStrDevTyep);
		if(len>0)
		{
			currentPos += len;
		}
		else
		{
			Serial.println("Len issue");
			Serial.println(len, DEC);
			return false;
		}
	}

	// Close JSON string
	len = snprintf(&pBuff[currentPos-1], (nMaxLen-currentPos), "]}");

	if(len >0 && currentPos <nMaxLen)
	{
		#if 0
		Serial.println("Printing pBuff");
		Serial.println(pBuff);
		#endif
		return true;
	}
	else
	{
		return false;
	}
}


// -- Format char buffer to return data as JSON object
// JSON object will have total sensors count and then sensors
// will contain each sensors I2C address and what is last reported
// temperature, soil moisture and ambient light
bool PlantSystem_WAPI_SensorData(char *pBuff, uint32_t nMaxLen)
{
	if(pBuff == NULL)
	{
		return false;
	}
	if(nMaxLen <= 64)
	{
		return false;
	}

	uint32_t currentPos = 0;
	int len;


	// Start JSON string
	len = snprintf(&pBuff[currentPos], (nMaxLen-currentPos), "{\"count\":%d,\"sensors\":[",
																gnSensorsOnBusCnt);
	if(len<0)
	{
		Serial.println("Buffer too short");
		return false;
	}
	else
	{
		currentPos += len;
	}

	// Construct response based on devices currently registered on the bus
	for(uint8_t i=0; i<gnSensorsOnBusCnt; i++)
	{
		len = snprintf(&pBuff[currentPos], (nMaxLen-currentPos), "{\"address\":\"0x%02X\",\"T\":%.2f,\"M\":%d,\"A\":%d},",
									gSensorsOnBus[i].i2cAddress, 
									gSensorsOnBus[i].temperature, 
									gSensorsOnBus[i].soilMoisture,
									gSensorsOnBus[i].ambienLight);
		if(len>0)
		{
			currentPos += len;
		}
		else
		{
			Serial.println("Len issue");
			Serial.println(len, DEC);
			return false;
		}
	}


	// Close JSON string
	len = snprintf(&pBuff[currentPos-1], (nMaxLen-currentPos), "]}");
	if(len<0)
	{
		Serial.println("Buffer too short");
		return false;
	}

	if(len>0 && currentPos <nMaxLen)
	{
		#if 0
		Serial.println("Printing pBuff");
		Serial.println(pBuff);
		#endif
		return true;
	}
	else
	{
		return false;
	}
}


// -- Format char buffer to return data as JSON object
// JSON object will have total solenoids count and then I2C
// address of each sensor
bool PlantSystem_WAPI_SolenoidList(char *pBuff, uint32_t nMaxLen)
{
	if(pBuff == NULL)
	{
		return false;
	}
	if(nMaxLen <= 32)
	{
		return false;
	}

	uint32_t currentPos = 0;
	int len;


	// Start JSON string
	len = snprintf(&pBuff[currentPos], (nMaxLen-currentPos), "{\"count\":%d,\"solenoids\":[",
																gnSolenoidsOnBusCnt);
	if(len<0)
	{
		Serial.println("Buffer too short");
		return false;
	}
	else
	{
		currentPos += len;
	}

	// Construct response based on devices currently registered on the bus
	for(uint8_t i=0; i<gnSolenoidsOnBusCnt; i++)
	{
		len = snprintf(&pBuff[currentPos], (nMaxLen-currentPos), "\"0x%02X\",",
									gSolenoidsOnBus[i].i2cAddress);

		if(len>0)
		{
			currentPos += len;
		}
		else
		{
			Serial.println("Len issue");
			Serial.println(len, DEC);
			return false;
		}
	}


	// Close JSON string
	len = snprintf(&pBuff[currentPos-1], (nMaxLen-currentPos), "]}");
	if(len<0)
	{
		Serial.println("Buffer too short");
		return false;
	}

	if(len>0 && currentPos <nMaxLen)
	{
		#if 0
		Serial.println("Printing pBuff");
		Serial.println(pBuff);
		#endif
		return true;
	}
	else
	{
		return false;
	}
}
