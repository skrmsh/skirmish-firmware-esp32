/*
Skirmish ESP32 Firmware

Display driver - Header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <conf.h>

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

/**
 * An object of this class can be used to control the ILI9341 Display
 * connected to the skirmish phaser module. It provides some methods
 * for rendering text and images and is used by the SkirmishUI class to
 * display the User Interface.
 */
class SkirmishDisplay {
   private:
    SPIClass spi = SPIClass();

   public:
    SkirmishDisplay();
    void init();

    Adafruit_ILI9341 tft =
        Adafruit_ILI9341(&spi, PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_RESET);

    // Color functions
    uint16_t gammaCorrection(uint16_t color);
    uint16_t color(uint8_t r, uint8_t g, uint8_t b);

    void setTextColor(uint16_t color);
    void clear();
    void clear(uint16_t bgColor);

    // Text functions
    void setFont(const GFXfont* f);
    void centerText(const char* text, uint16_t y, uint8_t fontSize,
                    bool clearLine, uint16_t bgColor);
    void centerText(const char* text, uint16_t y, uint8_t fontSize);
};