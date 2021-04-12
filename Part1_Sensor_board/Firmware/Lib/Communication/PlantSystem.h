#ifndef __PLANTSYSTEM_COMMS_H
#define __PLANTSYSTEM_COMMS_H

#include <stdbool.h>

// CMD (1byte) + DATATYPE (1byte) + DATALEN (2bytes) 
#define PLANTSYSTEM_MIN_HEADER_LENGHT	4

typedef enum 
{
	CMD_FIRST = 0x00,
	CMD_GET_DEVICE_INFO,
	CMD_GET_TEMPERATURE,
	CMD_GET_SOILMOISTURE,
	CMD_GET_AMBIENTLIGHT,
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