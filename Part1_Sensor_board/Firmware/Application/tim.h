#ifndef __tim_H
#define __tim_H

#include "main.h"

extern TIM_HandleTypeDef htim2;

void MX_TIM2_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#endif /*__ tim_H */

