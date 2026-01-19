# STM32 Hardware & CubeIDE Setup

## Hardware Connections

### ESP-01 (UART1)
**USART1 (to ESP-01)**
- PA9  – TX  
- PA10 – RX  

---

### PC Communication & Debug (UART2)
**USART2 (to USB-UART / PC)**
- PA2 – TX  
- PA3 – RX  

---

### BME280 (I2C)
**I2C (BME280 sensor)**
- PB9 – SDA  
- PB8 – SCL  

---

### E-Paper Module (SPI1 + GPIO)
**SPI1**
- PA5 – SCK  
- PA7 – MOSI / DIN  

**Control pins (GPIO output)**
- PA1 – RST  
- PA4 – SPI_CS  
- PA6 – DC  

**Status pin (GPIO input)**
- PA12 – BUSY  

---

## STM32CubeIDE Configuration

> Adjust pin names if your MCU variant differs.

### TIM3 (Periodic Interrupt Every Minute)
- Clock source: **Internal Clock**
- (Example) APB clock: **48 MHz**
- Prescaler: **47999**  
- Counter Period: **59999**  
- Interrupt: **Enabled**

This configuration (48 MHz / (47999 + 1) / (59999 + 1)) ≈ 1 Hz / 60 → 1 interrupt per minute.  
Adapt `Prescaler` and `Counter Period` to your own clock if different.

---

### USART1 (ESP-01)
- Mode: **Asynchronous**
- Baud rate: **115200**
- DMA: **Enabled on UART_RX** in **circular** mode

---

### USART2 (PC / Debug)
- Mode: **Asynchronous**
- Baud rate: **115200**

---

### I2C (BME280)
- Mode: **I2C**
- Configure speed (Standard/Fast) as needed by your application and wiring.

---

### SPI1 (E-Paper)
- Mode: **Transmit Only Master**
- Data size: **8 bits**
- Bit order: **MSB First**
- Prescaler: **256** (maximum, for a low SPI speed)
- Clock polarity (CPOL): **Low**
- Clock phase (CPHA): **1 Edge**

---

### GPIO
**Outputs (RST, SPI_CS, DC, etc.)**
- Mode: **Output**
- Output type: **Push-Pull**

**Inputs (BUSY)**
- Mode: **Input**
- Pull-up / Pull-down: configure as required by your e-paper module (often no pull, check datasheet).

---

## Code Modifications

File: `stm32f0xx_it.c`  
(Use the corresponding interrupt file for your STM32 series, e.g. `stm32f1xx_it.c`, `stm32l4xx_it.c`, etc.)

### 1. Declare the `minute` Variable

At the top of the file, in the **private variables** section, add:

extern uint16_t minute;

 2. Increment `minute` in the TIM3 Interrupt

Inside `TIM3_IRQHandler` (or `TIM3_IRQHandler(void)`), add:

void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
  minute++;
  /* USER CODE END TIM3_IRQn 0 */

  HAL_TIM_IRQHandler(&htim3);
}

Make sure the increment is inside the `USER CODE` section so it is not overwritten by code generation.
