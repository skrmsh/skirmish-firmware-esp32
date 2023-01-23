 /*
Skirmish ESP32 Firmware

User Interface - Splashscreen scene

Copyright (C) 2023 Ole Lange
*/


#include <inc/scenes/splash.h>
#include <theme.h>
#include <inc/const.h>
#include <inc/hitpoint.h>
#include <conf.h>

/**
 * Constructor of the scene. Sets the ID
*/
SplashscreenScene::SplashscreenScene(SkirmishUI *ui) : SkirmishUIScene(ui) {
    this->id = SCENE_SPLASHSCREEN;

    splashText = (char*) malloc(32 * sizeof(char));
}

/**
 * Is called when the scene is set
*/
void SplashscreenScene::onSet(uint8_t id) {
    this->id = id;
    if (id == SCENE_SPLASHSCREEN) {
        strcpy(splashText, "Loading...");
    }
    else if (id == SCENE_BLE_CONNECT) {
        strcpy(splashText, "Waiting for connection...");

        // Setting hitpoints to breathe animation
        // to indicate that the device is waiting
        // for connection
        hitpointSelectAnimation(HP_ANIM_BREATHE);
        hitpointSetAnimationSpeed(2);
        hitpointSetColor(SDT_PRIMARY_COLOR_RGB);
    }
    else if (id == SCENE_NO_GAME) {
        strcpy(splashText, "Please join a game!");

        // turn off leds
        hitpointSelectAnimation(HP_ANIM_SOLID);
        hitpointSetColor(0, 0, 0);
    }
    else {
        strcpy(splashText, "<INVALID SCENE>");
    }
}

/**
 * Updates the splashscreen scene
*/
bool SplashscreenScene::update() {
    return false;
}

/**
 * Renders the scene
*/
void SplashscreenScene::render() {
    ui->display->setFont(SDT_HEADER_FONT);

    ui->display->setTextColor(SDT_PRIMARY_COLOR);
    ui->display->centerText("SKIRMISH", 140, SDT_HEADER_FONT_SIZE);

    ui->display->setFont(SDT_TEXT_FONT);
    ui->display->setTextColor(SDT_TEXT_COLOR);
    ui->display->centerText(splashText, 160, 1);
}