
#include "DRIVER.h"

extern SPI_HandleTypeDef hspi1;

void SPI_WriteByte(uint8_t value)
{
    HAL_SPI_Transmit(&hspi1, &value, 1, 1000);
}

void SPI_Write_nByte(uint8_t *value, uint32_t len)
{
    HAL_SPI_Transmit(&hspi1, value, len, 1000);
}

void EPAPER_GPIO_Init(void)
{
    HAL_GPIO_WritePin(EPAPER_DC_PORT, EPAPER_DC_PIN, 0);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 0);
    HAL_GPIO_WritePin(EPAPER_RST_PORT, EPAPER_RST_PIN, 1);
}

void EPAPER_GPIO_Exit(void)
{
    HAL_GPIO_WritePin(EPAPER_DC_PORT, EPAPER_DC_PIN, 0);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 0);
    HAL_GPIO_WritePin(EPAPER_RST_PORT, EPAPER_RST_PIN, 0);
}

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
void EPAPER_Reset(void)
{
    HAL_GPIO_WritePin(EPAPER_RST_PORT, EPAPER_RST_PIN, 1);
    HAL_Delay(200);
    HAL_GPIO_WritePin(EPAPER_RST_PORT, EPAPER_RST_PIN, 0);
    HAL_Delay(2);
    HAL_GPIO_WritePin(EPAPER_RST_PORT, EPAPER_RST_PIN, 1);
    HAL_Delay(200);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
void EPAPER_SendCommand(uint8_t Reg)
{
    HAL_GPIO_WritePin(EPAPER_DC_PORT, EPAPER_DC_PIN, 0);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 0);
    SPI_WriteByte(Reg);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
void EPAPER_SendData(uint8_t Data)
{
    HAL_GPIO_WritePin(EPAPER_DC_PORT, EPAPER_DC_PIN, 1);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 0);
    SPI_WriteByte(Data);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 1);
}

/******************************************************************************
function :	send multiple data
parameter:
    Data : Write data
    len  : data length

/!\ not working on epaper, each data need to be separate by a CS pulse
******************************************************************************/
void EPAPER_Send_nData(uint8_t *Data, uint32_t len)
{
    HAL_GPIO_WritePin(EPAPER_DC_PORT, EPAPER_DC_PIN, 1);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 0);
    SPI_Write_nByte(Data, len);
    HAL_GPIO_WritePin(EPAPER_CS_PORT, EPAPER_CS_PIN, 1);
}

/******************************************************************************
function :	Read Busy
parameter:
******************************************************************************/
void EPAPER_ReadBusy(void)
{
    while (!HAL_GPIO_ReadPin(EPAPER_BUSY_PORT, EPAPER_BUSY_PIN))
    {
        HAL_Delay(20);
    }
    HAL_Delay(200);
}
