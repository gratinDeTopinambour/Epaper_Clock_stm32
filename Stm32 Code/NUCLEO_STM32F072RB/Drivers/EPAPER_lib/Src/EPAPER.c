

#include "EPAPER.h"
#include "DRIVER.h"
#include "EPAPER_LUT.h"
#include "pixel_font.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

unsigned char EPAPER_Flag = 0;

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPAPER_Init(void)
{
    EPAPER_Reset();

    EPAPER_SendCommand(0x61);
    EPAPER_SendData(0xF0);
    EPAPER_SendData(0x01);
    EPAPER_SendData(0x68);

    EPAPER_SendCommand(0x06);
    EPAPER_SendData(0x17);
    EPAPER_SendData(0x17);
    EPAPER_SendData(0x17);

    EPAPER_SendCommand(0x00);
    EPAPER_SendData(0x0F);
    EPAPER_SendData(0x0D);

    EPAPER_SendCommand(0x04); // POWER ON
    HAL_Delay(100);
    EPAPER_ReadBusy();
}


void EPAPER_Part_Init(void)
{
    EPAPER_Reset();

    EPAPER_SendCommand(0x04); // POWER ON
    HAL_Delay(100);
    EPAPER_ReadBusy();

    EPAPER_SendCommand(0x00);
    EPAPER_SendData(0x3F);
    EPAPER_SendData(0x0D);

    EPAPER_SendCommand(0x61);
    EPAPER_SendData(0xF0);
    EPAPER_SendData(0x01);
    EPAPER_SendData(0x68);


    EPAPER_SendCommand(0x50);
    EPAPER_SendData(0xF7);

    EPAPER_SendCommand(0xE3);
    EPAPER_SendData(0x88);

    EPAPER_lut();
}

void EPAPER_refresh(void)
{
    EPAPER_SendCommand(0x17);
    EPAPER_SendData(0xA5);
    EPAPER_ReadBusy();
}

void EPAPER_shift_image(const uint8_t *image, uint8_t *shifted_image, uint16_t width, uint16_t height, uint8_t shift)
{

	if(shift == 0){
		memcpy(shifted_image, image, (width % 8 == 0 ? (width / 8) : (width / 8 + 1))*height);
		return;
	}

	uint16_t byte_width = width % 8 == 0 ? (width / 8) : (width / 8 + 1);
	uint8_t carry = 0xFF << (8 - shift);
	uint8_t n_carry = 0xFF;
	uint8_t new_width_byte = (width+shift) % 8 == 0 ? ((width+shift) / 8) : ((width+shift) / 8 + 1);


	 for (uint16_t j = 0; j < height; j++)
	 {
		 for (uint16_t i = 0; i < byte_width; i++)
		 {
			 n_carry = image[i + j * byte_width] << (8-shift);
			 shifted_image[i + j * new_width_byte] = carry | (image[i + j * byte_width] >> shift);
			 carry = n_carry;
		 }

		 if(new_width_byte != byte_width){
			 shifted_image[(new_width_byte - 1) + j * new_width_byte] = carry | (0xFF >> shift);
		 }

		 carry = 0xFF << (8 - shift);

	 }
}

/******************************************************************************
function :	plot partial screen in KW mode
parameter:
    image: Image data
    color: BLACK or RED
    x_start: start x point
    y_start: start y point
    width: width
    height: height
******************************************************************************/
void EPAPER_KW_Partial_Display(const uint8_t *new_image, uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height)
{

    uint8_t hrst = x_start & 0xF8;                        // colonne de départ
    uint8_t hred = ((x_start + width - 1) & 0xF8) | 0x07; // colonne de fin
    uint8_t vrst8 = y_start >> 8;                         // Ligne de départ
    uint8_t vrst = y_start & 0xFF;                        // Ligne de départ
    uint8_t vred8 = (y_start + height - 1) >> 8;          // Ligne de fin
    uint8_t vred = (y_start + height - 1) & 0xFF;         // Ligne de fin
    uint8_t pt_scan = 0x01;

    EPAPER_SendCommand(0x91); // partial mode on cmd
    EPAPER_SendCommand(0x90); // partial window cmd
    EPAPER_SendData(hrst);
    EPAPER_SendData(hred);
    EPAPER_SendData(vrst8);
    EPAPER_SendData(vrst);
    EPAPER_SendData(vred8);
    EPAPER_SendData(vred);
    EPAPER_SendData(pt_scan);

    uint8_t shift = x_start & 0x07;
    uint16_t byte_width = (width+shift) % 8 == 0 ? ((width+shift) / 8) : ((width+shift) / 8 + 1);
    uint8_t shift_image[byte_width * height];


    EPAPER_shift_image(new_image, shift_image, width, height, shift);
    EPAPER_SendCommand(0x13);

    for (uint16_t j = 0; j < height; j++)
    {
        for (uint8_t i = 0; i < byte_width; i++)
        {
            EPAPER_SendData(shift_image[i + j * byte_width]);
        }
    }

    EPAPER_refresh();
    EPAPER_SendCommand(0x92); // partial mode off cmd
}

