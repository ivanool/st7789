#include "st7789.h"


static const char* TAG = "ixora";


/**
 * @brief Mounts the SPIFFS filesystem.
 *
 * This function configures and mounts the SPIFFS (SPI Flash File System) 
 * filesystem. If the filesystem cannot be mounted, it attempts to format 
 * it if the `format_if_mount_failed` option is set to true.
 *
 * The function logs appropriate error messages if the filesystem cannot 
 * be mounted or formatted, or if the SPIFFS partition cannot be found.
 * It also logs the total and used size of the SPIFFS partition upon 
 * successful mounting.
 *
 * @note This function uses the ESP-IDF SPIFFS API.
 */
void mount_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

/**
 * @brief Loads font data from a file into the provided buffer.
 *
 * This function opens the font file specified by FONT_FILE in binary read mode,
 * reads the font data into the provided buffer, and then closes the file.
 * 
 * @param font_data Pointer to the buffer where the font data will be loaded.
 *
 * @note The buffer size should be at least (FONT_END - FONT_START + 1) * FONT_HEIGHT bytes.
 * 
 * @warning If the file cannot be opened, an error message will be logged.
 * 
 * @return void
 */
void load_font(uint8_t *font_data) {
    FILE* file = fopen(FONT_FILE, "rb");
    if (!file) {
        ESP_LOGE("FONT", "Error al abrir el archivo de fuente");
    }

    fread(font_data, sizeof(uint8_t), (FONT_END - FONT_START + 1) * FONT_HEIGHT, file);  
    fclose(file);

    ESP_LOGI("FONT", "Fuente cargada correctamente");
}
