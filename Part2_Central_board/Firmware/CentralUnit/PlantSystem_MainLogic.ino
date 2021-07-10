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

// I2C bus config
#define I2C_ADDRESS_LIMIT_LOWER		0x08
#define I2C_ADDRESS_LIMIT_UPPER		0x7F

// Plant watering definitions
#define WATERING_VOLUME_MINIMUM		10
#define WATERING_VOLUME_MAXIMUM		550
#define WATERING_FLOW_EDGES_PER_L	5880	//98

// PlantSystem_tick periods
#define PERIOD_RESCAN_BUS			24*3600*1000	// Every 24h
#define PERIOD_RELOAD_SENSORS		15*1000			// Every 5sec
#define PERIOD_FLOW_RATE			2*1000			// Every 2sec

// PlantSystem_tick global variables
uint32_t nPeriod_Rescanbus=0;
uint32_t nPeriod_ReloadSensors=0;
uint32_t nPeriod_FlowRate=0;

// Watering global variables
bool bPrimeSystem_requestPending=false;
bool bWatering_requestPending=false;
bool bWatering_inProgress=false;
bool bCloseValvesOnPowerUp=false;
uint32_t nWatering_volume=0;
uint8_t nWatering_i2cAddress=0;
uint32_t nTimeout = 0;


