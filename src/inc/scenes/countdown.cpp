/*
Skirmish ESP32 Firmware

User Interface - Countdown scene

Copyright (C) 2023 Ole Lange
*/

#include "../scenes/countdown.h"

#include <stdio.h>   // included for mocking, maybe required to remove
#include <stdlib.h>  // included for mocking, maybe required to remove
#include <string.h>  // included for mocking, maybe required to remove

#include "../../conf.h"
#include "../../theme.h"
#include "../audio.h"
#include "../const.h"
#include "../hardware_control.h"
#include "../hitpoint.h"
#include "../log.h"
#include "../time.h"

/**
 * Constructor of the scene. Sets the ID
 */
CountdownScene::CountdownScene(SkirmishUI *ui) : SkirmishUIScene(ui) {
    this->id = SCENE_COUNTDOWN;
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

#ifndef NO_AUDIO
        if (secLeft == 3) {
            audioBegin("/three.wav");
        } else if (secLeft == 2) {
            audioBegin("/two.wav");
        } else if (secLeft == 1) {
            audioBegin("/one.wav");
        } else if (secLeft == 0) {
            audioBegin("/fight.wav");
        }
#endif
        hardwareVibrate(150);

        return true;  // render if countdown has changed
    }

    // if the countdown is finished change to the game scene
    if (secLeft <= 0) {
        hardwareVibrate(300);
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
    ui->display->setFont(SDT_HEADER_FONT);
    ui->display->setTextColor(SDT_TEXT_COLOR);

    ui->display->centerText(countdown, 200, 3, true, SDT_BG_COLOR);
#endif
}
