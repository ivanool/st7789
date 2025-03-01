#include "st7789.h"
#include <stdio.h>
#include <string.h>

#define FONT_WIDTH  8    
#define FONT_HEIGHT 12   
#define FONT_START  0x20 
#define FONT_END    0x7F 
#define FONT_FILE   "/spiffs/font.bin"  

uint8_t font_data[(FONT_END - FONT_START) * FONT_HEIGHT]; 


void load_font() {
    FILE* file = fopen(FONT_FILE, "rb");
    if (!file) {
        ESP_LOGE("FONT", "Error al abrir el archivo de fuente");
        return;
    }

    fread(font_data, sizeof(font_data), 1, file);
    fclose(file);
    ESP_LOGI("FONT", "Fuente cargada correctamente");
}


void draw_char_scaled(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale) {
    if (c < FONT_START || c > FONT_END) return; 

    uint8_t *glyph = &font_data[(c - FONT_START) * FONT_HEIGHT]; 

    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t line = glyph[row]; 

        for (int col = 0; col < FONT_WIDTH; col++) {
            if (line & (1 << (7 - col))) { 
                for (int i = 0; i < scale; i++) {
                    for (int j = 0; j < scale; j++) {
                        draw_pixel(x + col * scale + i, y + row * scale + j, color);
                    }
                }
            }
        }
    }
}

void draw_text_scaled(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t scale) {
    uint16_t cursor_x = x;

    while (*text) {
        if (*text == '\n') {
            y += (FONT_HEIGHT + 2) * scale;
            cursor_x = x;
        } else {
            draw_char_scaled(cursor_x, y, *text, color, scale);
            cursor_x += FONT_WIDTH * scale;
        }
        text++;
    }
}


void app_main() {
    mount_spiffs();  
    load_font();     

    INIT();  
    const char* numbers[] = {
        "1",
        "2",
        "20",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "10",
        "11",
        "12",
        "20"
    };
    draw_rectangle(0,0,TFT_WIDTH, TFT_HEIGHT, rgb888_to_rgb565(255, 255, 255));
    while (1)
    {
        for(int i = 0; i <13; i++){
            draw_text_scaled(10, 20, numbers[i], rgb888_to_rgb565(205, 127, 115), 3);
            vTaskDelay(4);
            draw_rectangle(10, 20, 100, 100, rgb888_to_rgb565(255, 255,255));
        }
        }
        
   
}
