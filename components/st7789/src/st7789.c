#include "st7789.h"

spi_device_handle_t spi;
static uint16_t frame_buffer[TFT_WIDTH * TFT_HEIGHT];

/**
 * @brief Sends a command to the ST7789 display.
 *
 * This function sets the display to command mode and transmits an 8-bit command
 * via SPI to the ST7789 display.
 *
 * @param cmd The 8-bit command to be sent to the display.
 */

void send_cmd(uint8_t cmd) {
    gpio_set_level(TFT_DC, CMD_MODE);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    ESP_ERROR_CHECK(spi_device_transmit(spi, &t));
}

/**
 * @brief Sends data to the ST7789 display via SPI.
 *
 * This function sends a block of data to the ST7789 display using the SPI interface.
 * It sets the data/command pin to data mode, prepares the SPI transaction, and transmits
 * the data using polling mode.
 *
 * @param data Pointer to the data buffer to be sent.
 * @param size Size of the data buffer in bytes.
 */
void send_data(const uint8_t* data, size_t size) {
    spi_transaction_t SPIT;
    gpio_set_level(TFT_DC, DATA_MODE);
    memset(&SPIT, 0, sizeof(spi_transaction_t));
    SPIT.length = size * 8;
    SPIT.tx_buffer = data;
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &SPIT));
}

/**
 * @brief Sends a 16-bit word to the display.
 *
 * This function takes a 16-bit data word, splits it into two 8-bit values,
 * and sends them to the display using the send_data function.
 *
 * @param data The 16-bit data word to be sent.
 */
void send_word(uint16_t data){
    uint8_t buffer[2] = {data >> 8, data & 0xFF};
    send_data(buffer, 2);
}



/**
 * @brief Initialize GPIO pins for the TFT display.
 *
 * This function configures the GPIO pins used for the TFT display's
 * Data/Command (TFT_DC), Reset (TFT_RST), and Backlight (TFT_BL) signals.
 * The pins are set to output mode with no pull-up or pull-down resistors,
 * and interrupts are disabled.
 *
 * The specific pins used are defined by the macros TFT_DC, TFT_RST, and TFT_BL.
 */
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

/**
 * @brief Resets the TFT display.
 *
 * This function performs a hardware reset on the TFT display by toggling the
 * reset pin (TFT_RST). It sets the reset pin low, waits for 20 milliseconds,
 * sets the reset pin high, and then sends the software reset command (SWRESET).
 * Finally, it waits for 150 milliseconds to allow the display to complete the
 * reset process.
 */
