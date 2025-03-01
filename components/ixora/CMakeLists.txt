idf_component_register(
    SRCS "src/ixora.c"
    INCLUDE_DIRS "include" "../ixora/include"
    REQUIRES driver spiffs st7789 mbedtls
)


# Si tienes un directorio `spiffs` con los archivos que quieres subir a SPIFFS
spiffs_create_partition_image(storage ${PROJECT_DIR}/spiffs_image FLASH_IN_PROJECT)
