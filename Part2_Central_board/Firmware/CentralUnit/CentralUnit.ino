#include <EEPROM.h>
#include <SPIFFS.h>
#include <stdarg.h>
#include <stdio.h>
#include <esp_task_wdt.h>
#include <ESPmDNS.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "CentralUnit_cfg.h"
#include "wifi_credentials.h"
#include "PlantSystem.h"
#include "Wire.h"
#include "time.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#define WDT_TIMEOUT		20000
#define EEPROM_SIZE		4

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

AsyncWebServer server(80);
int WiFi_status = WL_IDLE_STATUS; 
volatile uint32_t nFlowSensorCount = 0;
volatile uint32_t nFlowSensorCount_last = 0;

char dbgBuff[4024] = {0};
uint32_t dbgBuffPos = 0;
float fImpulsePerML = WATERING_FLOW_EDGES_PER_mL;

void IRAM_ATTR ISR_flowSensor() {
    nFlowSensorCount++;
}

// Function prototypes
void ps_debug_log_reset_Reason(RESET_REASON reason);
void ps_debug_log_verbose_reset_reason(RESET_REASON reason);
int vprintf_into_spiffs(const char* szFormat, va_list args);
void printLocalTime();
void LOG_Timestamp(void);
void setupWebServer(void);
void flowInterruptEnabled(bool state);
void LED_Blink(uint8_t nTimes, uint16_t n_delayPeriod);
void PlantSystem_tick(void);
void PlantSystem_ScanBus(void);
void PlantSystem_debugShowDevicesOnBus(void);
void PlantSystem_getDeviceInfo(uint8_t devID);
bool PlantSystem_SetSolenoidState(uint32_t I2CAddress, bool energized);
void PlantSystem_UpdateSensors(void);
void I2C_SendBuffer(uint8_t address, uint8_t *pBuffer, uint8_t nLen);
bool I2C_ReadFrom(uint8_t address, uint8_t nLen, uint8_t *pData, uint8_t nPos, uint8_t nBytes);
bool PlantSystem_WaterPlant(uint8_t solenoidAddress, uint32_t n_volume_in_mL);
void PlantSystem_RequestPrimeSystem(void);
bool PlantSystem_PrimeSystemWithWater(void);
bool PlantSystem_WAPI_GetDevices(char *pBuff, uint32_t nMaxLen);
bool PlantSystem_WAPI_SensorData(char *pBuff, uint32_t nMaxLen);
bool PlantSystem_WAPI_SolenoidList(char *pBuff, uint32_t nMaxLen);



