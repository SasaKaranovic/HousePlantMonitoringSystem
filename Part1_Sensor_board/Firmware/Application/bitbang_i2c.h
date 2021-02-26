#ifndef __BBI2C_H
#define __BBI2C_H

#include "main.h"


#define BBI2C_SDA_PORT  GPIOB
#define BBI2C_SDA_PIN   GPIO_PIN_4

#define BBI2C_SCL_PORT  GPIOB
#define BBI2C_SCL_PIN   GPIO_PIN_5



void bbi2c_init(void);

#endif
