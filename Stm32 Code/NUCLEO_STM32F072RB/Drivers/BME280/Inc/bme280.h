/*
 * bme280.h
 *
 *  Created on: Jan 18, 2026
 *      Author: valentin
 */

#ifndef BME280_INC_BME280_H_
#define BME280_INC_BME280_H_

#include "main.h"

#define BME_ID 0xD0
#define BME_ID_value 0x60

#define BME_RESET 0xE0
#define BME_RESET_cmd 0xB6

#define BME_T1_LSB 0x88
#define BME_P1_LSB 0x8E
#define BME_H1 0xA1
#define BME_H2_LSB 0xE1

#define BME_CTRL_HUM 0xF2
#define BME_STATUS 0xF3
#define BME_CTRL_MEAS 0xF4
#define BME_CONFIG 0xF5
#define BME_PRESS_MSB 0xF7
#define BME_PRESS_LSB 0xF8
#define BME_PRESS_XLSB 0xF9
#define BME_TEMP_MSB 0xFA
#define BME_TEMP_LSB 0xFB
#define BME_TEMP_XLSB 0xFC
#define BME_HUM_MSB 0xFD
#define BME_HUM_LSB 0xFE

#define BME_OVERSAMPLING_1 0b001
#define BME_OVERSAMPLING_2 0b010
#define BME_OVERSAMPLING_4 0b011
#define BME_OVERSAMPLING_8 0b100
#define BME_OVERSAMPLING_16 0b101

#define BME_MODE_SLEEP 0b00
#define BME_MODE_FORCED 0b01
#define BME_MODE_NORMAL 0b11

void I2C_Write_nByte(uint8_t *buffer, uint32_t len);
void I2C_Read_nByte(uint8_t addr, uint8_t *buffer, uint32_t len);
int BME_Init();
void BME_Read_Data(int *temp, uint32_t *press, uint32_t *hum);



#endif /* BME280_INC_BME280_H_ */
