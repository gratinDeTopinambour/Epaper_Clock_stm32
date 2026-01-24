/*
 * bme280.h
 *
 *  Created on: Jan 18, 2026
 *      Author: valentin
 *
 *  Header file for Bosch BME280 sensor driver
 *  Provides register definitions, configuration macros,
 *  and public API function prototypes
 */

#ifndef BME280_INC_BME280_H_
#define BME280_INC_BME280_H_

#include "main.h"

/* =========================================================
 * BME280 Identification Registers
 * ========================================================= */

/* Chip ID register (should read 0x60 for BME280) */
#define BME_ID 0xD0
#define BME_ID_value 0x60

/* =========================================================
 * Reset Register
 * ========================================================= */
#define BME_RESET 0xE0
#define BME_RESET_cmd 0xB6

/* =========================================================
 * Calibration Data Registers
 * ========================================================= */

/* Temperature calibration (T1, T2, T3) */
#define BME_T1_LSB 0x88

/* Pressure calibration (P1 → P9) */
#define BME_P1_LSB 0x8E


/* Humidity calibration */
#define BME_H1 0xA1 // H1 register
#define BME_H2_LSB 0xE1 // H2 → H6 registers start

/* =========================================================
 * Control and Status Registers
 * ========================================================= */
#define BME_CTRL_HUM 0xF2
#define BME_STATUS 0xF3
#define BME_CTRL_MEAS 0xF4
#define BME_CONFIG 0xF5

/* =========================================================
 * Measurement Data Registers
 * ========================================================= */

/* Pressure data (20-bit) */
#define BME_PRESS_MSB 0xF7
#define BME_PRESS_LSB 0xF8
#define BME_PRESS_XLSB 0xF9

/* Temperature data (20-bit) */
#define BME_TEMP_MSB 0xFA
#define BME_TEMP_LSB 0xFB
#define BME_TEMP_XLSB 0xFC

/* Humidity data (16-bit) */
#define BME_HUM_MSB 0xFD
#define BME_HUM_LSB 0xFE

/* =========================================================
 * Oversampling Settings
 * Used for temperature, pressure, and humidity
 * ========================================================= */
#define BME_OVERSAMPLING_1 0b001
#define BME_OVERSAMPLING_2 0b010
#define BME_OVERSAMPLING_4 0b011
#define BME_OVERSAMPLING_8 0b100
#define BME_OVERSAMPLING_16 0b101

/* =========================================================
 * Sensor Operating Modes
 * ========================================================= */
#define BME_MODE_SLEEP 0b00
#define BME_MODE_FORCED 0b01
#define BME_MODE_NORMAL 0b11

/* =========================================================
 * Public API Function Prototypes
 * ========================================================= */

/**
 * @brief Write multiple bytes to BME280 via I2C
 * @param buffer Pointer to data buffer (first byte = register address)
 * @param len    Number of bytes to transmit
 */
void I2C_Write_nByte(uint8_t *buffer, uint32_t len);

/**
 * @brief Read multiple bytes from BME280 via I2C
 * @param addr   Register address to read from
 * @param buffer Pointer to receive buffer
 * @param len    Number of bytes to read
 */
void I2C_Read_nByte(uint8_t addr, uint8_t *buffer, uint32_t len);

/**
 * @brief Initialize BME280 sensor
 *        - Resets the device
 *        - Verifies chip ID
 *        - Reads calibration coefficients
 * @retval 0  Success
 * @retval -1 Device ID mismatch
 */
int BME_Init(void);

/**
 * @brief Read compensated temperature, pressure and humidity
 * @param temp  Pointer to temperature in °C
 * @param press Pointer to pressure in hPa
 * @param hum   Pointer to relative humidity in %
 */
void BME_Read_Data(int *temp, uint32_t *press, uint32_t *hum);



#endif /* BME280_INC_BME280_H_ */
