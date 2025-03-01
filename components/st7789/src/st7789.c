#include "st7789.h"

spi_device_handle_t spi;


void send_cmd(uint8_t cmd) {
    gpio_set_level(TFT_DC, CMD_MODE);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    ESP_ERROR_CHECK(spi_device_transmit(spi, &t));
}

void send_data(const uint8_t* data, size_t size) {
    spi_transaction_t SPIT;
    gpio_set_level(TFT_DC, DATA_MODE);
    memset(&SPIT, 0, sizeof(spi_transaction_t));
    SPIT.length = size * 8;
    SPIT.tx_buffer = data;
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &SPIT));
}

void send_word(uint16_t data){
    uint8_t buffer[2] = {data >> 8, data & 0xFF};
    send_data(buffer, 2);
}



void gpio_init() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TFT_DC) | (1ULL << TFT_RST) | (1ULL << TFT_BL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

void RESET(){
    gpio_set_level(TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(TFT_RST, 1);
    send_cmd(SWRESET);

    vTaskDelay(pdMS_TO_TICKS(150));
}

void backlight(uint8_t duty) {

    ledc_timer_config_t ledc_timer = {
        .speed_mode = SPEED_MODE,
        .timer_num = TIMER_NUM,
        .duty_resolution = DUTY_RESOLUTION,
        .freq_hz = FREQUENCY_TIMER,
        .clk_cfg = CLK_CFG,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = SPEED_MODE,
        .channel = LEDC_CHANNEL,
        .gpio_num = GPIO_NUM,
        .timer_sel = TIMER_NUM,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL);

}

void porch_control() {
    
    send_cmd(PORCTRL);
    uint8_t porch_data[] = {
        0x0C,   // VBP: 12 (0x0C)
        0x0C,   // VFP: 12 (0x0C)
        0x00,   // Bit 7: PSON, Bits 6-0: HBP (0x00)
        0x18,   // HFP: 24 (0x18)
        0x04,   // HBP: 4 (0x04)
    };
    send_data(porch_data, sizeof(porch_data));
}

void set_orientation(uint8_t data) {
    send_cmd(MADCTL);
    send_data(&data, 1);
}

void set_window(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1) {
    x0 = (x0 >= TFT_WIDTH) ? TFT_WIDTH - 1 : x0;
    x1 = (x1 >= TFT_WIDTH) ? TFT_WIDTH - 1 : x1;
    y0 = (y0 >= TFT_HEIGHT) ? TFT_HEIGHT - 1 : y0;
    y1 = (y1 >= TFT_HEIGHT) ? TFT_HEIGHT - 1 : y1;

    send_cmd(CASET);
    send_word(x0 + X_OFFSET);
    send_word(x1 + X_OFFSET);
    
    send_cmd(RASET);
    send_word(y0 + Y_OFFSET);
    send_word(y1 + Y_OFFSET);
}

void INIT() {
    spi_init();
    gpio_init();
    RESET();
    
    send_cmd(SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(120));

    send_cmd(COLMOD);
    uint8_t colmod_data = 0x55;
    send_data(&colmod_data, 1);

    set_orientation(0x00); 
    
    porch_control(); 
    
    send_cmd(GCTRL);
    uint8_t gctrl_data = 0x75; 
    send_data(&gctrl_data, 1);
    
    send_cmd(VCOMS);
    uint8_t vcoms_data = 0x2B; 
    send_data(&vcoms_data, 1);
     
    vTaskDelay(pdMS_TO_TICKS(10));
    send_cmd(INVON);
    send_cmd(NORON);
    send_cmd(DISPON);
    vTaskDelay(pdMS_TO_TICKS(150));
    
    backlight(128); 
}

void spi_init() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI,
        .sclk_io_num = TFT_SCLK,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TFT_HEIGHT * TFT_WIDTH * 2 
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000, 
        .mode = 0,
        .spics_io_num = TFT_CS,
        .queue_size = 9,
        .flags = SPI_DEVICE_NO_DUMMY
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));
}



uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}


/**
 * @brief Sends an array of colors to the display.
 *
 * This function takes an array of 16-bit color values and converts them into
 * an array of bytes, which are then sent to the display using the send_data function.
 *
 * @param color Pointer to an array of 16-bit color values.
 * @param size The number of color values in the array.
 */
void send_color(uint16_t * color, uint16_t size){
    static uint8_t byte[1024];
    int index = 0;
    for(int i = 0; i < size; i++){
        byte[index++] = (color[i]>>8) & 0xFF;
        byte[index++] = color[i]&0xFF;
    }
    send_data(byte, size*2);
}

/**
 * @brief Draw a single pixel on the display.
 *
 * This function sets a window at the specified coordinates and sends the color
 * data to draw a single pixel.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The color of the pixel in 16-bit format.
 */
void draw_pixel(uint16_t x, uint16_t y, uint16_t color){
    set_window(x, x, y, y);
    send_cmd(RAMWR);
    send_color(&color, 1);
}


/**
 * @brief Draws a filled rectangle on the display.
 *
 * This function sets a window on the display and fills it with the specified color.
 *
 * @param x1 The x-coordinate of the top-left corner of the rectangle.
 * @param y1 The y-coordinate of the top-left corner of the rectangle.
 * @param x2 The x-coordinate of the bottom-right corner of the rectangle.
 * @param y2 The y-coordinate of the bottom-right corner of the rectangle.
 * @param color The color to fill the rectangle with.
 */
void draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    set_window(x1, x2, y1, y2);
    send_cmd(RAMWR);

    uint16_t width = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;
    uint32_t total_pixels = width * height;

    uint16_t color_buffer[512];
    const uint16_t chunk_size = sizeof(color_buffer)/sizeof(uint16_t);

    for (int i = 0; i < chunk_size; i++) {
        color_buffer[i] = color;
    }

    while (total_pixels > 0) {
        uint16_t current_chunk = (total_pixels > chunk_size) ? chunk_size : total_pixels;
        send_color(color_buffer, current_chunk);
        total_pixels -= current_chunk;
    }
}