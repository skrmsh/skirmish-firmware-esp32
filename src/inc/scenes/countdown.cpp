/*
Skirmish ESP32 Firmware

User Interface - Countdown scene

Copyright (C) 2023 Ole Lange
*/

#include <conf.h>
#include <inc/const.h>
#include <inc/hitpoint.h>
#include <inc/log.h>
#include <inc/scenes/countdown.h>
#include <inc/time.h>
#include <theme.h>

/**
 * Constructor of the scene. Sets the ID
 */
CountdownScene::CountdownScene(SkirmishUI *ui) : SkirmishUIScene(ui) {
    this->id = SCENE_JOINED_GAME;
    countdown = (char *)malloc(3 * sizeof(char));
    strcpy(countdown, "  ");
}

/**
 * Is called when the scene is set
 */
void CountdownScene::onSet(uint8_t id) {}

/**
 * Updates the splashscreen scene
 */
bool CountdownScene::update() {
    secLeft = ui->game->startTime - getCurrentTS();
    if (secLeft >= 0 && secLeft != prevSecLeft) {
        prevSecLeft = secLeft;
        sprintf(countdown, "%d", secLeft);

        return true;  // render if countdown has changed
    }

    // if the countdown is finished change to the game scene
    if (secLeft == 0) {
        ui->setScene(SCENE_GAME);
        return false;
    }

    return false;
}

/**
 * Renders the scene
 */
void CountdownScene::render() {
#ifndef NO_DISPLAY
    ui->display->setFont(SDT_SUBHEADER_FONT);
    ui->display->setTextColor(SDT_TEXT_COLOR);
    ui->display->centerText("START  IN", 50, 1);
    ui->display->centerText("SECONDS", 280, 1);

    ui->display->setFont(SDT_HEADER_FONT);
    ui->display->setTextColor(SDT_PRIMARY_COLOR);

    ui->display->centerText(countdown, 185, 3, true, SDT_BG_COLOR);
#endif
}