#ifndef __adc_H
#define __adc_H

#include "main.h"

extern ADC_HandleTypeDef hadc;

void MX_ADC_Init(void);
uint32_t ADC_Measure(uint32_t channel);

#endif /*__ adc_H */

