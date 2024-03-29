/*
Skirmish ESP32 Firmware

User Interface - Joined Game scene

Copyright (C) 2023 Ole Lange
*/

#include "game.h"

#include <stdio.h>   // included for mocking, maybe required to remove
#include <stdlib.h>  // included for mocking, maybe required to remove
#include <string.h>  // included for mocking, maybe required to remove

#include "../../conf.h"
#include "../../fonts/bitmaps.h"
#include "../../theme.h"
#include "../const.h"
#include "../hardware_control.h"
#include "../hitpoint.h"
// #include "../mocks.h"  // MOCK: REMOVE

/**
 * Constructor of the scene. Sets the ID
 */
GameScene::GameScene(SkirmishUI *ui) : SkirmishUIScene(ui) {
    this->id = SCENE_GAME;

    knownPlayerName = (char *)malloc(33 * sizeof(char));
    knownTeamName = (char *)malloc(33 * sizeof(char));
}

/**
 * Is called when the scene is set
 */
void GameScene::onSet(uint8_t id) {}

/**
 * Updates the splashscreen scene
 */
bool GameScene::update() {
    uint32_t mnow = millis();

    // Start blinking if the player got hit
    if (ui->game->player.wasHit) {
        hardwareVibrate(150);
        hitBlinkUntil = mnow + 1500;
        ui->game->player.wasHit = false;
        return true;
    }

    // Stop blinking after the specified delay
    if (mnow > hitBlinkUntil && hitBlinkUntil != 0) {
        hitBlinkUntil = 0;
        return true;
    }

    // Show msgbox if the player has hit
    if (ui->game->player.hasHit) {
        ui->msgBox("Hit!", ui->game->player.hasHitName, 2000);
        ui->game->player.hasHit = false;
    }

    // Re-Render if the canFire state changed
    prevCanFire = canFire;
    canFire = ui->game->player.canFire();
    if (canFire != prevCanFire) {
        return true;
    }

    if (ui->game->wasDataUpdated | ui->game->player.wasDataUpdated |
        ui->game->team.wasDataUpdated) {
        ui->game->wasDataUpdated = false;
        ui->game->player.wasDataUpdated = false;
        ui->game->team.wasDataUpdated = false;

        if (strcmp(knownPlayerName, ui->game->player.name) != 0) {
            strcpy(knownPlayerName, ui->game->player.name);
            ui->clearRequired = true;
        }

        if (strcmp(knownTeamName, ui->game->team.name) != 0) {
            strcpy(knownTeamName, ui->game->team.name);
            ui->clearRequired = true;
        }

        return true;
    }

    return false;
}

/**
 * Renders the scene
 */
