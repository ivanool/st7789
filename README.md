# ST7789 Driver for TTGO T-Display

A high-performance SPI driver for the ST7789 LCD controller on the TTGO T-Display (ESP32). This driver provides essential functions to control the display, draw graphics, render text, and manage the backlight.

![TTGO T-Display](https://i.imgur.com/example_image.png)

## Table of Contents
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [Credits](#credits)
- [License](#license)

---

## Features
- **Display Initialization**: Full setup sequence for ST7789.
- **Backlight Control**: PWM-based brightness adjustment.
- **Graphics Primitives**: Draw pixels, rectangles, and text.
- **Image Loading**: Support for loading images from SPIFFS.
- **Font Rendering**: Custom font support (8x12 font included).
- **SPI Optimization**: High-speed SPI transfers (40 MHz).

---

## Hardware Requirements
- TTGO T-Display board (ESP32 with integrated ST7789 display).
- ESP-IDF development environment (v4.4+ recommended).
- SPIFFS configured for font storage.

---

## Installation
1. **Clone Repository**:
   ```bash
   git clone https://github.com/your_repo/st7789_ttgo_driver.git
