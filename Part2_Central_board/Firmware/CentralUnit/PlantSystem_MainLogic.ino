// Plant system note
// CMD (1byte) + DATATYPE (1byte) + DATALEN (2bytes) 


// Typedefs to allow us easier tracking on devices on the bus and storing received data
typedef struct I2C_Device_TypeDef
{
    uint8_t i2cAddress;
    uint8_t deviceType;
} I2C_Device_TypeDef; 

typedef struct I2C_Sensor_TypeDef
{
    uint8_t i2cAddress;
    float temperature;
    uint32_t soilMoisture;
    uint32_t ambienLight;
} I2C_Sensor_TypeDef;

typedef struct I2C_Solenoid_TypeDef
{
    uint8_t i2cAddress;
    uint8_t lastState;
} I2C_Solenoid_TypeDef;

// Every device that we saw on the bus
I2C_Device_TypeDef gDevicesOnBus[250] = { {.i2cAddress=0, .deviceType=0} };
uint8_t gDevicesOnBusCount = 0;

// All sensors that are registered on the bus
I2C_Sensor_TypeDef gSensorsOnBus[20] = { {.i2cAddress=0, .temperature=0.0, .soilMoisture=0, .ambienLight=0} };
uint8_t gnSensorsOnBusCnt = 0;

// All solenoids that are on the bus
I2C_Solenoid_TypeDef gSolenoidsOnBus[20] = { {.i2cAddress=0, .lastState=0} };
uint8_t gnSolenoidsOnBusCnt = 0;

// Periods
#define PERIOD_RESCAN_BUS		24*3600*1000	// Every 24h
#define PERIOD_RELOAD_SENSORS	15*1000			// Every 5sec

uint32_t nPeriod_Rescanbus=0;
uint32_t nPeriod_ReloadSensors=0;

void PlantSystem_tick(void)
{
	// -- Rescan I2C bus
	if(nPeriod_Rescanbus <= millis())
	{
		PlantSystem_ScanBus();
		#if 1
		PlantSystem_debugShowDevicesOnBus();
		#endif
		nPeriod_Rescanbus = millis() + PERIOD_RESCAN_BUS;
	}

	// -- Reload sensor data
	if(nPeriod_ReloadSensors <= millis())
	{
		PlantSystem_UpdateSensors();
		nPeriod_ReloadSensors = millis() + PERIOD_RELOAD_SENSORS;
	}
	
}


void PlantSystem_ScanBus(void)
{
	Serial.println ("Scanning I2C bus...");
	for (uint8_t i = 8; i< 127; i++)
	{
		Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
		if (Wire.endTransmission () == 0)  	// Receive 0 = success (ACK response) 
		{
			gDevicesOnBus[gDevicesOnBusCount++].i2cAddress = i;
			// gDevicesOnBusCount++;
		}
	}
	Serial.print ("Found ");      
	Serial.print (gDevicesOnBusCount, DEC);        // numbers of devices
	Serial.println (" device(s).");

	for(uint8_t i=0; i<gDevicesOnBusCount; i++)
	{
		PlantSystem_getDeviceInfo(i);
	}
}


void PlantSystem_debugShowDevicesOnBus(void)
{
	uint8_t count = 0;

	// -- Print all devices
	Serial.print ("There are total of ");
	Serial.print (gDevicesOnBusCount, DEC);        // numbers of devices
	Serial.println (" device(s) on I2C bus.");

	for (uint8_t i = 0; i< gDevicesOnBusCount; i++)
	{
		Serial.print ("Device @ 0x");
		Serial.print (gDevicesOnBus[i].i2cAddress, HEX);        // numbers of devices
		if(gDevicesOnBus[i].deviceType == DEVICETYPE_SENSOR_V1)
		{
			Serial.println (" registered as Temp+ALS+SoilMoisture.");
		}
		else if(gDevicesOnBus[i].deviceType == DEVICETYPE_SOLENOID_V1)
		{
			Serial.println (" registered as Solenoid driver");
		}
		else if(gDevicesOnBus[i].deviceType == 0x00 || gDevicesOnBus[i].deviceType == 0xFF)
		{
			Serial.println (" responded with invalid value?!");
		}
		else if(gDevicesOnBus[i].deviceType <= DEVICETYPE_FIRST || gDevicesOnBus[i].deviceType >= DEVICETYPE_LAST)
		{
			Serial.println (" responded with unsupported value.");
		}
		
	}

	// -- Print all sensors
	Serial.print ("There are total of ");
	Serial.print (gnSensorsOnBusCnt, DEC);        // numbers of devices
	Serial.println (" SENSORS on I2C bus.");
	for (uint8_t i = 0; i< gnSensorsOnBusCnt; i++)
	{
		Serial.print("Sensor @ 0x");
		Serial.println (gSensorsOnBus[i].i2cAddress, HEX);        // numbers of devices
	}

	// -- Print all solenoids
	Serial.print ("There are total of ");
	Serial.print (gnSolenoidsOnBusCnt, DEC);        // numbers of devices
	Serial.println (" SOLENOIDS on I2C bus.");
	for (uint8_t i = 0; i< gnSolenoidsOnBusCnt; i++)
	{
		Serial.print("Solenoid @ 0x");
		Serial.println (gSolenoidsOnBus[i].i2cAddress, HEX);        // numbers of devices
	}

}

