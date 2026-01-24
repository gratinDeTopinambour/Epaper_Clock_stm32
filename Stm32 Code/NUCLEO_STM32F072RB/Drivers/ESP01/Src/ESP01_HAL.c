/*
 * ESP01_HAL.c
 *
 *  Created on: Jan 14, 2026
 *      Author: valentin
 *
 *  Description:
 *  Hardware Abstraction Layer for ESP-01 (ESP8266) module.
 *  Communication is done using UART + DMA (circular buffer).
 *  This file provides:
 *    - AT command handling
 *    - DMA RX parsing
 *    - WiFi initialization
 *    - Date retrieval via HTTP
 */

#include "ESP01_HAL.h"
#include "stdio.h"
#include "string.h"

/* ==============================
 * UART HANDLES
 * ============================== */

/* UART used to communicate with ESP-01 */
extern UART_HandleTypeDef huart1;
UART_HandleTypeDef *wifi_uart = &huart1;

/* UART used for debugging (PC terminal) */
//extern UART_HandleTypeDef huart2;
//UART_HandleTypeDef *pc_uart   = &huart2;

/* ==============================
 * DMA RX BUFFER
 * ============================== */

/* Circular DMA RX buffer (defined elsewhere) */
extern uint8_t dma_rx_buf[1024];

uint16_t dma_buf_size = 1024;          // DMA RX buffer size
volatile uint16_t rx_last_pos = 0;     // Last read position in DMA buffer


/**
 * @brief  Send a debug message to the PC UART.
 *         The message is transmitted in small chunks to avoid UART saturation.
 * @param  msg Null-terminated string to send.
 * @retval None.
 */
//void Send_To_PC(const char *msg)
//{
//    size_t len = strlen(msg);
//    size_t chunk_size = 128;
//    size_t sent = 0;
//
//    while (sent < len)
//    {
//        size_t to_send = (len - sent > chunk_size) ? chunk_size : (len - sent);
//        HAL_UART_Transmit(pc_uart, (uint8_t *)(msg + sent), to_send, HAL_MAX_DELAY);
//        sent += to_send;
//    }
//}


/**
 * @brief  Send an AT command to the ESP01 and wait for a specific response.
 *         Reception is handled using DMA with a circular buffer.
 * @param  cmd               AT command to send (without CRLF).
 * @param  response_buffer   Buffer to store the received response.
 * @param  response_buf_size Size of the response buffer.
 * @param  expected          Expected substring to validate success.
 * @param  timeout_ms        Timeout in milliseconds.
 *
 * @retval 0  Success (expected pattern found)
 * @retval -1 Failure (timeout or expected pattern not found)
 */
