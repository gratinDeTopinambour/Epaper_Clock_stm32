

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <stdio.h>

/**
 * e-Paper GPIO
 **/
#define EPAPER_RST_PIN RST_Pin
#define EPAPER_RST_PORT RST_GPIO_Port

#define EPAPER_DC_PIN DC_Pin
#define EPAPER_DC_PORT DC_GPIO_Port

#define EPAPER_CS_PIN SPI_CS_Pin
#define EPAPER_CS_PORT SPI_CS_GPIO_Port

#define EPAPER_BUSY_PIN BUSY_Pin
#define EPAPER_BUSY_PORT BUSY_GPIO_Port

#define EPAPER_MOSI_PIN DIN_Pin
#define EPAPER_MOSI_PORT DIN_GPIO_Port

#define EPAPER_SCLK_PIN SCK_Pin
#define EPAPER_SCLK_PORT SCK_GPIO_Port

void SPI_WriteByte(uint8_t value);
void SPI_Write_nByte(uint8_t *value, uint32_t len);

void EPAPER_GPIO_Init(void);
void EPAPER_GPIO_Exit(void);

void EPAPER_Reset(void);
void EPAPER_SendCommand(uint8_t Reg);
void EPAPER_SendData(uint8_t Data);
void EPAPER_Send_nData(uint8_t *pData, uint32_t Len);
void EPAPER_ReadBusy(void);
#endif