void RESET(){
    gpio_set_level(TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(TFT_RST, 1);
    send_cmd(SWRESET);

    vTaskDelay(pdMS_TO_TICKS(150));
}
/**
 * @brief Configures and sets the backlight duty cycle.
 *
 * This function initializes the LEDC timer and channel configurations 
 * and sets the duty cycle for the backlight. It uses the LEDC (LED 
 * Controller) peripheral to control the brightness of the backlight.
 *
 * @param duty The duty cycle to set for the backlight. This value 
 *             determines the brightness of the backlight.
 *
 * @note The function assumes that the following macros are defined:
 *       - SPEED_MODE: The speed mode of the LEDC timer.
 *       - TIMER_NUM: The timer number to use for the LEDC timer.
 *       - DUTY_RESOLUTION: The resolution of the duty cycle.
 *       - FREQUENCY_TIMER: The frequency of the LEDC timer.
 *       - CLK_CFG: The clock configuration for the LEDC timer.
 *       - LEDC_CHANNEL: The LEDC channel to configure.
 *       - GPIO_NUM: The GPIO number to use for the LEDC channel.
 *       - LEDC_HIGH_SPEED_MODE: The high-speed mode for the LEDC.
 */

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

/**
 * @brief Configures the porch settings for the ST7789 display.
 *
 * This function sends the PORCTRL command to the display and then sends the
 * porch control data. The porch control data includes the following settings:
 * - VBP (Vertical Back Porch): 12 (0x0C)
 * - VFP (Vertical Front Porch): 12 (0x0C)
 * - PSON (Partial Scan On): 0 (Bit 7)
 * - HBP (Horizontal Back Porch): 4 (0x04)
 * - HFP (Horizontal Front Porch): 24 (0x18)
 *
 * The porch settings control the timing of the display's vertical and horizontal
 * blanking intervals.
 */
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

/**
 * @brief Sets the orientation of the display.
 *
 * This function sends a command to set the orientation of the display
 * by writing the provided data to the MADCTL register.
 *
 * @param data The orientation data to be set. This is typically a value
 *             that configures the display's rotation and mirroring.
 */
void set_orientation(uint8_t data) {
    send_cmd(MADCTL);
    send_data(&data, 1);
}
/**
 * @brief Set the window area for subsequent drawing commands.
 *
 * This function sets the rectangular area on the display where the next drawing
 * commands will take effect. The coordinates are adjusted to ensure they are
 * within the valid range of the display dimensions.
 *
 * @param x0 The starting x-coordinate of the window.
 * @param x1 The ending x-coordinate of the window.
 * @param y0 The starting y-coordinate of the window.
 * @param y1 The ending y-coordinate of the window.
 *
 * The coordinates are clamped to the display dimensions defined by TFT_WIDTH
 * and TFT_HEIGHT. The offsets X_OFFSET and Y_OFFSET are added to the coordinates
 * before sending the commands to the display.
 */

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

/**
 * @brief Initializes the ST7789 display controller.
 *
 * This function performs the necessary initialization sequence for the ST7789
 * display controller, including SPI and GPIO initialization, sending commands
 * to configure the display, and setting the backlight brightness.
 *
 * The initialization sequence includes:
 * - Initializing SPI and GPIO interfaces.
 * - Resetting the display.
 * - Exiting sleep mode.
 * - Setting the color mode.
 * - Setting the display orientation.
 * - Configuring porch control.
 * - Setting the gate control.
 * - Setting the VCOMS voltage.
 * - Enabling display inversion.
 * - Setting the display to normal mode.
 * - Turning on the display.
 * - Setting the backlight brightness.
 *
 * @note This function uses FreeRTOS delay functions to ensure proper timing
 *       between commands.
 */
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

/**
 * @brief Initializes the SPI bus and adds the SPI device.
 *
 * This function configures the SPI bus with the specified settings and adds the SPI device to the bus.
 * It sets up the MOSI, SCLK, and other necessary pins, as well as the maximum transfer size.
 * The SPI device is configured with a clock speed of 80 MHz, mode 0, and other relevant settings.
 *
 * @note This function uses the ESP-IDF SPI driver and checks for errors during initialization.
 *
 * @param None
 *
 * @return None
 */
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
        .clock_speed_hz = 80 * 1000 * 1000, 
        .mode = 0,
        .spics_io_num = TFT_CS,
        .queue_size = 9,
        .flags = SPI_DEVICE_NO_DUMMY
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));
}



/**
 * @brief Convert RGB888 color format to RGB565 color format.
 *
 * This function takes 8-bit red, green, and blue color components (RGB888)
 * and converts them to a 16-bit RGB565 color format.
 *
 * @param r The red component (0-255).
 * @param g The green component (0-255).
 * @param b The blue component (0-255).
 * @return The 16-bit RGB565 color value.
 */
uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}



/**
 * @brief Sends a buffer of color data to the display.
 *
 * This function takes an array of 16-bit color values and sends them to the display
 * in chunks. Each color value is split into two bytes and stored in a temporary buffer
 * before being sent.
 *
 * @param color Pointer to an array of 16-bit color values.
 * @param size Number of color values in the array.
 */
void send_color(uint16_t *color, uint16_t size) {
    static uint8_t byte_buffer[1024]; 
    const uint16_t chunk_size = 512;
    uint16_t sent = 0;

    while (sent < size) {
        uint16_t remaining = size - sent;
        uint16_t current_chunk = (remaining > chunk_size) ? chunk_size : remaining;
        uint16_t index = 0;

        for (uint16_t i = 0; i < current_chunk; i++) {
            uint16_t c = color[sent + i];
            byte_buffer[index++] = (c >> 8) & 0xFF;
            byte_buffer[index++] = c & 0xFF;
        }

        send_data(byte_buffer, current_chunk * 2);
        sent += current_chunk;
    }
}

/**
 * @brief Clears the frame buffer by filling it with the specified color.
 *
 * This function iterates over the entire frame buffer and sets each pixel to the given color.
 *
 * @param color The color to fill the frame buffer with. The color is represented as a 16-bit value.
 */
