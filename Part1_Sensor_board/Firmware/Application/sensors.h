#ifndef __SENSORS_H
#define __SENSORS_H

#include "main.h"

void Sensor_TakeMeasurement(void);
void Sensors_PrintDebug(void);
void MCP9800_readTemp(int8_t *deg, uint8_t *frac);


void Sensors_GetTemperatureReading(uint8_t *deg, uint8_t *frac);
void Sensors_GetSoilMoistureReading(uint8_t *b3, uint8_t *b2, uint8_t *b1, uint8_t *b0);
void Sensors_GetAmbientLightReading(uint8_t *b3, uint8_t *b2, uint8_t *b1, uint8_t *b0);

#endif