#ifndef __PLANTSYSTEM_COMMS_H
#define __PLANTSYSTEM_COMMS_H

#include <stdbool.h>

#ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#else 
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else // ESP32 Before IDF 4.0
#include "rom/rtc.h"
#endif

// CMD (1byte) + DATATYPE (1byte) + DATALEN (2bytes) 
#define PLANTSYSTEM_MIN_HEADER_LENGHT	4

typedef enum 
{
	CMD_FIRST = 0x00,
	CMD_GET_DEVICE_INFO,
	CMD_GET_TEMPERATURE,
	CMD_GET_SOILMOISTURE,
	CMD_GET_AMBIENTLIGHT,
	CMD_GET_SOLENOID_STATE,
	CMD_SET_SOLENOID_STATE,
	CMD_LAST
} plant_cmd_t;


typedef enum 
{
	DATATYPE_FIRST = 0x00,
	DATATYPE_NONE,
	DATATYPE_SINGLE_VALUE,
	DATATYPE_KEY_VALUE_PAIR,
	DATATYPE_LAST
} plant_dataType_t;



typedef enum 
{
	DEVICETYPE_FIRST = 0x00,
	DEVICETYPE_SENSOR_V1,
	DEVICETYPE_SOLENOID_V1,
	DEVICETYPE_LAST
} plant_deviceType_t;

bool PlanSystem_ParseBuffer(uint8_t *pBuffer, uint8_t nBufferLen, uint8_t *cmd, uint8_t *dataType, uint8_t *dataLength);

#endif