/*
 * bme680.h
 *
 *  Created on: Nov 1, 2024
 *      Author: andre
 */

#ifndef INC_BME680_H_
#define INC_BME680_H_

#include "stm32f3xx_hal.h"

// BME680 I2C address
#define BME680_ADDRESS 0x76 << 1

// Register definitions for BME680
extern uint8_t bme680_address;
#define BME680_REG_ID            0xD0
#define BME680_REG_CTRL_HUM      0x72
#define BME680_REG_CTRL_MEAS     0x74
#define BME680_REG_TEMP_MSB      0x22

// Function declarations
HAL_StatusTypeDef BME680_Init(void);
HAL_StatusTypeDef BME680_ReadTemperature(float *temperature);
HAL_StatusTypeDef BME680_ReadPressure(float *pressure);
HAL_StatusTypeDef BME680_ReadHumidity(float *humidity);

#endif /* INC_BME680_H_ */

