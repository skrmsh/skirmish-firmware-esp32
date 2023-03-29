/*
Skirmish ESP32 Firmware

User Interface - Joined Game scene

Copyright (C) 2023 Ole Lange
*/

#include <conf.h>
#include <inc/const.h>
#include <inc/hitpoint.h>
#include <inc/scenes/joined_game.h>
#include <theme.h>

/**
 * Constructor of the scene. Sets the ID
 */
JoinedGameScene::JoinedGameScene(SkirmishUI *ui) : SkirmishUIScene(ui) {
    this->id = SCENE_JOINED_GAME;
}

/**
 * Is called when the scene is set
 */
void JoinedGameScene::onSet(uint8_t id) {}

/**
 * Updates the splashscreen scene
 */
bool JoinedGameScene::update() {
    if (ui->game->startTime > 0) {
        ui->setScene(SCENE_COUNTDOWN);
    }

    return false;
}

/**
 * Renders the scene
 */
void JoinedGameScene::render() {
#ifndef NO_DISPLAY
    ui->display->setFont(SDT_HEADER_FONT);

    ui->display->setTextColor(SDT_PRIMARY_COLOR);
    ui->display->centerText("SKIRMISH", 140, SDT_HEADER_FONT_SIZE);

    ui->display->setFont(SDT_SUBHEADER_FONT);
    ui->display->setTextColor(SDT_TEXT_COLOR);
    ui->display->centerText(ui->game->gid, 170, 1);
#endif
}