void clear_frame_buffer(uint16_t color) {
    for (uint32_t i = 0; i < TFT_WIDTH * TFT_HEIGHT; i++) {
        frame_buffer[i] = color;
    }
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
void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= TFT_WIDTH || y >= TFT_HEIGHT) return;
    frame_buffer[y * TFT_WIDTH + x] = color;
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

    x1 = (x1 < TFT_WIDTH) ? x1 : TFT_WIDTH - 1;
    x2 = (x2 < TFT_WIDTH) ? x2 : TFT_WIDTH - 1;
    y1 = (y1 < TFT_HEIGHT) ? y1 : TFT_HEIGHT - 1;
    y2 = (y2 < TFT_HEIGHT) ? y2 : TFT_HEIGHT - 1;

    if (x1 > x2) { uint16_t t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { uint16_t t = y1; y1 = y2; y2 = t; }

    for (uint16_t y = y1; y <= y2; y++) {
        for (uint16_t x = x1; x <= x2; x++) {
            frame_buffer[y * TFT_WIDTH + x] = color;
        }
    }
}

/**
 * @brief Flushes the entire frame buffer to the display.
 *
 * This function sets the window to cover the entire display area and sends the
 * frame buffer content to the display using the RAMWR command.
 */
void flush_frame_buffer() {
    set_window(0, TFT_WIDTH - 1, 0, TFT_HEIGHT - 1);
    send_cmd(RAMWR);
    send_color(frame_buffer, TFT_WIDTH * TFT_HEIGHT);
}


/**
 * @brief Loads an image from a file and displays it on the screen.
 *
 * This function reads an image from the specified file path and displays it
 * on the screen using the ST7789 display driver. The image is expected to be
 * in a raw format with a width of IMAGE_WIDTH and a height of IMAGE_HEIGHT.
 *
 * @param path The file path to the image to be loaded.
 *
 * The function performs the following steps:
 * 1. Opens the file at the specified path in binary read mode.
 * 2. Allocates a buffer to hold the image data.
 * 3. Reads the image data from the file into the buffer.
 * 4. Closes the file.
 * 5. Iterates over each column of the image, setting the display window and
 *    sending the image data to the display.
 * 6. Frees the allocated image buffer.
 *
 * Note: Ensure that IMAGE_WIDTH and IMAGE_HEIGHT are defined appropriately
 *       for the image being loaded.
 */
void load_image(const char* path) {
    FILE* file = fopen(path, "rb");
    uint16_t* img_buf = malloc(TFT_WIDTH * TFT_HEIGHT * 2);
    
    fread(img_buf, 2, TFT_WIDTH * TFT_HEIGHT, file);
    fclose(file);

    for (int x = 0; x < TFT_WIDTH; x++) {
        set_window(x, x, 0, TFT_HEIGHT-1);
        send_cmd(RAMWR);
        send_color(&img_buf[x * TFT_HEIGHT], TFT_HEIGHT);
    }
    
    free(img_buf);
}


/**
 * @brief Draws a scaled character on the display.
 *
 * This function draws a character at the specified coordinates with the given
 * color and scale factor. The character is drawn using a font array.
 *
 * @param x The x-coordinate where the character will be drawn.
 * @param y The y-coordinate where the character will be drawn.
 * @param c The character to be drawn.
 * @param color The color of the character.
 * @param scale The scale factor for the character size.
 * @param font A pointer to the font array.
 *
 * @note The function does nothing if the character is outside the font range.
 */

void draw_char_scaled(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale, uint8_t *font) {
    if (c < FONT_START || c > FONT_END) return; 

    uint8_t *glyph = &font[(c - FONT_START) * FONT_HEIGHT]; 

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


/**
 * @brief Draws scaled text on the display.
 *
 * This function draws a string of text on the display at the specified
 * coordinates, with the specified color and scale. The text is drawn
 * using the provided font data.
 *
 * @param x The x-coordinate where the text should start.
 * @param y The y-coordinate where the text should start.
 * @param text The string of text to be drawn.
 * @param color The color of the text.
 * @param scale The scale factor for the text size.
 * @param font_data Pointer to the font data used for rendering the text.
 */
void draw_text_scaled(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t scale, uint8_t *font_data) {
    
    uint16_t cursor_x = x;

    while (*text) {
        if (*text == '\n') {
            y += (FONT_HEIGHT + 2) * scale;
            cursor_x = x;
        } else {
            draw_char_scaled(cursor_x, y, *text, color, scale, font_data);
            cursor_x += FONT_WIDTH * scale;
        }
        text++;
    }
}