int Send_ATCMD_DMA(const char *cmd, char *response_buffer, size_t response_buf_size, const char *expected, uint32_t timeout_ms)
{
	/* Clear pending RX data before sending a new command */
	Flush_Rx_Buffer(10);

#if ESP01_DEBUG
	Send_To_PC(cmd);
	Send_To_PC("\r\n");
#endif //ESP01_DEBUG

	/* Send command and CRLF */
	HAL_UART_Transmit(wifi_uart, (uint8_t *)cmd, strlen(cmd), HAL_MAX_DELAY);
	HAL_UART_Transmit(wifi_uart, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

	uint32_t start = HAL_GetTick();
	size_t resp_len = 0;
	response_buffer[0] = '\0';

	/* Read incoming data until timeout or expected pattern is found */
    while ((HAL_GetTick() - start) < timeout_ms && resp_len < response_buf_size - 1)
    {
        uint8_t buf[64];
        int len = Get_New_Data(buf, sizeof(buf));
        if (len > 0)
        {
            if (resp_len + len >= response_buf_size - 1)
                len = response_buf_size - 1 - resp_len;
            memcpy(response_buffer + resp_len, buf, len);
            resp_len += len;
            response_buffer[resp_len] = '\0';

            if (expected && strstr(response_buffer, expected))
                break;
        }
        else
        {
            HAL_Delay(1);
        }
    }

#if ESP01_DEBUG
    Send_To_PC(response_buffer);
#endif //ESP01_DEBUG

    if (!(expected && strstr(response_buffer, expected))) // Si motif non trouvé
    {
#if ESP01_DEBUG
    	Send_To_PC("expected not found\r\n");
#endif //ESP01_DEBUG
    	return -1;
    }

    return 0;
}

/**
 * @brief  Retrieve newly received bytes from the DMA circular RX buffer.
 * @param  buf     Output buffer where new data will be copied.
 * @param  bufsize Size of the output buffer.
 *
 * @retval Number of bytes copied
 */
int Get_New_Data(uint8_t *buf, uint16_t bufsize)
{
	/* Current DMA write position */
    uint16_t pos = dma_buf_size - __HAL_DMA_GET_COUNTER(wifi_uart->hdmarx);

    int len = 0;

    if (pos != rx_last_pos)
    {
        if (pos > rx_last_pos)
            len = pos - rx_last_pos;
        else
            len = dma_buf_size - rx_last_pos + pos;

        if (len > bufsize)
            len = bufsize;

        for (int i = 0; i < len; i++)
            buf[i] = dma_rx_buf[(rx_last_pos + i) % dma_buf_size];

        rx_last_pos = pos;

        return len;
    }
    return 0;
}

/**
 * @brief  Read incoming DMA data until a specific pattern is detected.
 * @param  pattern     String pattern to wait for.
 * @param  resp_buf    Buffer to store received data.
 * @param  bufsize     Size of the response buffer.
 * @param  timeout_ms  Timeout in milliseconds.
 *
 * @retval 0  Pattern found
 * @retval -1 Timeout or error
 */
int Read_DMA_Until_Pattern(const char *pattern, char *resp_buf, size_t bufsize, uint32_t timeout_ms)
{

	uint32_t start = HAL_GetTick();
	size_t resp_len = 0;
	resp_buf[0] = '\0';

	while ((HAL_GetTick() - start) < timeout_ms && resp_len < bufsize - 1)
	{
		uint8_t buf[64];
		int len = Get_New_Data(buf, sizeof(buf));
		if (len > 0)
		{
			if (resp_len + len >= bufsize - 1)
				len = bufsize - 1 - resp_len;
			memcpy(resp_buf + resp_len, buf, len);
			resp_len += len;
			resp_buf[resp_len] = '\0';

			if (pattern && strstr(resp_buf, pattern))
				break;
		}
		else
		{
			HAL_Delay(1);
		}
	}

#if ESP01_DEBUG
    Send_To_PC(resp_buf);
#endif //ESP01_DEBUG

    if (!(pattern && strstr(resp_buf, pattern)))
    {
#if ESP01_DEBUG
    	Send_To_PC("expected not found\r\n");
#endif //ESP01_DEBUG
    	return -1;
    }
    return 0;
}

/**
 * @brief  Flush the DMA RX buffer by waiting until no new data is received.
 * @param  timeout_ms Time to wait for RX inactivity (in milliseconds).
 * @retval None.
 */
void Flush_Rx_Buffer(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    volatile uint16_t last_pos = rx_last_pos;


    while ((HAL_GetTick() - start) < timeout_ms)
    {
        uint16_t pos = dma_buf_size - __HAL_DMA_GET_COUNTER(wifi_uart->hdmarx);

        if (pos != last_pos)
        {
            last_pos = pos;
            start = HAL_GetTick();
        }
    }
    rx_last_pos = last_pos;
}

/**
 * @brief  Initialize the ESP01 module and connect to a WiFi network.
 * @param  ssid     WiFi SSID.
 *
 * @retval 0  Success
 * @retval -1 Failure at any step
 */
int Init_Wifi(const char *ssid, const char *passwd)
{
	char resp[512]; /* Buffer for ESP responses */
	char cmd[512] = {0}; /* Buffer for dynamic AT commands */

	/* Start UART DMA reception */
	HAL_UART_Receive_DMA(wifi_uart, dma_rx_buf, 1024);
	HAL_Delay(500); /* Allow ESP to stabilize */

	/* Test communication */
	if(Send_ATCMD_DMA("AT", resp, sizeof(resp), "OK", ESP01_TIMEOUT)!=0) return -1;

	/* Set WiFi mode to Station */
	if(Send_ATCMD_DMA("AT+CWMODE=1", resp, sizeof(resp), "OK", ESP01_TIMEOUT)!=0) return -1;

	/* Enable DHCP */
	if(Send_ATCMD_DMA("AT+CWDHCP=1,1", resp, sizeof(resp), "OK", ESP01_TIMEOUT)!=0) return -1;

	/* Connect to WiFi network */
	snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, passwd);

	/* Connection may take several seconds */
	if(Send_ATCMD_DMA(cmd, resp, sizeof(resp), "OK", 15000)!=0) return -1;

	/* Request IP address */
	if(Send_ATCMD_DMA("AT+CIFSR", resp, sizeof(resp), "OK", ESP01_TIMEOUT)!=0) return -1;

	return 0;
}

