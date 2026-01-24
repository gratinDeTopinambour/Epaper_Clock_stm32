/*
 * bme280.c
 *
 *  Created on: Jan 18, 2026
 *      Author: valentin
 *
 *  Driver for Bosch BME280 sensor (Temperature, Pressure, Humidity)
 *  Communication via I2C using STM32 HAL
 */

#include "bme280.h"
#include "stdio.h"

/* ========= Calibration coefficients from BME280 ========== */
/* These values are read once from the sensor and used for
 * temperature, pressure and humidity compensation formulas
 * (as defined in the Bosch datasheet)
 */

/* Temperature calibration */
uint16_t T1;
int16_t T2;
int16_t T3;

/* Pressure calibration */
uint16_t P1;
int16_t P2;
int16_t P3;
int16_t P4;
int16_t P5;
int16_t P6;
int16_t P7;
int16_t P8;
int16_t P9;

/* Humidity calibration */
uint8_t H1;
int16_t H2;
uint8_t H3;
int16_t H4;
int16_t H5;
int8_t H6;


/* Fine temperature value used internally by Bosch formulas
 * Must be computed during temperature compensation and reused
 * for pressure and humidity calculations
 */
int t_fine;

/* ========= BME280 I2C Address =========
 * 0x76 if SDO pin is connected to GND
 * 0x77 if SDO pin is connected to VDD
 */
#define BME_ADDR 0x76

extern I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef *bme_i2c = &hi2c1;

static int BME_Comp(void);
static int BME_temp_comp(int adc_T);
static uint32_t BME_press_comp(int adc_P);
static uint32_t BME_hum_comp(int adc_H);


/* =========================================================
 * I2C write helper
 * Sends "len" bytes from buffer to the BME280
 * ========================================================= */
void I2C_Write_nByte(uint8_t *buffer, uint32_t len)
{
	uint16_t l_shift_addr = BME_ADDR<<1; // HAL expects 8-bit address
	HAL_I2C_Master_Transmit(bme_i2c, l_shift_addr, buffer, len, 1000);
	HAL_Delay(100);
}

/* =========================================================
 * I2C read helper
 * Writes register address, then reads "len" bytes
 * ========================================================= */
void I2C_Read_nByte(uint8_t addr, uint8_t *buffer, uint32_t len)
{
	uint16_t l_shift_addr = BME_ADDR<<1; // HAL expects 8-bit address

	/* Send register address */
	HAL_I2C_Master_Transmit(bme_i2c, l_shift_addr, &addr, 1, 1000);
	HAL_Delay(100);

	/* Read data */
	HAL_I2C_Master_Receive(bme_i2c, l_shift_addr, buffer, len, 1000);
	HAL_Delay(100);
}


/* =========================================================
 * Initialize BME280
 * - Reset sensor
 * - Check chip ID
 * - Read calibration coefficients
 * ========================================================= */
int BME_Init()
{


	/* Reset the sensor */
	uint8_t cmd[2] = {BME_RESET, BME_RESET_cmd};
	I2C_Write_nByte(cmd,2);

	/* Read and verify chip ID */
	uint8_t id;
	I2C_Read_nByte(BME_ID, &id,1);
	if(id != BME_ID_value) return -1; /* check BME id is correct*/



	/* Read calibration data from sensor */
	BME_Comp();

	return 0;
}


/* =========================================================
 * Read temperature, pressure and humidity
 * Output:
 *  temp  -> temperature in °C
 *  press -> pressure in hPa
 *  hum   -> relative humidity in %
 * ========================================================= */
void BME_Read_Data(int *temp, uint32_t *press, uint32_t *hum)
{

	uint8_t cmd[2];
	uint8_t reg_value;

	/* Configure humidity oversampling */
	reg_value = 0 | BME_OVERSAMPLING_1;
	cmd[0] = BME_CTRL_HUM;
	cmd[1] = reg_value;
	I2C_Write_nByte(cmd,2);

	/* Configure temperature & pressure oversampling + forced mode */
	reg_value = 0 | (BME_OVERSAMPLING_1<<5) | (BME_OVERSAMPLING_1<<2) | BME_MODE_FORCED;
	cmd[0] = BME_CTRL_MEAS;
	cmd[1] = reg_value;
	I2C_Write_nByte(cmd,2);

	/* Read raw measurement data */
	uint8_t bme_data[8];
	I2C_Read_nByte(BME_PRESS_MSB, bme_data,8);

	/* Assemble raw ADC values (20-bit for T & P, 16-bit for H) */
	int adc_P = bme_data[0]<<12 | bme_data[1]<<4 | bme_data[2]>>4;
	int adc_T = bme_data[3]<<12 | bme_data[4]<<4 | bme_data[5]>>4;
	int adc_H = bme_data[6]<<8 | bme_data[7];

	/* Apply compensation algorithms */
	*temp = BME_temp_comp(adc_T);
	*press = BME_press_comp(adc_P);
	*hum = BME_hum_comp(adc_H);

}