void PlantSystem_tick(void)
{
	bool bWateringError = false;
	esp_task_wdt_reset();

	// -- Rescan I2C bus
	if(nPeriod_Rescanbus <= millis())
	{
		esp_task_wdt_reset();
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


	// -- Close all solenoids on boot
	if(bCloseValvesOnPowerUp == false)
	{
		// Turn OFF all solenoids
		delay(500);
		esp_task_wdt_reset();
		Serial.println("De-energizing ALL solenoids");
		for(uint8_t i=0; i<gnSolenoidsOnBusCnt; i++)
		{
			if (PlantSystem_SetSolenoidState(gSolenoidsOnBus[i].i2cAddress, false) == true)
			{
				Serial.print("Closed solenoid 0x");
				Serial.println(gSolenoidsOnBus[i].i2cAddress, HEX);
			}
			else
			{
				Serial.print("Failed to close solenoid 0x");
				Serial.println(gSolenoidsOnBus[i].i2cAddress, HEX);
			}
			delay(500);
		}
		bCloseValvesOnPowerUp = true;
	}

	#if 0
	// -- Flow sensor debug
	if(nPeriod_FlowRate <= millis())
	{
		Serial.print("nFlowSensorCount: ");
		Serial.println(nFlowSensorCount, DEC);
		nPeriod_FlowRate = millis() + PERIOD_FLOW_RATE;
	}
	#endif

	// -- Process watering request
	if(bWatering_requestPending || bWatering_inProgress)
	{
		//	Watering logic
		//  Pump creates high pressure inside watering system. Because of this
		//	at the beginning of watering process we will first, open valve, then turn on pump.
		//	When we are done, we will first turn off pump and then solenoid.
		//
		//	In previous version, water pump was too weak so we turned on the pump first to keep the pressure on
		//	and prevent water from "backing up" (i.e letting air in from plant side).
		//	With peristaltic pump we can open the solenoids first and still keep air out of the system.
		//

		Serial.println("bWatering_requestPending | bWatering_inProgress");
		// Transition to in progress
		bWatering_inProgress = true;
		bWatering_requestPending = false;
		nFlowSensorCount = 0;

		// - Open solenoid
		Serial.println("Energizing solenoid");
		if (PlantSystem_SetSolenoidState(nWatering_i2cAddress, true) != true)
		{
			bWatering_requestPending = false;
			bWatering_inProgress = false;
			digitalWrite(WATER_PUMP_EN_PIN, LOW);
			Serial.println("Turning pump OFF");
			Serial.println("Failed to open solenoid. Aborting...");
			LED_Blink(4, 250);
			return;
		}


		// - Reset flow counter
		nFlowSensorCount = 0;
		nFlowSensorCount_last = 0;
		flowInterruptEnabled(true);
		
		// - Turn ON pump
		Serial.println("Turning pump ON");
		delay(200);
		digitalWrite(WATER_PUMP_EN_PIN, HIGH);

		esp_task_wdt_reset();
		nTimeout = millis() + 10000;	// 10sec time-out for watering
		
		// Calculate water flow
		uint32_t nWaterFlow_millis_timestamp = millis();
		float flowRate = 0.0;
		unsigned int flowMilliLitres =0;
		unsigned long totalMilliLitres = 0;

		while(millis() < nTimeout)
		{
			if((millis() - nWaterFlow_millis_timestamp) > 1000)    // Only process counters once per second
			{
				flowInterruptEnabled(false);

				nFlowSensorCount_last = nFlowSensorCount;
				nFlowSensorCount = 0;
				bWateringError = true;
				

				flowRate = ((1000.0 / (millis() - nWaterFlow_millis_timestamp)) * nFlowSensorCount_last) / WATERING_FLOW_EDGES_PER_L;
				flowMilliLitres = (flowRate / 60) * 1000;
				totalMilliLitres += flowMilliLitres;

				if(totalMilliLitres >= nWatering_volume)
				{
					break;
				}

				nWaterFlow_millis_timestamp = millis();
				esp_task_wdt_reset();
				flowInterruptEnabled(true);
			}
		}
		esp_task_wdt_reset();

		// - Turn OFF pump
		digitalWrite(WATER_PUMP_EN_PIN, LOW);
		Serial.println("Turning pump OFF");
		delay(200);

		// - Close solenoid
		Serial.println("De-Energizing solenoid");
		PlantSystem_SetSolenoidState(nWatering_i2cAddress, false);


		// Store last flow data and prepare for new request
		nFlowSensorCount_last = 0;
		nFlowSensorCount = 0;
		nWatering_volume = 0;

		// Reset request
		bWatering_inProgress = false;
		bWatering_requestPending = false;
		if(bWateringError)
		{
			LED_Blink(3, 250);
		}
		else
		{
			LED_Blink(2, 500);
		}
	}


	// -- Process prime system with water request
	if(bPrimeSystem_requestPending)
	{
		esp_task_wdt_reset();
		PlantSystem_PrimeSystemWithWater();
		bPrimeSystem_requestPending = false;
	}
}


void PlantSystem_ScanBus(void)
{
	Serial.println ("Scanning I2C bus...");
	for (uint8_t i=I2C_ADDRESS_LIMIT_LOWER; i<I2C_ADDRESS_LIMIT_UPPER; i++)
	{
		Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
		if (Wire.endTransmission () == 0)  	// Receive 0 = success (ACK response) 
		{
			gDevicesOnBus[gDevicesOnBusCount++].i2cAddress = i;
			// gDevicesOnBusCount++;
		}
	}
	esp_task_wdt_reset();
	Serial.print ("Found ");      
	Serial.print (gDevicesOnBusCount, DEC);        // numbers of devices
	Serial.println (" device(s).");

	for(uint8_t i=0; i<gDevicesOnBusCount; i++)
	{
		PlantSystem_getDeviceInfo(i);
		delay(300);
		esp_task_wdt_reset();
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
	if(I2CAddress<I2C_ADDRESS_LIMIT_LOWER || I2CAddress>I2C_ADDRESS_LIMIT_UPPER )
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
	delay(200);

	// Send read back value request
	txBuffer[0] = CMD_GET_SOLENOID_STATE;
	txBuffer[1] = DATATYPE_NONE;
	txBuffer[2] = 0;
	txBuffer[3] = 0;
	I2C_SendBuffer(I2CAddress, txBuffer, 4);
	
	// Wait 100 ms
	delay(200);

	// Read new solenoid state
	uint8_t retData = 0;
	I2C_ReadFrom(I2CAddress, 10, &retData, PLANTSYSTEM_MIN_HEADER_LENGHT+1, 1);

	if(requestedState == retData)
	{
		Serial.print("New state == Requested state 0x");
		Serial.print(retData, DEC);
		Serial.print("==0x");
		Serial.println(requestedState, DEC);
		return true;
	}
	else
	{
		Serial.print("New state != Requested state 0x");
		Serial.print(retData, DEC);
		Serial.print("!=0x");
		Serial.println(requestedState, DEC);
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

	// Prepare variables
	uint8_t rxBuffer[20] = {0};
	uint32_t timeout = millis() + 2000;
	bool bReady = false;

	Wire.requestFrom(address, nLen, true);
	while(millis() <= timeout)
	{
		if(Wire.available())
		{
			bReady = true;
			break;
		}
	}

	if(bReady == false)
	{
		return false;
	}

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



// -- Request opening of solenoid at address `solenoidAddress` 
// 	  and pumping out (approximately) `n_volume_in_mL` mL of fluid 
bool PlantSystem_WaterPlant(uint8_t solenoidAddress, uint32_t n_volume_in_mL)
{
	// Valid I2C address requested
	if(solenoidAddress<I2C_ADDRESS_LIMIT_LOWER || solenoidAddress>I2C_ADDRESS_LIMIT_UPPER)
	{
		Serial.println("Invalid I2C address!");
		return false;
	}

	// Valid volume requested
	if(n_volume_in_mL < WATERING_VOLUME_MINIMUM || n_volume_in_mL > WATERING_VOLUME_MAXIMUM)
	{
		Serial.println("Invalid volume requested!");
		return false;
	}

	// Check if solenoid with this address is registered on the bus
	bool bFound = false;
	for(uint8_t i=0; i<gnSolenoidsOnBusCnt; i++)
	{
		if(gSolenoidsOnBus[i].i2cAddress == solenoidAddress)
		{
			bFound=true;
			break;
		}
	}

	// Solenoid not found or invalid request
	if(bFound == false)
	{
		Serial.println("Solenoid not found!");
		return false;
	}

	// Valid request, add to next tick

	bWatering_requestPending=true;
	nWatering_volume=n_volume_in_mL;
	nWatering_i2cAddress=solenoidAddress;


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

void PlantSystem_RequestPrimeSystem(void)
{
	bPrimeSystem_requestPending = true;
}


bool PlantSystem_PrimeSystemWithWater(void)
{

	bool bSolenoidOpen = false;
	
	// Turn ON all solenoids
	Serial.println("Energizing ALL solenoids");
	for(uint8_t i=0; i<gnSolenoidsOnBusCnt; i++)
	{
		if (PlantSystem_SetSolenoidState(gSolenoidsOnBus[i].i2cAddress, true) == true)
		{
			Serial.print("Openned solenoid 0x");
			Serial.println(gSolenoidsOnBus[i].i2cAddress, HEX);
			bSolenoidOpen = true;
		}
		else
		{
			Serial.print("Failed to open solenoid 0x");
			Serial.println(gSolenoidsOnBus[i].i2cAddress, HEX);
		}
		delay(500);

		// Only turn on pump AFTER solenoid has oppened
		if(bSolenoidOpen)
		{
			// Turn ON Pump
			Serial.println("Turning pump ON");
			digitalWrite(WATER_PUMP_EN_PIN, HIGH);
			delay(1000);
		}
	}

	// Let the water flow for a while
	delay(5000);

	// Turn OFF pump
	delay(1000);
	Serial.println("Turning pump OFF");
	digitalWrite(WATER_PUMP_EN_PIN, LOW);

	// Turn OFF all solenoids
	Serial.println("De-energizing ALL solenoids");
	for(uint8_t i=0; i<gnSolenoidsOnBusCnt; i++)
	{
		if (PlantSystem_SetSolenoidState(gSolenoidsOnBus[i].i2cAddress, false) == true)
		{
			Serial.print("Closed solenoid 0x");
			Serial.println(gSolenoidsOnBus[i].i2cAddress, HEX);
		}
		else
		{
			Serial.print("Failed to close solenoid 0x");
			Serial.println(gSolenoidsOnBus[i].i2cAddress, HEX);
		}
		delay(500);
	}


	nFlowSensorCount = 0;
	return true;

}

