
#ifndef __EPAPER_H_
#define __EPAPER_H_

#include "DRIVER.h"

// Display resolution
#define EPAPER_WIDTH 240
#define EPAPER_HEIGHT 360
#define EPAPER_BUFFER_SIZE ((EPAPER_WIDTH*EPAPER_HEIGHT)/8)

#define WHITE 0x00
#define BLACK 0xFF
#define RED 0x0F

#define LUTGC_TEST //
#define LUTDU_TEST //

void EPAPER_Init(void);
void EPAPER_Init_Fast(void);
void EPAPER_Part_Init(void);
void EPAPER_refresh(void);
void EPAPER_shift_image(const uint8_t *image, uint8_t *shifted_image, uint16_t width, uint16_t height, uint8_t shift);
void EPAPER_KW_Partial_Display(const uint8_t *new_image, uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height);
void EPAPER_KW_White_Display(void);
void EPAPER_Clear(void);
void EPAPER_lut(void);
void EPAPER_Dup_Bits(const uint8_t input, uint8_t *output, uint8_t mult);
void EPAPER_Size_Mult(const uint8_t *image, uint8_t *mult_image, uint8_t mult, uint16_t byte_width, uint16_t height);
void EPAPER_Print_Char(const uint8_t *image,  uint8_t size, uint16_t x_start, uint16_t y_start);
void EPAPER_Print_String(const char *string,  uint8_t size, uint16_t x_start, uint16_t x_end, uint16_t y_start);
void EPAPER_Print_Hour(uint16_t min,  uint16_t prev_min);
void EPAPER_Print_Moon_Phase(uint8_t moon_phase, uint16_t min, uint16_t rise_time, uint16_t fall_time);
void EPAPER_Print_Date(uint8_t day,  uint8_t dd, uint8_t mm);
void EPAPER_Print_temp(int temp);
void EPAPER_Print_press(uint32_t press);
void EPAPER_Print_hum(uint32_t hum);

#endif