/******************************************************************************
function :	plot full screen in KW mode
parameter:
******************************************************************************/
void EPAPER_KW_White_Display(void)
{

	EPAPER_SendCommand(0x13); //buffer for new image


	 for (uint16_t i = 0; i < EPAPER_BUFFER_SIZE; i++)
	 {
		 EPAPER_SendData(0xFF);
	 }

	EPAPER_refresh();
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPAPER_Clear(void)
{

	EPAPER_SendCommand(0x10);
	for (uint16_t j = 0; j < EPAPER_HEIGHT; j++)
	 {
		 for (uint16_t i = 0; i < (EPAPER_WIDTH % 8 == 0 ? (EPAPER_WIDTH / 8) : (EPAPER_WIDTH / 8 + 1)); i++)
		 {
			 EPAPER_SendData(0x00);
		 }
	 }

	EPAPER_SendCommand(0x13);
	for (uint16_t j = 0; j < EPAPER_HEIGHT; j++)
	 {
		 for (uint16_t i = 0; i < (EPAPER_WIDTH % 8 == 0 ? (EPAPER_WIDTH / 8) : (EPAPER_WIDTH / 8 + 1)); i++)
		 {
			 EPAPER_SendData(0x00);
		 }
	 }

	EPAPER_refresh();
}

void EPAPER_lut(void)
{
    uint8_t count;
    EPAPER_SendCommand(0x20);        // vcom
    for(count = 0; count < 42 ; count++)
    {
        EPAPER_SendData(EPAPER_lut_vcom[count]);
    }

    EPAPER_SendCommand(0x21);        // ww --
    for(count = 0; count < 42 ; count++)
    {
        EPAPER_SendData(EPAPER_lut_ww[count]);
    }

    EPAPER_SendCommand(0x22);        // bw r
    for(count = 0; count < 42 ; count++)
    {
        EPAPER_SendData(EPAPER_lut_bw[count]);
    }

    EPAPER_SendCommand(0x23);        // wb w
    for(count = 0; count < 42 ; count++)
    {
        EPAPER_SendData(EPAPER_lut_wb[count]);
    }

    EPAPER_SendCommand(0x24);        // bb b
    for(count = 0; count < 42 ; count++)
    {
        EPAPER_SendData(EPAPER_lut_bb[count]);
    }
}

/******************************************************************************
function :	refresh screen to remove glitch
parameter:
******************************************************************************/
void EPAPER_Full_Refresh(void)
{
	EPAPER_Init();
	EPAPER_Clear();
}

/******************************************************************************
function :	dup n time pixel inside input
parameter:
******************************************************************************/
void EPAPER_Dup_Bits(const uint8_t input, uint8_t *output, uint8_t mult)
{
	if (mult <= 0) return;

	uint16_t bit_pos = 0;
	uint8_t bit = 0;
	uint8_t byte_index = 0;
	uint8_t bit_index = 0;

    uint8_t output_bytes = (8 * mult + 7) / 8; // Round up to nearest byte
    for (uint8_t i = 0; i < output_bytes; i++) {
        output[i] = 0;
    }

	for (int8_t b = 7; b>=0; b--)
	{
		bit = (input>>b) & 1;
		for (uint8_t k = 0; k < mult; k++)
		{
			byte_index = bit_pos / 8;
			bit_index = 7 - (bit_pos % 8);
			if(bit) output[byte_index] |= (1<<bit_index);
			bit_pos++;
		}
	}
}

/******************************************************************************
function :	increase image by mult (horizontal image)
parameter:
    image: Image data
    mult_image: buffer that will contain the multiply image (mult_image need to be big enought)
    byte_width: number of byte per column
    height: number of pixel per line
******************************************************************************/
void EPAPER_Size_Mult(const uint8_t *image, uint8_t *mult_image, uint8_t mult, uint16_t byte_width, uint16_t height)
{
	if(byte_width*mult*8 > 240 || height*mult > 360){
		return;
	}

    uint8_t byte_buf[20];

    for (uint16_t j = 0; j < height*mult; j++)
    {
        for (uint8_t i = 0; i < byte_width; i++)
        {

        	EPAPER_Dup_Bits(image[i + j/mult*byte_width], byte_buf, mult);

            for (uint8_t k = 0; k < mult; k++)
            {
            	mult_image[i*mult + k + j*byte_width*mult] = byte_buf[k];
            }
        }
    }

}

/******************************************************************************
function :	print char
parameter:
******************************************************************************/
void EPAPER_Print_Char(const uint8_t *image,  uint8_t size, uint16_t x_start, uint16_t y_start)
{
	if(size == 1){
		EPAPER_KW_Partial_Display(image, x_start, y_start, 8, 5);
		return;
	}

	uint8_t image_buf[5*size*size];


	EPAPER_Size_Mult(image, image_buf, size, 1, 5);

	EPAPER_KW_Partial_Display(image_buf, x_start, y_start, 8*size, 5*size);
}

static inline uint8_t char_pixel_width(char c)
{
    if (c == ' ' || c == ',')
        return 3;   // 2 pixels glyph + 1 spacing
    else
        return 6;   // 5 pixels glyph + 1 spacing
}

static uint16_t font_index(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0') * 5;
    else if (c >= 'A' && c <= 'Z')
        return (c - 'A' + 10) * 5;
    else if (c >= 'a' && c <= 'z')
        return (c - 'a' + 36) * 5;
    else if (c == '*')
        return 62 * 5;
    else if (c == '-')
        return 63 * 5;
    else if (c == '%')
        return 64 * 5;
    else if (c == ' ')
        return 65 * 5;
    else
        return (65 * 5 + 2); // , / unknown
}

/******************************************************************************
function :	print char
parameter:
******************************************************************************/
void EPAPER_Print_String(const char *string,  uint8_t size, uint16_t x_start, uint16_t x_end, uint16_t y_start)
{
	// can't have size equal to 0
	if (!string || size == 0)
	        return;

	uint16_t max_width_px = x_end - x_start;
	uint16_t max_height_px = EPAPER_WIDTH - y_start;

	if (max_width_px < 6 * size || max_height_px < 8 * size)
	        return;

	uint8_t line_buf[256];              // enough for most widths (5px font)
	uint8_t line_buf_mult[(x_end - x_start) * size];

	uint16_t x_cursor_px = 0;
	uint16_t y_cursor_px = 0;
	uint16_t width_for_mult = 0;

	// Clear buffers
	memset(line_buf, 0xFF, sizeof(line_buf));
	memset(line_buf_mult, 0xFF, sizeof(line_buf_mult));

	while (*string) {

		uint8_t char_w = char_pixel_width(*string);
		uint16_t scaled_w = char_w * size;

		// Wrap line if needed
		if (x_cursor_px + scaled_w > max_width_px) {

			// Draw current line
			if (size == 1) {
				memcpy(line_buf_mult, line_buf, x_cursor_px);
				EPAPER_KW_Partial_Display(line_buf_mult, y_start + y_cursor_px, x_start, 8, max_width_px);
			} else {
				EPAPER_Size_Mult(line_buf, line_buf_mult, size, 1, width_for_mult);
				EPAPER_KW_Partial_Display(line_buf_mult, y_start + y_cursor_px, x_start, 8 * size,max_width_px);
			}

			// Move to next line
			y_cursor_px += 8 * size;
			if (y_cursor_px + 8 * size > max_height_px)
				return;

			memset(line_buf, 0xFF, sizeof(line_buf));
			memset(line_buf_mult, 0xFF, sizeof(line_buf_mult));
			x_cursor_px = 0;
			width_for_mult = 0;
		}

		uint16_t buf_index = x_cursor_px / size;
		uint16_t ind = font_index(*string);

		if (*string == ' ' || *string == ',') {
			memcpy(&line_buf[buf_index], &bpixel[ind], 2);
		} else {
			memcpy(&line_buf[buf_index], &bpixel[ind], 5);
		}

		x_cursor_px += scaled_w;
		width_for_mult += char_w;
		string++;
	}

	// Flush last line
	if (x_cursor_px > 0) {
		if (size == 1) {
			memcpy(line_buf_mult, line_buf, x_cursor_px);
			EPAPER_KW_Partial_Display(line_buf_mult, y_start + y_cursor_px, x_start, 8, max_width_px);
		} else {
			EPAPER_Size_Mult(line_buf, line_buf_mult, size, 1, width_for_mult);
			EPAPER_KW_Partial_Display(line_buf_mult, y_start + y_cursor_px, x_start, 8 * size,max_width_px);
		}
	}
}

/******************************************************************************
function :	vertical flip image
parameter:
******************************************************************************/
static void EPAPER_V_Flip(const uint8_t *image, uint8_t *flip_image, uint16_t byte_width, uint16_t height)
{
    for (uint16_t j = 0; j < height; j++)
    {
        for (uint8_t i = 0; i < byte_width; i++)
        {

        	flip_image[i + j*byte_width] = image[i + ((height - 1) - j)*byte_width];
        }
    }

}

/******************************************************************************
function :	print hour
parameter:  min ,minute from 0h00
			prev_min, previous minute from 0h00
******************************************************************************/
void EPAPER_Print_Moon_Phase(uint8_t moon_phase, uint16_t min, uint16_t rise_time, uint16_t fall_time)
{
	uint16_t v_pos = 108; //center
	uint16_t h_pos = 108;
	uint8_t radius = 49;
	uint16_t ind = 0;
	uint8_t icone_buf[16*2*2*2];
	uint8_t icone_buf_flip[16*2*2*2];

	if(min < 90){
		v_pos = 0;
		h_pos = (uint16_t)(130 - (77.0/2) - (min/180.0)*77);
	}
	else if(min < 270){
		v_pos = sin(-(min-90)/180.0*3.14/2 - 0.5*3.14)*radius + 49;
		h_pos = cos(-(min-90)/180.0*3.14/2 - 0.5*3.14)*radius + 51;
	}
	else if(min < 450){
		v_pos = (uint16_t)(0 + 49 + ((min-270.0)/180.0)*77);
		h_pos = 2;
	}
	else if(min < 630){
		v_pos = sin(-(min-450)/180.0*3.14/2 - 3.14)*radius + 126;
		h_pos = cos(-(min-450)/180.0*3.14/2 - 3.14)*radius + 51;
	}
	else if(min < 810){
		v_pos = 176;
		h_pos = (uint16_t)(2 + 49 + ((min-630.0)/180.0)*77);
	}
	else if(min < 990){
		v_pos = sin(-(min-810)/180.0*3.14/2 - 1.5*3.14)*radius + 126;
		h_pos = cos(-(min-810)/180.0*3.14/2 - 1.5*3.14)*radius + 128;
	}
	else if(min < 1170){
		v_pos = (uint16_t)(126 - ((min-990.0)/180.0)*77);
		h_pos = 178;
	}
	else if(min < 1350){
		v_pos = sin(-(min-1170)/180.0*3.14/2)*radius + 49;
		h_pos = cos(-(min-1170)/180.0*3.14/2)*radius + 128;
	}
	else{
		v_pos = 0;
		h_pos = (uint16_t)(128 - ((min-1350.0)/180.0)*77);
	}


	if(min>rise_time && min<fall_time){
		ind = 0;//soleil
		EPAPER_Size_Mult(&icone[ind], icone_buf, 2, 2, 16);
		EPAPER_KW_Partial_Display(icone_buf, v_pos, h_pos, 32, 32);
	}
	else
	{


		if(moon_phase > 4){
			ind = (8-moon_phase)*32;
			EPAPER_Size_Mult(&moon[ind], icone_buf, 2, 2, 16);
			EPAPER_V_Flip(icone_buf, icone_buf_flip, 4, 32);
			EPAPER_KW_Partial_Display(icone_buf_flip, v_pos, h_pos, 32, 32);
		}
		else{
			ind = moon_phase*32;
			EPAPER_Size_Mult(&moon[ind], icone_buf, 2, 2, 16);
			EPAPER_KW_Partial_Display(icone_buf, v_pos, h_pos, 32, 32);
		}

	}

}


/******************************************************************************
function :	print hour
parameter:  min ,minute from 0h00
			prev_min, previous minute from 0h00
******************************************************************************/
void EPAPER_Print_Hour(uint16_t min,  uint16_t prev_min)
{
	uint8_t first_digit;
	uint8_t second_digit;
	uint8_t third_digit;
	uint8_t fourth_digit;
	uint16_t v_pos = 104;
	uint16_t h_pos = 60;

	if(prev_min/600 != min/600){
		first_digit = min/600;
		EPAPER_Print_Char(&bpixel[first_digit*5],  8, v_pos, h_pos);
	}

	if(prev_min/60 != min/60){
		second_digit = (min%600)/60;
		EPAPER_Print_Char(&bpixel[second_digit*5],  8, v_pos, 108);
	}

	if(prev_min/10 != min/10){
		third_digit = (min%60)/10;
		EPAPER_Print_Char(&bpixel[third_digit*5],  8, 40, h_pos);
	}

	if(prev_min%10 != min%10){
		fourth_digit = min%10;
		EPAPER_Print_Char(&bpixel[fourth_digit*5],  8, 40, 108);
	}
}

/******************************************************************************
function :	print date
parameter:  day , 0 to 6 for monday to sunday
			dd, day of the mounth
			mm, 0 to 11 for january to december
******************************************************************************/
void EPAPER_Print_Date(uint8_t day,  uint8_t dd, uint8_t mm)
{
	char date[21] = "";
	switch(day){
		case 0:
			//lundi
			strcat(date, "Lundi ");
			break;

		case 1:
			//mardi
			strcat(date, "Mardi ");
			break;

		case 2:
			//mercredi
			strcat(date, "Mercredi ");
			break;

		case 3:
			//jeudi
			strcat(date, "Jeudi ");
			break;

		case 4:
			//vendredi
			strcat(date, "Vendredi ");
			break;

		case 5:
			//samedi
			strcat(date, "Samedi ");
			break;

		case 6:
			//dimanche
			strcat(date, "Dimanche ");
			break;

		default:
			//lundi
			strcat(date, "Lundi ");
	}

	char char_dd[3];
	sprintf(char_dd, "%u", dd);
	strcat(date, char_dd);

	switch(mm){
		case 0:
			//janvier
			strcat(date, " janvier");
			break;

		case 1:
			//fevrier
			strcat(date, " fevrier");
			break;

		case 2:
			//mars
			strcat(date, " mars");
			break;

		case 3:
			//avril
			strcat(date, " avril");
			break;

		case 4:
			//mai
			strcat(date, " mai");
			break;

		case 5:
			//juin
			strcat(date, " juin");
			break;

		case 6:
			//juillet
			strcat(date, " juillet");
			break;

		case 7:
			//aout
			strcat(date, " aout");
			break;

		case 8:
			//septembre
			strcat(date, " septembre");
			break;

		case 9:
			//octobre
			strcat(date, " octobre");
			break;

		case 10:
			//novembre
			strcat(date, " novembre");
			break;

		case 11:
			//decembre
			strcat(date, " decembre");
			break;

		default:
			//janvier
			strcat(date, " janvier");
	}
	EPAPER_Print_String(date,  3, 0, 360, 208);
}

/******************************************************************************
function :	print temperature
parameter:  temp: temperature
******************************************************************************/
void EPAPER_Print_temp(int temp)
{
	char string_temp[7];
	int dec_temp = temp%10;
	int num_temp = temp/10;
	if(temp <= 0)
	{
		dec_temp = 0;
		num_temp = 0;
	}
	if(temp >= 1000)
	{
		dec_temp = 99;
		num_temp = 9;
	}
	sprintf(string_temp, "%2d,%1d*C", num_temp, dec_temp);
	EPAPER_Print_String(string_temp,  4, 218, 360, 150);
}

/******************************************************************************
function :	print pression
parameter:  press: pression
******************************************************************************/
void EPAPER_Print_press(uint32_t press)
{
	char string_press[9];
	sprintf(string_press, "%4lu hPa", press);
	EPAPER_Print_String(string_press,  3, 215, 360, 10);
}

/******************************************************************************
function :	print humidity
parameter:  hum: humidity in %
******************************************************************************/
void EPAPER_Print_hum(uint32_t hum)
{
	char string_hum[5];
	sprintf(string_hum, "%2lu %%", hum);
	EPAPER_Print_String(string_hum,  4, 266, 360, 80);
}
