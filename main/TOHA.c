#include "st7789.h"
#include <stdio.h>
#include <string.h>











void app_main() {
    uint8_t font_data[(FONT_END - FONT_START) * FONT_HEIGHT]; 
    mount_spiffs();  
    load_font(font_data);     

    INIT();  
    while (1)
    {
        load_image("/spiffs/1.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/2.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/3.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/4.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/5.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/6.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/7.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/8.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/9.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/10.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/11.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/12.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/13.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/14.bin");

    }

}