// -- Get device info
// Used to get information from the device (type of device i.e. sensor or solenoid etc).
// Add each device to gDevicesOnBus
// Also add to gSensorsOnBus if type is sensor or gSolenoidsOnBus if type is solenoid
void PlantSystem_getDeviceInfo(uint8_t devID)
{
	if(devID > gDevicesOnBusCount)
	{
		Serial.println("Invalid device ID");
		return;
	}

	if(gDevicesOnBus[devID].i2cAddress == 0)
	{
		Serial.println("Invalid device I2C address");
		return;
	}

	Serial.print("I2C: Talking to address 0x");
	Serial.println(gDevicesOnBus[devID].i2cAddress, HEX);
	
	uint8_t txBuffer[4] = {CMD_GET_DEVICE_INFO, DATATYPE_NONE, 0, 0};

	I2C_SendBuffer(gDevicesOnBus[devID].i2cAddress, txBuffer, 4);
	delay(200);

	uint8_t deviceType = 0;
	I2C_ReadFrom(gDevicesOnBus[devID].i2cAddress, 10,  &deviceType, PLANTSYSTEM_MIN_HEADER_LENGHT+1, 1);
	gDevicesOnBus[devID].deviceType = deviceType;

	// Finally add sensors to gSensorsOnBus and solenoids to gSolenoidsOnBus
	if(deviceType == DEVICETYPE_SENSOR_V1)
	{
		gSensorsOnBus[gnSensorsOnBusCnt++].i2cAddress = gDevicesOnBus[devID].i2cAddress;
	}
	else if(deviceType == DEVICETYPE_SOLENOID_V1)
	{
		gSolenoidsOnBus[gnSolenoidsOnBusCnt++].i2cAddress = gDevicesOnBus[devID].i2cAddress;
	}

	#if 0
	Serial.println ("Response");
	for(uint8_t i=0; i<10; i++)
	{
		Serial.print ("[");
		Serial.print (i, DEC);
		Serial.print ("] = 0x");
		Serial.println (rxBuffer[i], HEX);
	}
	#endif

}

// -- 
bool PlantSystem_SetSolenoidState(uint32_t I2CAddress, bool energized)
{
	if(I2CAddress < 0x08 || I2CAddress > 0x7F )
	{
		return false;
	}
	
	uint8_t txBuffer[5];
	uint8_t requestedState = (energized ? 1 : 0);

	// Send new solenoid value
	txBuffer[0] = CMD_SET_SOLENOID_STATE;
	txBuffer[1] = DATATYPE_SINGLE_VALUE;
	txBuffer[2] = 0;
	txBuffer[3] = 1;
	txBuffer[4] = requestedState;
	I2C_SendBuffer(I2CAddress, txBuffer, 5);

	// Wait 100 ms
	delay(100);

	// Send read back value request
	txBuffer[0] = CMD_GET_SOLENOID_STATE;
	txBuffer[1] = DATATYPE_NONE;
	txBuffer[2] = 0;
	txBuffer[3] = 0;
	I2C_SendBuffer(I2CAddress, txBuffer, 4);
	
	// Wait 100 ms
	delay(100);

	// Read new solenoid state
	uint8_t newState = 0;
	I2C_ReadFrom(I2CAddress, 10, &newState, PLANTSYSTEM_MIN_HEADER_LENGHT+1, 1);

	if(requestedState == newState)
	{
		return true;
	}
	else
	{
		return false;
	}

}


