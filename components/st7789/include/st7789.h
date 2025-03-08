#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "driver/ledc.h"
#include "ixora.h"

#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_BL 4


//CMD
#define SWRESET 0x01 //RESET screen 
#define NOP 0x00
#define SLPIN 0x10 //SLEEP MODE
#define SLPOUT 0x11 //SLEEP OUT
#define PTLON 0x12 //PARTIAL DISPLAY MODE ON
#define NORON 0x13 // NORMAL DISPLAY MODE ON
#define INVOFF 0x20 //DISPLAY INVERSION MODE OFF
#define INVON 0x21 //DISPLAY INVERSION ON
#define DISPOFF 0x28 //DISPLAY OFF 
#define DISPON 0x29 //DISPLAY On
#define CASET 0x2A //COLUMN ADDRESS SET
#define RASET 0x2B
#define COLMOD 0x3A
#define MADCTL 0x36
#define PORCTRL 0xB2
#define GCTRL 0xB7
#define POWSAVE 0xBC
#define VCOMS 0xBB
#define RAMWR 0x2C
#define TEON 0x35
#define Y_OFFSET    40
#define X_OFFSET    52

//DATA FOR CMD

#define COLOR_65K 0x55

//program
#define CMD_MODE 0
#define DATA_MODE 1
#define SPI_FREQUENCY 40 * 1000 * 1000
#define TFT_HEIGHT 240
#define TFT_WIDTH 135
#define WINDOW_PIXEL TFT_HEIGHT * TFT_WIDTH

//backlight
#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define TIMER_NUM LEDC_TIMER_0
#define DUTY_RESOLUTION LEDC_TIMER_8_BIT
#define FREQUENCY_TIMER 5000
#define CLK_CFG LEDC_AUTO_CLK

#define LEDC_CHANNEL LEDC_CHANNEL_0
#define GPIO_NUM TFT_BL


#define FONT_WIDTH  8    
#define FONT_HEIGHT 12   
#define FONT_START  0x20 
#define FONT_END    0x7F 


#define FONT_FILE   "/spiffs/font.bin"  


void RESET();
void spi_init();
void send_data(const uint8_t* data, size_t size);
void send_word(uint16_t data);
void send_cmd(uint8_t cmd);
void INIT();
void backlight(uint8_t duty);
void porch_control();
void set_window(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);
uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b);
void draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void send_color(uint16_t * color, uint16_t size);
void load_image(const char* path);
void draw_char_scaled(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale, uint8_t *font);
void draw_text_scaled(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t scale, uint8_t *font_data);