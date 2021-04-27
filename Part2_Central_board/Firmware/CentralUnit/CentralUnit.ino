#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <esp_task_wdt.h>
#include "CentralUnit_cfg.h"
#include "wifi_credentials.h"
#include "PlantSystem.h"
#include "Wire.h"

#define WDT_TIMEOUT		20000

AsyncWebServer server(80);
int WiFi_status = WL_IDLE_STATUS; 
volatile uint32_t nFlowSensorCount = 0;
volatile uint32_t nFlowSensorCount_last = 0;

void IRAM_ATTR ISR_flowSensor() {
    nFlowSensorCount++;
}


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

	Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

	// Configure WDT
	Serial.println("Configuring WDT...");
	esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
	esp_task_wdt_add(NULL); //add current thread to WDT watch

	// Connect to WiFi
	while ( WiFi_status != WL_CONNECTED) {
		Serial.print("Connecting to SSID: ");
		Serial.println(ssid);
		WiFi_status = WiFi.begin(ssid, password);

		// wait 5 seconds for connection:
		esp_task_wdt_reset();
		delay(5000);
	}

	Serial.print("WiFi IP: ");
	Serial.println(WiFi.localIP());

	digitalWrite(LED_PIN, LOW);

	setupWebServer();
	server.begin();

	attachInterrupt(FLOW_SENSOR_PIN, ISR_flowSensor, FALLING);
}


void loop()
{
	while(1)
	{
		PlantSystem_tick();
	}
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

	// Get devices on the bus
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


	// Get devices on the bus
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
				int len;

				len = snprintf(buff, 200, "{ \"result\": OK, \"Address\":%d, \"Volume\":%d }", 
															nAddress, nVolume);
				request->send(200, "text/plain", buff);
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
}