// -- Read data from each registered sensor
// Temperature, Soil moisture and ambient light
void PlantSystem_UpdateSensors(void)
{
	uint8_t txBuffer[4] = {CMD_GET_TEMPERATURE, DATATYPE_NONE, 0, 0};
	uint8_t rxBuffer[4] = {0};

	Serial.print("Reading data from sensors...");
	for(uint8_t i=0; i<gnSensorsOnBusCnt; i++)
	{
		#if 0
		Serial.print("Reading from sensor #");
		Serial.print(i, DEC);
		Serial.print(" address: 0x");
		Serial.println(gSensorsOnBus[i].i2cAddress, HEX);
		#endif

		// -- Temperature
		// Send read temperature request
		txBuffer[0] = CMD_GET_TEMPERATURE;
		I2C_SendBuffer(gSensorsOnBus[i].i2cAddress, txBuffer, 4);
		delay(200);

		// Read response
		I2C_ReadFrom(gSensorsOnBus[i].i2cAddress, 10, rxBuffer, PLANTSYSTEM_MIN_HEADER_LENGHT+1, 2);
		gSensorsOnBus[i].temperature = rxBuffer[0] + rxBuffer[1]/10;
		delay(200);

		#if 0
		Serial.print("T: ");
		Serial.print(rxBuffer[0], DEC);
		Serial.println(rxBuffer[1], DEC);
		#endif

		// -- Soil moisture
		// Send read soilmositure request
		txBuffer[0] = CMD_GET_SOILMOISTURE;
		I2C_SendBuffer(gSensorsOnBus[i].i2cAddress, txBuffer, 4);
		delay(200);

		// Read response
		I2C_ReadFrom(gSensorsOnBus[i].i2cAddress, 10, rxBuffer, PLANTSYSTEM_MIN_HEADER_LENGHT+1, 4);
		gSensorsOnBus[i].soilMoisture = (rxBuffer[0]<<24) | (rxBuffer[1]<<16) | (rxBuffer[2]<<8) | (rxBuffer[3]);
		delay(200);

		#if 0
		Serial.print("M: ");
		Serial.print(rxBuffer[0], DEC);
		Serial.print(rxBuffer[1], DEC);
		Serial.print(rxBuffer[2], DEC);
		Serial.println(rxBuffer[3], DEC);
		#endif

		// -- Ambient light
		// Send read soilmositure request
		txBuffer[0] = CMD_GET_AMBIENTLIGHT;
		I2C_SendBuffer(gSensorsOnBus[i].i2cAddress, txBuffer, 4);
		delay(200);

		// Read response
		I2C_ReadFrom(gSensorsOnBus[i].i2cAddress, 10, rxBuffer, PLANTSYSTEM_MIN_HEADER_LENGHT+1, 4);
		gSensorsOnBus[i].ambienLight = (rxBuffer[0]<<24) | (rxBuffer[1]<<16) | (rxBuffer[2]<<8) | (rxBuffer[3]);
		delay(200);

		#if 0
		Serial.print("A: ");
		Serial.print(rxBuffer[0], DEC);
		Serial.print(rxBuffer[1], DEC);
		Serial.print(rxBuffer[2], DEC);
		Serial.println(rxBuffer[3], DEC);
		#endif

		#if 0
		Serial.print("Sensor #");
		Serial.println(i, DEC);

		Serial.print("T: ");
		Serial.println((float)gSensorsOnBus[i].temperature, 1);

		Serial.print("S: ");
		Serial.println(gSensorsOnBus[i].soilMoisture, DEC);

		Serial.print("A: ");
		Serial.println(gSensorsOnBus[i].ambienLight, DEC);
		#endif
	}
	Serial.println("done.");

}

// -- Send `nLen` bytes from `pBuffer` to `address` I2C address
void I2C_SendBuffer(uint8_t address, uint8_t *pBuffer, uint8_t nLen)
{
	Wire.beginTransmission(address); // transmit to device
	for(uint8_t i=0; i<nLen; i++)
	{
		Wire.write(pBuffer[i]);
	}
	Wire.endTransmission();    // stop transmitting}
}


// -- Read `nLen` bytes from `address` I2C address
// Then store (return) `nBytes` to `pData` starting from `nPos` (discard)
bool I2C_ReadFrom(uint8_t address, uint8_t nLen, uint8_t *pData, uint8_t nPos, uint8_t nBytes)
{
	if(nLen > 20)
	{
		Serial.println("Invalid length!");
		return false;
	}

	if(pData == NULL && nBytes > 0)
	{
		Serial.println("Invalid pointer!");
		return false;
	}

	if(nBytes+nPos > nLen)
	{
		Serial.println("Buffer overflow");
		return false;
	}

	uint8_t rxBuffer[20] = {0};

	Wire.requestFrom(address, nLen, true);
	while(Wire.available() == 0);
	for(uint8_t i=0; i<nLen; i++)
	{
		rxBuffer[i] = Wire.read();
	}
	Wire.endTransmission();    // stop transmitting

	// Copy over bytes
	if(nBytes > 0)
	{
		for(uint8_t i=0; i<nBytes; i++)
		{
			pData[i] = rxBuffer[nPos+i];
		}
	}

	#if 0
	Serial.println ("Response");
	for(uint8_t i=0; i<nLen; i++)
	{
		Serial.print ("[");
		Serial.print (i, DEC);
		Serial.print ("] = 0x");
		Serial.println (rxBuffer[i], HEX);
	}
	#endif

	return true;
}