void setup()
{
	esp_task_wdt_init(30, true);

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	pinMode(I2C_EN_PIN, OUTPUT);
	digitalWrite(I2C_EN_PIN, HIGH);
		
	pinMode(WATER_PUMP_EN_PIN, OUTPUT);
	digitalWrite(WATER_PUMP_EN_PIN, LOW);
		
	pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
	
	Serial.begin(115200);

	// Setup Logging
	if (!SPIFFS.begin(true)) {
		Serial.println("An Error has occurred while mounting SPIFFS");
		while(1)
		{
			digitalWrite(LED_PIN, HIGH);
			delay(500);
			digitalWrite(LED_PIN, LOW);
			delay(500);
		}
	}
	else
	{
		Serial.println("SPIFFS mounted");

	}
	
	delay(500);
	esp_log_set_vprintf(&vprintf_into_spiffs);
	esp_log_level_set("*", ESP_LOG_DEBUG);
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- SPIFFS Boot up\n");

	// Log reset reason
	Serial.println("Storing reason in log.txt");
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- CPU0 ");
	ps_debug_log_verbose_reset_reason(rtc_get_reset_reason(0));
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- CPU1 ");
	ps_debug_log_verbose_reset_reason(rtc_get_reset_reason(1));

	Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

	// EEPROM begin
	EEPROM.begin(EEPROM_SIZE);

	// Debug EEPROM before read
	Serial.println("Before read: fImpulsePerML");
	Serial.println((float)(fImpulsePerML),4);
	
	// Read flow value from EEPROM
	fImpulsePerML = EEPROM.readFloat(0);
	
	// Debug EEPROM after read
	Serial.println("After read: fImpulsePerML");
	Serial.println((float)(fImpulsePerML),4);
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- fImpulsePerML: %f\n", fImpulsePerML);


	// Configure WDT
	Serial.println("Configuring WDT...");
	esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
	esp_task_wdt_add(NULL); //add current thread to WDT watch

	// Connect to WiFi
	esp_log_write(ESP_LOG_INFO, "SKWS", "-- Configuring WiFi.\n");
	WiFi.mode(WIFI_STA);
	WiFi.setHostname("sk_watering_system");
	esp_log_write(ESP_LOG_INFO, "SKWS", "-- Connecting to WiFi...\n");
	while ( WiFi_status != WL_CONNECTED) {
		Serial.print("Connecting to SSID: ");
		Serial.println(ssid);
		WiFi_status = WiFi.begin(ssid, password);

		esp_task_wdt_reset();

		digitalWrite(LED_PIN, HIGH);
		delay(500);
		digitalWrite(LED_PIN, LOW);
		delay(500);		
		digitalWrite(LED_PIN, HIGH);
		delay(500);
		digitalWrite(LED_PIN, LOW);
		delay(500);		
		digitalWrite(LED_PIN, HIGH);
		delay(500);
		digitalWrite(LED_PIN, LOW);
		delay(500);
	}
	esp_log_write(ESP_LOG_INFO, "SKWS", "-- Connected.\n");
	digitalWrite(LED_PIN, LOW);

	Serial.print("WiFi IP: ");
	Serial.println(WiFi.localIP());
	Serial.print("Hostname: ");
	Serial.println(WiFi.getHostname());

	char cIP_buff[50] = {0};
	String sIP = WiFi.localIP().toString();
	sIP.toCharArray(cIP_buff, 50);

	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- WiFi IP: %s\n", cIP_buff);
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- WiFi hostname: %s\n",WiFi.getHostname());

	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- mDNS started\n");

	// Update time
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	printLocalTime();
	LOG_Timestamp();

	// Config printout
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "\n-- I2C_BUS_RESCAN_ENABLED=%d\n", I2C_BUS_RESCAN_ENABLED);
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- PERIOD_RESCAN_BUS=%d\n", PERIOD_RESCAN_BUS);
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- PERIOD_RELOAD_SENSORS=%d\n", PERIOD_RELOAD_SENSORS);
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- FLOW_RATE_DEBUG_ENABLED=%d\n", FLOW_RATE_DEBUG_ENABLED);
	esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- PERIOD_FLOW_RATE_DEBUG=%d\n\n", PERIOD_FLOW_RATE_DEBUG);

	digitalWrite(LED_PIN, LOW);

	setupWebServer();
	server.begin();
	esp_log_write(ESP_LOG_INFO, "SKWS", "-- Started web server.\n");

	MDNS.addService("http", "tcp", 80);

	ArduinoOTA.setHostname("WateringSystem");
	ArduinoOTA.setPassword("Tr6spEh1");

	ArduinoOTA
	.onStart([]() {
	  String type;
	  if (ArduinoOTA.getCommand() == U_FLASH)
	  {
	    type = "sketch";
	  }
	  else // U_SPIFFS
	  {
	    type = "filesystem";
	  	SPIFFS.end();
	  }

	  // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
	  Serial.println("Start updating " + type);
	})
	.onEnd([]() {
	  Serial.println("\nEnd");
	})
	.onProgress([](unsigned int progress, unsigned int total) {
	  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	})
	.onError([](ota_error_t error) {
	  Serial.printf("Error[%u]: ", error);
	  if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
	  else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
	  else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
	  else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
	  else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});

	ArduinoOTA.begin();

	flowInterruptEnabled(true);
	esp_log_write(ESP_LOG_INFO, "SKWS", "-- Leaving setup.\n");
}


void flowInterruptEnabled(bool state)
{
	if(state)
	{
		attachInterrupt(FLOW_SENSOR_PIN, ISR_flowSensor, FALLING);
	}
	else
	{
		detachInterrupt(FLOW_SENSOR_PIN);
	}
}


void loop()
{
	ArduinoOTA.handle();
	PlantSystem_tick();
}