/* =========================================================
 * Read calibration coefficients from BME280
 * Must be called once after reset
 * ========================================================= */
static int BME_Comp()
{


	/* ---- Temperature calibration (0x88 -> 0x8D) ---- */
	uint8_t bme_temp_comp[6];
	I2C_Read_nByte(BME_T1_LSB, bme_temp_comp,6);

	T1 = bme_temp_comp[0] | bme_temp_comp[1]<<8;
	T2 = bme_temp_comp[2] | bme_temp_comp[3]<<8;
	T3 = bme_temp_comp[4] | bme_temp_comp[5]<<8;

	/* ---- Pressure calibration (0x8E -> 0x9F) ---- */
	uint8_t bme_press_comp[18];
	I2C_Read_nByte(BME_P1_LSB, bme_press_comp,18);

	P1 = bme_press_comp[0] | bme_press_comp[1]<<8;
	P2 = bme_press_comp[2] | bme_press_comp[3]<<8;
	P3 = bme_press_comp[4] | bme_press_comp[5]<<8;
	P4 = bme_press_comp[6] | bme_press_comp[7]<<8;
	P5 = bme_press_comp[8] | bme_press_comp[9]<<8;
	P6 = bme_press_comp[10] | bme_press_comp[11]<<8;
	P7 = bme_press_comp[12] | bme_press_comp[13]<<8;
	P8 = bme_press_comp[14] | bme_press_comp[15]<<8;
	P9 = bme_press_comp[16] | bme_press_comp[17]<<8;

	/* ---- Humidity calibration ---- */
	uint8_t bme_hum_comp[7];
	I2C_Read_nByte(BME_H1, &H1,1); //0xA1
	I2C_Read_nByte(BME_H2_LSB, bme_hum_comp,7); //0xE1 -> 0xE7

	H2 = bme_hum_comp[0] | bme_hum_comp[1]<<8;
	H3 = bme_hum_comp[2];
	H4 = bme_hum_comp[3]<<4 | (bme_hum_comp[4] & 0x0F); //0xE4/0xE5[3:0] -> H4[11:4]/[3:0]
	H5 = bme_hum_comp[5]<<4 | (bme_hum_comp[4]>>4); //0xE5[7:4]/0xE6 -> H5[3:0]/[11:4]
	H6 = bme_hum_comp[6];

	return 0;
}

/* =========================================================
 * Temperature compensation
 * Returns temperature in °C (185 = 18.5°C)
 * ========================================================= */
static int BME_temp_comp(int adc_T)
{
	int var1, var2, T;
	var1 = (((adc_T>>3) - ((int)T1<<1))*((int)T2))>>11;
	var2 = (((((adc_T>>4) - ((int)T1))*((adc_T>>4) - ((int)T1)))>>12) * ((int)T3))>>14;
	t_fine = var1 + var2;
	T = (t_fine*5 + 128) >> 8;
	T = T/10; //temperature in °C 185 = 18.5°C

	return T;
}

/* =========================================================
 * Pressure compensation
 * Returns pressure in hPa
 * ========================================================= */
static uint32_t BME_press_comp(int adc_P)
{
	int64_t var1, var2, p;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1*var1*(int64_t)P6;
	var2 = var2 + ((var1*(int64_t)P5)<<17);
	var2 = var2 + (((int64_t)P4)<<35);
	var1 = ((var1*var1*(int64_t)P3)>>8) + ((var1*(int64_t)P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)P1)>>33;
	if(var1 == 0)
	{
		return 0; // avoid division by zero
	}
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)P9)*(p>>13)*(p>>13))>>25;
	var2 = (((int64_t)P8)*p)>>19;
	p = ((p + var1 + var2)>>8) + (((int64_t)P7)<<4);

	p = p/256;
	p = p/100; //pression in hPa
	return (uint32_t)p;
}

/* =========================================================
 * Humidity compensation
 * Returns relative humidity in %
 * ========================================================= */
static uint32_t BME_hum_comp(int adc_H)
{
	int H;
	H = (t_fine - ((int)76800));
	H = (((((adc_H<<14) - (((int)H4)<<20) - (((int)H5) * H)) + ((int)16384))>>15) *
			(((((((H * ((int)H6))>>10) * (((H * ((int)H3))>>11) + ((int)32768)))>>10) +
					((int)2097152)) * ((int)H2) + 8192)>>14));
	H = (H - (((((H>>15) * (H>>15))>>7) * ((int)H1))>>4));
	H = (H < 0 ? 0 : H);
	H = (H > 419430400 ? 419430400 : H);
	H = H>>12;
	H = H/1024;
	return (uint32_t)H;
}
