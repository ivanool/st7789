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

#define IMAGE_WIDTH 135
#define IMAGE_HEIGHT 240
#define IMAGE_FILE "/spiffs/image.bin"
#define IMAGEFILE_2 "/spiffs/image2.bin"


void load_image(const char* path) {
    FILE* file = fopen(path, "rb");
    uint16_t* img_buf = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 2);
    
    fread(img_buf, 2, IMAGE_WIDTH * IMAGE_HEIGHT, file);
    fclose(file);

    for (int x = 0; x < IMAGE_WIDTH; x++) {
        set_window(x, x, 0, IMAGE_HEIGHT-1);
        send_cmd(RAMWR);
        send_color(&img_buf[x * IMAGE_HEIGHT], IMAGE_HEIGHT);
    }
    
    free(img_buf);
}


void app_main() {
    mount_spiffs();  
    load_font();     

    INIT();  
    while (1)
    {
        load_image(IMAGE_FILE);
        vTaskDelay(10);
        load_image(IMAGEFILE_2);
        vTaskDelay(10);
    }

}