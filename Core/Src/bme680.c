/*
 * bme680.c
 *
 *  Created on: Nov 1, 2024
 *      Author: andre
 */

#include "bme680.h"
#include "i2c.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;

uint8_t bme680_address = 0;  // Define the global address variable

HAL_StatusTypeDef BME680_Init(void) {
    uint8_t id;
    HAL_StatusTypeDef ret;

    // Add delay to ensure the sensor has time to power up and stabilize
    HAL_Delay(1000); // Increased delay to 1 second for stability

    // Soft Reset the BME680
    uint8_t reset_cmd = 0xB6;
    HAL_I2C_Mem_Write(&hi2c1, (0x76 << 1), 0xE0, I2C_MEMADD_SIZE_8BIT, &reset_cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(100); // Wait for reset to complete

    // I2C Scanner to detect the sensor
    HAL_UART_Transmit(&huart2, (uint8_t *)"Starting I2C Scanner...\n", 24, HAL_MAX_DELAY);
    for (uint8_t i = 1; i < 128; i++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (i << 1), 3, 100) == HAL_OK) {
            char scanner_msg[50];
            snprintf(scanner_msg, sizeof(scanner_msg), "Device found at address: 0x%02X\n", i);
            HAL_UART_Transmit(&huart2, (uint8_t *)scanner_msg, strlen(scanner_msg), HAL_MAX_DELAY);
        }
    }

    // Try different I2C addresses (0x76 or 0x77)
    for (int i = 0; i < 2; i++) {
        bme680_address = (i == 0) ? (0x76 << 1) : (0x77 << 1);
        char addr_msg[50];
        snprintf(addr_msg, sizeof(addr_msg), "Trying address: 0x%02X\n", bme680_address >> 1);
        HAL_UART_Transmit(&huart2, (uint8_t *)addr_msg, strlen(addr_msg), HAL_MAX_DELAY);

        // Attempt to read sensor ID multiple times (e.g., 3 attempts)
        for (int attempt = 0; attempt < 3; attempt++) {
            HAL_UART_Transmit(&huart2, (uint8_t *)"Attempting to read sensor ID...\n", 32, HAL_MAX_DELAY);

            ret = HAL_I2C_Mem_Read(&hi2c1, bme680_address, BME680_REG_ID, I2C_MEMADD_SIZE_8BIT, &id, 1, 500);

            if (ret == HAL_OK && id == 0x61) {
                HAL_UART_Transmit(&huart2, (uint8_t *)"BME680 Initialized Successfully\n", 33, HAL_MAX_DELAY);
                return HAL_OK;
            } else {
                uint32_t error = HAL_I2C_GetError(&hi2c1);
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg), "Failed to read sensor ID (attempt: %d, ret: %d, I2C Error: 0x%08lX)\n", attempt + 1, ret, error);
                HAL_UART_Transmit(&huart2, (uint8_t *)error_msg, strlen(error_msg), HAL_MAX_DELAY);
            }

            HAL_Delay(100); // Short delay between attempts
        }
    }

    // If no successful response, return error
    HAL_UART_Transmit(&huart2, (uint8_t *)"BME680 Initialization Failed\n", 29, HAL_MAX_DELAY);
    return HAL_ERROR;
}



// Function to read temperature from BME680 sensor
HAL_StatusTypeDef BME680_ReadTemperature(float *temperature) {
    HAL_StatusTypeDef ret;
    uint8_t rawData[3];

    // Set sensor to forced mode to take a measurement
    uint8_t ctrl_meas = (0x02 << 5) | (0x05 << 2) | 0x01; // Temp x2, Pressure x16, Forced mode
    ret = HAL_I2C_Mem_Write(&hi2c1, BME680_ADDRESS, BME680_REG_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &ctrl_meas, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return HAL_ERROR;
    }

    // Wait for the measurement to complete (typical measurement time)
    HAL_Delay(100);

    // Read the temperature registers (3 bytes)
    ret = HAL_I2C_Mem_Read(&hi2c1, BME680_ADDRESS, BME680_REG_TEMP_MSB, I2C_MEMADD_SIZE_8BIT, rawData, 3, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return HAL_ERROR;
    }

    // Convert the raw data to temperature (using datasheet formulas)
    int32_t temp_raw = (int32_t)((rawData[0] << 12) | (rawData[1] << 4) | (rawData[2] >> 4));
    *temperature = (float)temp_raw / 100.0f; // Adjust based on BME680 datasheet conversion

    return HAL_OK;
}

// Function to read pressure from BME680 sensor
HAL_StatusTypeDef BME680_ReadPressure(float *pressure) {
    return HAL_ERROR; // Not implemented yet
}

// Function to read humidity from BME680 sensor
HAL_StatusTypeDef BME680_ReadHumidity(float *humidity) {
    return HAL_ERROR; // Not implemented yet
}