/**
 * @brief  Retrieve current date and time from google server.
 *         Date is extracted from the HTTP "Date" header.
 * @param  day     Output day of week (0=Mon ... 6=Sun).
 * @param  dd      Output day of month.
 * @param  mm      Output month (0=Jan ... 11=Dec).
 * @param  yy      Output year.
 * @param  minute  Output time in minutes since midnight.
 *
 * @retval 0  Success
 * @retval -1 Failure (communication, parsing, or timeout error)
 */
int Get_Date(uint8_t *day, uint8_t *dd, uint8_t *mm, uint16_t *yy, uint16_t *minute)
{
	char resp[512]; /* Buffer for ESP responses */
	char date[40];  /* Extracted HTTP date string */

	/* need int value for sscanf function on stm32*/
	int hour;
	int min;
	int dd_var;
	int yy_var;

	char day_str[4]; /* 3-letter day string (e.g. "Fri") */
	char month_str[4]; /* 3-letter month string (e.g. "Jan") */

	/* Open TCP connection to Google server */
	if(Send_ATCMD_DMA("AT+CIPSTART=\"TCP\",\"216.239.35.0\",80", resp, sizeof(resp), "OK", ESP01_TIMEOUT)!=0) return -1;

	/* Inform ESP8266 of upcoming HTTP request length */
	if(Send_ATCMD_DMA("AT+CIPSEND=38", resp, sizeof(resp), ">", ESP01_TIMEOUT)!=0) return -1;

	/* Send minimal HTTP GET request */
	if(Send_ATCMD_DMA("GET / HTTP/1.1\r\nHost: 216.239.35.0\r\n", resp, sizeof(resp), "+IPD", ESP01_TIMEOUT)!=0) return -1;

	/* Read incoming data until end of Date header */
	if(Read_DMA_Until_Pattern("GMT\r\n", resp, sizeof(resp), ESP01_TIMEOUT)!=0) return -1;

	/*
	 * Expected HTTP header format:
	 * Date: Fri, 16 Jan 2026 15:25:15 GMT
	 */
	if(Date_from_HTTP(resp, date, sizeof(date))!=0) return -1;

	/* Parse date components */
	if(sscanf(date, "%3s, %2d %3s %4d %2d:%2d", day_str, &dd_var, month_str, &yy_var, &hour, &min) != 6) return -1;

	/* Convert and store results */
	*minute = hour*60 + min;
	*dd = dd_var;
	*yy = yy_var;
	*day = day_from_str(day_str);
	*mm = month_from_str(month_str);

	/* Close TCP connection */
	if(Send_ATCMD_DMA("AT+CIPCLOSE", resp, sizeof(resp), "OK", ESP01_TIMEOUT)!=0) return -1;
	return 0;

}

/**
 * @brief  Convert a 3-letter day string to a numeric value.
 * @param  day  Three-letter day string (e.g. "Mon").
 * @retval Day index (0=Mon ... 6=Sun).
 */
uint8_t day_from_str(const char *day)
{
    const char *days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    for (uint8_t i = 0; i < 7; i++)
    {
        if (memcmp(day, days[i],3) == 0) return i;
    }

    return 0; /* Default to Monday if parsing fails */
}

/**
 * @brief  Convert a 3-letter month string to a numeric value.
 * @param  month Three-letter month string (e.g. "Jan").
 * @retval Month index (0=Jan ... 11=Dec).
 */
uint8_t month_from_str(const char *month)
{
    const char *months[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    for (uint8_t i = 0; i < 12; i++)
    {
        if (memcmp(month, months[i],3) == 0) return i;
    }
    return 0; /* Default to January if parsing fails */
}

/**
 * @brief  Extract the HTTP "Date" header from a received HTTP response.
 * @param  trame         Full HTTP response buffer.
 * @param  date_buf      Output buffer for extracted date string.
 * @param  date_buf_size Size of the output buffer.
 *
 * @retval 0  Success
 * @retval -1 Failure (not found or buffer too small)
 */
int Date_from_HTTP(const char *trame, char *date_buf, size_t date_buf_size)
{
	if (!trame || !date_buf || date_buf_size == 0) return -1;

    const char *date_start;
    const char *date_end;
    size_t date_len;

    /* Locate "Date: " header */
    date_start = strstr(trame, "Date: ");
    if (!date_start) return -1;

    /* Skip "Date: " */
    date_start += strlen("Date: ");

    /* Find end of header line */
    date_end = strpbrk(date_start, "\r\n");
    if (!date_end) return -1;

    date_len = date_end - date_start;
    if (date_len >= date_buf_size) return -1;

    /* Copy date string */
    memcpy(date_buf, date_start, date_len);
    date_buf[date_len] = '\0';

    return 0;
}
