/*
Skirmish ESP32 Firmware

Display driver

Copyright (C) 2023 Ole Lange
*/

#include <inc/display.h>
#include <inc/log.h>
#include <conf.h>
#include <theme.h>

#include <fonts/theNeueBlack18pt.h>

/**
 * Display class constructor
*/
SkirmishDisplay::SkirmishDisplay() {
}

#ifdef GAMMA_CORRECTION
const uint8_t PROGMEM gamma_correction_curve_g[256] = {0, 0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 12, 13, 14, 15, 16, 17, 18, 18, 19, 20, 21, 22, 23, 24, 25, 26, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 255};
const uint8_t PROGMEM gamma_correction_curve_b[256] = {0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 8, 9, 9, 10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 28, 29, 29, 30, 31, 32, 33, 33, 34, 35, 36, 37, 38, 38, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 48, 49, 50, 51, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 129, 130, 131, 132, 133, 134, 135, 136, 137, 139, 140, 141, 142, 143, 144, 145, 147, 148, 149, 150, 151, 152, 153, 155, 156, 157, 158, 159, 160, 162, 163, 164, 165, 166, 168, 169, 170, 171, 172, 173, 175, 176, 177, 178, 179, 181, 182, 183, 184, 185, 187, 188, 189, 190, 192, 193, 194, 195, 196, 198, 199, 200, 201, 203, 204, 205, 206, 207, 209, 210, 211, 212, 214, 215, 216, 217, 219, 220, 221, 222, 224, 225, 226, 228, 229, 230, 231, 233, 234, 235, 236, 238, 239, 240, 242, 243, 244, 245, 247, 248, 249, 251, 252, 253, 255};
#endif

/**
 * Gamma correction for RGB565 values.
 * 
 * @param color RGB565 color input value
 * @return RGB565 output value gamme corrected
*/
uint16_t SkirmishDisplay::gammaCorrection(uint16_t color) {
    #ifdef GAMMA_CORRECTION
    // Extracting RGB values from the color parameter
    uint8_t r = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
    uint8_t g = ((((color >> 5) & 0x3F) * 259) + 33) >> 6;
    uint8_t b = (((color & 0x1F) * 527) + 23) >> 6;

    g = gamma_correction_curve_g[g];
    b = gamma_correction_curve_b[b];

    // Creating new RGB565 color from the corrected RGB values
    uint16_t r5 = ((r >> 3) & 0x1f) << 11;
    uint16_t g6 = ((g >> 2) & 0x3f) << 5;
    uint16_t b5 = (b >> 3) & 0x1f;
    
    return (uint16_t) (r5 | g6 | b5);
    #else
    return color;
    #endif
}

uint16_t SkirmishDisplay::color(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t r5 = ((r >> 3) & 0x1f) << 11;
    uint16_t g6 = ((g >> 2) & 0x3f) << 5;
    uint16_t b5 = (b >> 3) & 0x1f;
    
    return (uint16_t) (r5 | g6 | b5);
}

/**
 * Initialises the display hardware
*/
void SkirmishDisplay::init() {
    logInfo("Init: Display");

    // SPI init
    spi.begin(PIN_TFT_SCK, -1, PIN_TFT_DATA); // Starting SPI
    spi.setFrequency(SPI_MAX_FREQ); // Setting to max speed (80MHz)
    
    // TFT init
    tft.begin();
    tft.setRotation(DISPLAY_ROTATION);
    clear();
}

/**
 * Clears the screen
*/
void SkirmishDisplay::clear() {
    clear(SDT_BG_COLOR);
}

/**
 * Clears the screen with a specific color, applies gamma correction
 * 
 * @param bgColor the background color
*/
void SkirmishDisplay::clear(uint16_t bgColor) {
    tft.fillScreen(gammaCorrection(bgColor));
}

/**
 * Sets the text color, applies gamma correction
 * 
 * @param color new text color
*/
void SkirmishDisplay::setTextColor(uint16_t color) {
    tft.setTextColor(gammaCorrection(color));
}

/**
 * Sets the font
 * 
 * @param f pointer to the new font
*/
void SkirmishDisplay::setFont(const GFXfont *f) {
    tft.setFont(f);
}

/**
 * Prints a texts centered on the screen using AdafruitGFX built-in font at
 * the specified y position
 * 
 * @param text text that will be rendered
 * @param y vertical position of the text (upper edge)
 * @param fontSize size of the font [1-7]
 * @param charWidth if a different font is used the character width is required
 * @param charHeight if a different font is used the character height is required
 * @param clearLine should the background of this text be cleared
 * @param bgColor background color to use when clearing the background
*/
void SkirmishDisplay::centerText(const char* text, uint16_t y, uint8_t fontSize,
                                 bool clearLine, uint16_t bgColor) {
    // Calculate the length of the text in characters
    uint8_t textLength = strlen(text);

    // Set the font size
    tft.setTextSize(fontSize);

    // Calculate text dimensions
    uint16_t textWidth;
    uint16_t textHeight;
    int16_t x0, y0;
    tft.getTextBounds(text, 0, y, &x0, &y0, &textWidth, &textHeight);

    // Return if the text is too wide to fit on the display
    if (textWidth > 240) return;

    // If clearLine is true, clear the line on the display
    // where the text will be drawn
    if (clearLine) {
        tft.fillRect(0, y0, 240, textHeight, gammaCorrection(bgColor));
    }
    
    // Calculate the x coordinate to center the text on the display
    uint8_t x = 120 - (textWidth / 2);
    tft.setCursor(x, y);
    // Print the text on the display
    tft.println(text);
}

void SkirmishDisplay::centerText(const char* text, uint16_t y, uint8_t fontSize) {
    centerText(text, y, fontSize, false, SDT_BG_COLOR);
}