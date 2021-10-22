#ifndef __CENTRAL_UNIT_CFG
#define __CENTRAL_UNIT_CFG

#define LED_PIN					2

#define I2C_SDA_PIN				21
#define I2C_SCL_PIN				22
#define I2C_EN_PIN				19

#define WATER_PUMP_EN_PIN		12
#define FLOW_SENSOR_PIN			26

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

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

// I2C bus config
#define I2C_MAX_SENSORS_ON_BUS		20
#define I2C_MAX_SOLENOIDS_ON_BUS	20
#define I2C_MAX_DEVICES_ON_BUS		(I2C_MAX_SENSORS_ON_BUS + I2C_MAX_SOLENOIDS_ON_BUS)
#define I2C_ADDRESS_LIMIT_LOWER		0x08
#define I2C_ADDRESS_LIMIT_UPPER		0x7F

// Plant watering definitions
#define WATERING_VOLUME_MINIMUM		10
#define WATERING_VOLUME_MAXIMUM		550
#define WATERING_FLOW_EDGES_PER_mL	4

// Sanity checks
#if (WATERING_VOLUME_MINIMUM <= 0)
#error "WATERING_VOLUME_MINIMUM has to be non-zero positive (unsigned int) value!"
#endif
#if (WATERING_VOLUME_MAXIMUM <= 0)
#error "WATERING_VOLUME_MAXIMUM has to be non-zero positive (unsigned int) value!"
#endif
#if (WATERING_FLOW_EDGES_PER_mL <= 0)
#error "WATERING_FLOW_EDGES_PER_mL has to be non-zero positive (float) value!"
#endif

// PlantSystem_tick periods and debug
#define I2C_BUS_RESCAN_ENABLED		0				// Define to enable I2C bus periodic rescan
#define PERIOD_RESCAN_BUS			24*3600*1000	// Every PERIOD_RESCAN_BUS miliseconds perform I2C bus rescan
#define PERIOD_RELOAD_SENSORS		15*1000			// Every PERIOD_RELOAD_SENSORS miliseconds poll data from sensors
#define FLOW_RATE_DEBUG_ENABLED		0				// Define to enable periodic flow rate debug serial printouts
#define PERIOD_FLOW_RATE_DEBUG		2*1000			// Every PERIOD_FLOW_RATE_DEBUG print out flow rate values (used for debug)

#endif