/*
Skirmish ESP32 Firmware

User Interface - Splashscreen scene

Copyright (C) 2023 Ole Lange
*/

#include "splash.h"

#include <stdlib.h>  // included for mocking, maybe required to remove
#include <string.h>  // included for mocking, maybe required to remove

#include "../../conf.h"
#include "../../theme.h"
#include "../const.h"
#include "../hitpoint.h"
// #include "../qrcode.h"  // MOCK: REMOVE

/**
 * Constructor of the scene. Sets the ID
 */
SplashscreenScene::SplashscreenScene(SkirmishUI *ui) : SkirmishUIScene(ui) {
    this->id = SCENE_SPLASHSCREEN;

    splashText = (char *)malloc(32 * sizeof(char));

    qrBytes = (uint8_t *)malloc(qrcode_getBufferSize(3) * sizeof(uint8_t));
}

/**
 * Is called when the scene is set
 */
void SplashscreenScene::onSet(uint8_t id) {
    this->id = id;
    if (id == SCENE_SPLASHSCREEN) {
        strcpy(splashText, "Loading...");
    } else if (id == SCENE_BLE_CONNECT) {
        strcpy(splashText, "Waiting for connection...");

        // Setting hitpoints to breathe animation
        // to indicate that the device is waiting
        // for connection
        hitpointSelectAnimation(HP_ANIM_BREATHE);
        hitpointSetAnimationSpeed(2);
        hitpointSetColor(SDT_PRIMARY_COLOR_RGB);

        // Generating QR Code
        qrcode_initText(&nameQR, qrBytes, 3, ECC_LOW, ui->bluetooth->getName());
    } else if (id == SCENE_BLE_RECONNECT) {
        strcpy(splashText, "Please re-connect!");

        hitpointSelectAnimation(HP_ANIM_ROTATE);
        hitpointSetAnimationSpeed(8);
        hitpointSetColor(0, 0, 255);

    } else if (id == SCENE_NO_GAME) {
        strcpy(splashText, "Please join a game!");

        // turn off leds
        hitpointSelectAnimation(HP_ANIM_SOLID);
        hitpointSetColor(0, 0, 0);
    } else {
        strcpy(splashText, "<INVALID SCENE>");
    }
}

/**
 * Updates the splashscreen scene
 */
bool SplashscreenScene::update() { return false; }

/**
 * Renders the scene
 */
void SplashscreenScene::render() {
#ifndef NO_DISPLAY
    ui->display->setFont(SDT_HEADER_FONT);

    uint8_t base_y = 170;
    if (id == SCENE_BLE_CONNECT) base_y = 80;

    ui->display->setTextColor((id == SCENE_BLE_RECONNECT) ? SDT_SECONDARY_COLOR
                                                          : SDT_PRIMARY_COLOR);
    ui->display->centerText("SKIRMISH", base_y, SDT_HEADER_FONT_SIZE);

    ui->display->setFont(SDT_TEXT_FONT);
    ui->display->setTextColor(SDT_TEXT_COLOR);
    ui->display->centerText(splashText, base_y + 5, 1);

    // Render a qr code to the connect scene
    if (id == SCENE_BLE_CONNECT) {
        uint8_t pos_x = 120 - (nameQR.size * 5) / 2;
        for (uint8_t y = 0; y < nameQR.size; y++) {
            for (uint8_t x = 0; x < nameQR.size; x++) {
                if (qrcode_getModule(&nameQR, x, y)) {
                    ui->display->tft.fillRect(
                        pos_x + (x * 5), base_y + 55 + (y * 5), 5, 5,
                        ui->display->gammaCorrection(SDT_TEXT_COLOR));
                } else {
                    ui->display->tft.fillRect(
                        pos_x + (x * 5), base_y + 55 + (y * 5), 5, 5,
                        ui->display->gammaCorrection(SDT_BG_COLOR));
                }
            }
        }
    }
#endif
}
