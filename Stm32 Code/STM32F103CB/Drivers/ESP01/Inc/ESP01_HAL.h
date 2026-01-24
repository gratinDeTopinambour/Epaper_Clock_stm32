/*
 * ESP01_HAL.h
 *
 *  Created on: Jan 14, 2026
 *      Author: valentin
 */

#ifndef ESP01_HAL_H_
#define ESP01_HAL_H_

#include "main.h"    // HAL, UART, etc.
#include <stdbool.h> // Types booléens
#include <stddef.h>  // Types de taille (size_t, etc.)
#include <stdint.h>  // Types entiers standard (uint8_t, uint16_t, etc.)
#include <string.h>  // Pour strlen, strcpy dans les fonctions inline

#define ESP01_DEBUG 0 //log de debug

#define ESP01_TIMEOUT 2000

extern uint8_t dma_rx_buf[1024]; // Buffer DMA pour la réception ESP01
extern UART_HandleTypeDef *wifi_uart;

//#ifdef UART_PC
//extern UART_HandleTypeDef *pc_uart;
//#endif //UART_PC

//void Send_To_PC(const char *msg);
int Get_New_Data(uint8_t *buf, uint16_t bufsize);
int Send_ATCMD_DMA(const char *cmd, char *response_buffer, size_t response_buf_size, const char *expected, uint32_t timeout_ms);
int Read_DMA_Until_Pattern(const char *pattern, char *resp_buf, size_t bufsize, uint32_t timeout_ms);
void Flush_Rx_Buffer(uint32_t timeout_ms);
int Init_Wifi(const char *ssid, const char *passwd);
int Get_Date(uint8_t *day, uint8_t *dd, uint8_t *mm, uint16_t *yy, uint16_t *minute);
uint8_t day_from_str(const char *day);
uint8_t month_from_str(const char *month);
int Date_from_HTTP(const char *trame, char *date_buf, size_t date_buf_size);

#endif /* ESP01_INC_ESP01_HAL_H_ */