void setupWebServer(void)
{
	server.onNotFound([](AsyncWebServerRequest *request){
	  request->send(404);
	});

	// send a file when /index is requested
	server.on("/index.html", HTTP_ANY, [](AsyncWebServerRequest *request){
		request->send(200, "text/plain", "Plant System v0.1");
	});

	// send a file when / is requested
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
		request->send(200, "text/plain", "Plant System v0.1");
	});


	// Read log.txt
	server.on("/log.txt", HTTP_GET, [](AsyncWebServerRequest *request){
		if(SPIFFS.exists("/log.txt")) 
		{
		  Serial.println("Log.txt request received");
	      request->send(SPIFFS, "/log.txt", "text/plain");
		} 
		else 
		{
	      request->send(200, "text/plain", "log.txt not found.");
	    }
	});

	// Delete /log.txt
	server.on("/delete_log", HTTP_GET, [](AsyncWebServerRequest *request){
		Serial.println("Delete Log.txt request received");
		if(SPIFFS.exists("/log.txt")) 
		{
			File writeLog = SPIFFS.open("/log.txt", FILE_WRITE);
			if(!writeLog) Serial.println("Couldn't open log.txt"); 
			delay(50);
			LOG_Timestamp();
			esp_log_write(ESP_LOG_DEBUG, "SKWS", "-- Log wiped.\n");
			writeLog.close();
			request->send(200, "text/plain", "log.txt reset");
		} 
		else 
		{
	      request->send(200, "text/plain", "log.txt not found.");
	    }
	});


	// Get sensors on the bus
	server.on("/sensorData", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Sensor data");

		char buff[500] = {0};
		if(PlantSystem_WAPI_SensorData(buff, 500) == true)
		{
			request->send(200, "text/plain", buff);
			return;
		}
		else
		{
			Serial.println("PlantSystem_WAPI_SensorData() error");
			request->send(200, "text/plain", "PlantSystem_WAPI_SensorData() FAIL");
			return;
		}
	});

	// Get solenoids on the bus
	server.on("/solenoidData", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Solenoid data");

		char buff[500] = {0};
		if(PlantSystem_WAPI_SolenoidList(buff, 500) == true)
		{
			request->send(200, "text/plain", buff);
			return;
		}
		else
		{
			Serial.println("PlantSystem_WAPI_SolenoidList() error");
			request->send(200, "text/plain", "PlantSystem_WAPI_SolenoidList() FAIL");
			return;
		}
	});


	// Get devices on the bus
	server.on("/deviceData", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Device data");

		char buff[500] = {0};
		if(PlantSystem_WAPI_GetDevices(buff, 500) == true)
		{
			request->send(200, "text/plain", buff);
			return;
		}
		else
		{
			Serial.println("PlantSystem_WAPI_GetDevices() error");
			request->send(200, "text/plain", "PlantSystem_WAPI_GetDevices() FAIL");
			return;
		}
	});

	// Get number of flow sensor ticks for last watering action
	server.on("/flowSensor", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Flow sensor data");

		char buff[100] = {0};
		int len;
		len = snprintf(buff, 100, "{flowSensor:%d}", nFlowSensorCount_last);

		if(len>0)
		{
			request->send(200, "text/plain", buff);
			return;
		}
		else
		{
			Serial.println("PlantSystem_WAPI_GetDevices() error");
			request->send(200, "text/plain", "nFlowSensorCount FAIL");
			return;
		}
	});


	// Set solenoid state
	server.on("/setSolenoid", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Set solenoid to state");

		if ( request->hasParam("address") && request->hasParam("state") )
		{
			String xAddress;
			String xState;

			uint32_t nAddress 	= 0;
			uint32_t nState 	= 0;

			xAddress = request->getParam("address")->value();
			nAddress = xAddress.toInt();

			xState = request->getParam("state")->value();
			nState = xState.toInt();

			if(PlantSystem_SetSolenoidState(nAddress, nState) == true)
			{
				request->send(200, "text/plain", "OK");
				return;
			}
			else
			{
				Serial.println("PlantSystem_SetSolenoidState() error");
				request->send(200, "text/plain", "PlantSystem_SetSolenoidState() FAIL");
				return;
			}
		}
		else
		{
			request->send(200, "text/plain", "MISSING ARGUMENTS");
			return;
		}
	});

	// Water plant connected to solenoid with X ml of water
	server.on("/waterPlant", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Water plant request");

		if ( request->hasParam("address") && request->hasParam("volume") )
		{
			String xAddress;
			String xVolume;

			uint32_t nAddress 	= 0;
			uint32_t nVolume 	= 0;

			xAddress = request->getParam("address")->value();
			nAddress = xAddress.toInt();

			xVolume = request->getParam("volume")->value();
			nVolume = xVolume.toInt();

			if(PlantSystem_WaterPlant(nAddress, nVolume) == true)
			{
				char buff[200] = {0};
				int len = 0;

				len = snprintf(buff, 200, "{ \"result\": OK, \"Address\":%d, \"Volume\":%d }", 
															nAddress, nVolume);
				request->send(200, "text/plain", buff);
				UNUSED(len);
				return;
			}
			else
			{
				Serial.println("PlantSystem_WaterPlant() error");
				request->send(200, "text/plain", "PlantSystem_WaterPlant() FAIL");
				return;
			}
		}
		else
		{
			request->send(200, "text/plain", "MISSING ARGUMENTS");
			return;
		}
	});


	// Prime the system with water
	server.on("/PrimeSystemWithWater", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("PlantSystem_RequestPrimeSystem()");

		PlantSystem_RequestPrimeSystem();
		request->send(200, "text/plain", "OK");
		return;
	});

	// Calibrate flow sensor or read current value
	server.on("/calibrateFlowSensor", HTTP_GET, [] (AsyncWebServerRequest *request) {
		Serial.println("Calibrate edges per mL");

		char buff[200] = {0};

		if ( request->hasParam("edges") )
		{
			String xEdges;
			float fEdges = 0;

			xEdges = request->getParam("edges")->value();
			fEdges = xEdges.toFloat();

			// Update value
			fImpulsePerML = fEdges;
			EEPROM.writeFloat(0, fEdges);//EEPROM.put(address, param);
			EEPROM.commit();

			Serial.print("Changing fImpulsePerML to ");
			Serial.println((float)(fImpulsePerML),4);

			snprintf(buff, 200, "New value set to %f", fEdges);
			request->send(200, "text/plain", buff);
			return;
		}
		else
		{
			snprintf(buff, 200, "Current value is %f\r\nLast impulse count was: %d", fImpulsePerML, nFlowSensorCount_last);
			request->send(200, "text/plain", buff);
			return;
		}
	});
}


void LED_Blink(uint8_t nTimes, uint16_t n_delayPeriod)
{
	while(nTimes--)
	{
		digitalWrite(LED_PIN, HIGH);
		delay(n_delayPeriod);

		digitalWrite(LED_PIN, LOW);
		delay(n_delayPeriod);
	}
}