void GameScene::render() {
    uint8_t player_r, player_g, player_b;

    player_r = ui->game->player.color_r;
    player_g = ui->game->player.color_g;
    player_b = ui->game->player.color_b;

    // Set hitpoint color
    if (hitBlinkUntil > 0) {
        hitpointSelectAnimation(HP_ANIM_BLINK);
        hitpointSetAnimationSpeed(15);
        hitpointSetColor(255, 255, 255);
    } else {
        if (!ui->game->player.isInviolable() ||
            !ui->game->player.inviolableLightsOff) {
            hitpointSelectAnimation(HP_ANIM_SOLID);
            hitpointSetColor(player_r, player_g, player_b);
        } else {
            hitpointSelectAnimation(HP_ANIM_SOLID);
            hitpointSetColor(0, 0, 0);
        }
    }

#ifndef NO_DISPLAY

    // Drawing player colored bar
    uint16_t playerColor = ui->display->gammaCorrection(
        ui->display->color(ui->game->player.color_r, ui->game->player.color_g,
                           ui->game->player.color_b));

    ui->display->tft.fillRect(35, 27, 170, 13, playerColor);
    ui->display->tft.fillTriangle(26, 33, 35, 27, 35, 39, playerColor);
    ui->display->tft.fillTriangle(214, 33, 205, 27, 205, 39, playerColor);

    // Drawing game name
    if (strlen(ui->game->gid) < 10)  // Selecting font size based on length
        ui->display->setFont(SDT_HEADER_FONT);
    else
        ui->display->setFont(SDT_SUBHEADER_FONT);

    ui->display->setTextColor(SDT_PRIMARY_COLOR);
    ui->display->centerText(ui->game->gid, 75, 1);

    // Drawing player name
    ui->display->setFont(SDT_SUBHEADER_FONT);
    ui->display->setTextColor(SDT_PRIMARY_COLOR);
    ui->display->centerText(ui->game->player.name, 110, 1);

    // Drawing player ranking
    char rankingString[8];
    sprintf(rankingString, "%d/%d", ui->game->player.rank,
            ui->game->playerCount);
    ui->display->setFont(SDT_SUBHEADER_FONT);
    ui->display->setTextColor(SDT_SECONDARY_COLOR);
    ui->display->centerText(rankingString, 127, 1, true, SDT_BG_COLOR);

    // Drawing team name
    if (ui->game->team.tid != 0) {  // Team ID 0 -> No Team
        ui->display->setFont(SDT_SUBHEADER_FONT);
        ui->display->setTextColor(SDT_PRIMARY_COLOR);
        ui->display->centerText(ui->game->team.name, 160, 1);
    }

    // Drawing team ranking
    if (ui->game->team.tid != 0) {  // Team ID 0 -> No Team
        sprintf(rankingString, "%d/%d", ui->game->team.rank,
                ui->game->teamCount);
        ui->display->setFont(SDT_SUBHEADER_FONT);
        ui->display->setTextColor(SDT_SECONDARY_COLOR);
        ui->display->centerText(rankingString, 177, 1, true, SDT_BG_COLOR);
    }

    // Clear line with ammo and points display
    ui->display->tft.fillRect(0, 210, 240, 20,
                              ui->display->gammaCorrection(SDT_BG_COLOR));

    // Drawing Left Ammo
    if (ui->game->player.ammoLimit) {
        ui->display->setFont(SDT_TEXT_FONT);
        ui->display->tft.setTextSize(2);
        ui->display->setTextColor(SDT_TEXT_COLOR);
        ui->display->tft.setCursor(15, 210);
        ui->display->tft.printf("\x0F %d", ui->game->player.ammo);
    }

    // Drawing Points
    ui->display->setFont(SDT_TEXT_FONT);
    ui->display->tft.setTextSize(2);
    ui->display->setTextColor(SDT_TEXT_COLOR);
    ui->display->tft.setCursor(140, 210);
    ui->display->tft.printf("\x04 %05d", ui->game->player.points);

    // Drawing Health
    uint16_t healthY = 240;
    uint8_t healthX = 30;
    uint16_t color = SDT_HEALTH_COLOR;
    uint8_t barWidth = 17;
    uint8_t barHeight = 10;
    uint8_t barSpace = 2;
    ui->display->setFont(SDT_TEXT_FONT);
    ui->display->tft.setTextSize(2);
    ui->display->setTextColor(SDT_TEXT_COLOR);
    ui->display->tft.setCursor(15, healthY - 3);
    ui->display->tft.printf("\x03");
    for (int j = 0; j < 10; j++) {
        for (int i = 0; i < barWidth; i++) {
            if ((j * barWidth) + i <
                ((float)ui->game->player.health * (barWidth / 10.0)))
                color = SDT_HEALTH_COLOR;
            else
                color = SDT_HEALTH_BG_COLOR;
            ui->display->tft.drawLine(
                healthX + 10 + i + ((barWidth + barSpace) * j), healthY,
                healthX + i + ((barWidth + barSpace) * j), healthY + barHeight,
                ui->display->gammaCorrection(color));
        }
    }

    // Clearing symbol area
    ui->display->tft.fillRect(0, 260, 240, 60,
                              ui->display->gammaCorrection(SDT_BG_COLOR));

    // Drawing Phaser Enable Symbol
    ui->display->tft.drawBitmap(
        6, 266, bitmapPhaser, 48, 48,
        ui->display->gammaCorrection(canFire ? SDT_PRIMARY_COLOR
                                             : SDT_GAME_SYMBOL_DISABLED_COLOR));
    // Drawing Inviolable Symbol
    ui->display->tft.drawBitmap(
        66, 266, bitmapShield, 48, 48,
        ui->display->gammaCorrection(ui->game->player.isInviolable()
                                         ? SDT_PRIMARY_COLOR
                                         : SDT_GAME_SYMBOL_DISABLED_COLOR));
    // Drawing Ammo Limit Symbol
    ui->display->tft.drawBitmap(
        126, 266, bitmapAmmo, 48, 48,
        ui->display->gammaCorrection(ui->game->player.ammoLimit
                                         ? SDT_PRIMARY_COLOR
                                         : SDT_GAME_SYMBOL_DISABLED_COLOR));

    /*
    // Drawling line over it if canFire is false
    if (!canFire) {
        ui->display->tft.drawLine(6, 312, 52, 266, 0xffff);
        ui->display->tft.drawLine(6, 313, 53, 266, 0xffff);
        ui->display->tft.drawLine(6, 314, 54, 266, 0xffff);
        ui->display->tft.drawLine(7, 314, 54, 267, 0xffff);
        ui->display->tft.drawLine(8, 314, 54, 268, 0xffff);
    }

    if (!ui->game->player.isInviolable()) {
        ui->display->tft.drawLine(66, 312, 112, 266, 0xffff);
        ui->display->tft.drawLine(66, 313, 113, 266, 0xffff);
        ui->display->tft.drawLine(66, 314, 114, 266, 0xffff);
        ui->display->tft.drawLine(67, 314, 114, 267, 0xffff);
        ui->display->tft.drawLine(68, 314, 114, 268, 0xffff);
    }

    if (!ui->game->player.ammoLimit) {
        ui->display->tft.drawLine(126, 312, 172, 266, 0xffff);
        ui->display->tft.drawLine(126, 313, 173, 266, 0xffff);
        ui->display->tft.drawLine(126, 314, 174, 266, 0xffff);
        ui->display->tft.drawLine(127, 314, 174, 267, 0xffff);
        ui->display->tft.drawLine(128, 314, 174, 268, 0xffff);
    }
    */

    // Drawing Additional Symbol field
    // TODO

    // Drawing msg box showing if you've hit someone
#endif
}
