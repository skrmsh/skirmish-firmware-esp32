 /*
Skirmish ESP32 Firmware

User Interface - Joined Game scene

Copyright (C) 2023 Ole Lange
*/


#include <inc/scenes/game.h>
#include <theme.h>
#include <inc/const.h>
#include <inc/hitpoint.h>
#include <inc/hardware_control.h>
#include <fonts/bitmaps.h>
#include <conf.h>

/**
 * Constructor of the scene. Sets the ID
*/
GameScene::GameScene(SkirmishUI *ui) : SkirmishUIScene(ui) {
    this->id = SCENE_JOINED_GAME;
}

/**
 * Is called when the scene is set
*/
void GameScene::onSet(uint8_t id) {
}

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
        hasHitMsgBoxUntil = mnow + 2000;
        ui->game->player.hasHit = false;
        return true;
    }

    // Hide hitbox after the specified delay
    if (mnow > hasHitMsgBoxUntil && hasHitMsgBoxUntil != 0) {
        hasHitMsgBoxUntil = 0;
        ui->clearRequired = true;
        return true;
    }

    // Re-Render if the canFire state changed
    prevCanFire = canFire;
    canFire = ui->game->player.canFire();
    if (canFire != prevCanFire) {
        return true;
    }

    if (ui->game->wasDataUpdated |
        ui->game->player.wasDataUpdated |
        ui->game->team.wasDataUpdated) {

        ui->game->wasDataUpdated = false;
        ui->game->player.wasDataUpdated = false;
        ui->game->team.wasDataUpdated = false;
        
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
    }
    else {
        if (!ui->game->player.isInviolable()) {
            hitpointSelectAnimation(HP_ANIM_SOLID);
            hitpointSetColor(player_r, player_g, player_b);
        }
        else {
            hitpointSelectAnimation(HP_ANIM_SOLID);
            hitpointSetColor(0, 0, 0);
        }
    }

    // Drawing game name
    if (strlen(ui->game->gid) < 10) // Selecting font size based on length
        ui->display->setFont(SDT_HEADER_FONT);
    else
        ui->display->setFont(SDT_SUBHEADER_FONT);

    ui->display->setTextColor(SDT_PRIMARY_COLOR);
    ui->display->centerText(ui->game->gid, 55, 1);

    // Drawing player name
    ui->display->setFont(SDT_SUBHEADER_FONT);
    ui->display->setTextColor(SDT_PRIMARY_COLOR);
    ui->display->centerText(ui->game->player.name, 80, 1);

    // Drawing team name
    if (ui->game->team.tid != 0) { // Team ID 0 -> No Team
        ui->display->setFont(SDT_SUBHEADER_FONT);
        ui->display->setTextColor(SDT_PRIMARY_COLOR);
        ui->display->centerText(ui->game->team.name, 100, 1);
    }

    // Drawing player ranking (only if no has hit msg is visible)
    if (hasHitMsgBoxUntil == 0) {
        char rankingString[8];
        sprintf(rankingString, "%d/%d", ui->game->player.rank, ui->game->playerCount);
        ui->display->setFont(SDT_HEADER_FONT);
        ui->display->setTextColor(SDT_PRIMARY_COLOR);
        ui->display->centerText(rankingString, 160, 1, true, SDT_BG_COLOR);

        // Drawing team ranking
        if (ui->game->team.tid != 0) { // Team ID 0 -> No Team
            sprintf(rankingString, "%d/%d", ui->game->team.rank, ui->game->teamCount);
            ui->display->setFont(SDT_SUBHEADER_FONT);
            ui->display->setTextColor(SDT_PRIMARY_COLOR);
            ui->display->centerText(rankingString, 185, 1, true, SDT_BG_COLOR);
        }
    }

     // Clear line with ammo and points display
     ui->display->tft.fillRect(0, 210, 240, 20, ui->display->gammaCorrection(SDT_BG_COLOR));

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
    ui->display->tft.setCursor(15, healthY-3);
    ui->display->tft.printf("\x03");
    for (int j = 0; j < 10; j ++) {
        for (int i = 0; i < barWidth; i ++) {
            if ((j * barWidth) + i < ((float) ui->game->player.health * (barWidth / 10.0))) 
            color = SDT_HEALTH_COLOR;
            else color = SDT_HEALTH_BG_COLOR;
            ui->display->tft.drawLine(healthX + 10 + i + ((barWidth + barSpace) * j), healthY, healthX + i + ((barWidth + barSpace) * j), healthY + barHeight, ui->display->gammaCorrection(color));
        }
    }

    // Clearing symbol containers
    ui->display->tft.fillRect(0, 260, 240, 60, ui->display->gammaCorrection(SDT_BG_COLOR));

    // Drawing symbol containers
    ui->display->tft.drawLine(0, 260, 240, 260, ui->display->gammaCorrection(SDT_TEXT_COLOR));
    ui->display->tft.drawLine(59, 260, 59, 320, ui->display->gammaCorrection(SDT_TEXT_COLOR));
    ui->display->tft.drawLine(120, 260, 120, 320, ui->display->gammaCorrection(SDT_TEXT_COLOR));
    ui->display->tft.drawLine(181, 260, 181, 320, ui->display->gammaCorrection(SDT_TEXT_COLOR));

    // Drawing Phaser Enable Symbol
    ui->display->tft.drawBitmap(6, 266, bitmapPhaser, 48, 48, ui->display->gammaCorrection(SDT_PRIMARY_COLOR));

    // Drawling line over it if canFire is false
    if (!canFire) {
        ui->display->tft.drawLine(6, 312, 52, 266, 0xffff);
        ui->display->tft.drawLine(6, 313, 53, 266, 0xffff);
        ui->display->tft.drawLine(6, 314, 54, 266, 0xffff);
        ui->display->tft.drawLine(7, 314, 54, 267, 0xffff);
        ui->display->tft.drawLine(8, 314, 54, 268, 0xffff);
    }

    // Drawing Inviolable Symbol
    ui->display->tft.drawBitmap(66, 266, bitmapShield, 48, 48, ui->display->gammaCorrection(SDT_PRIMARY_COLOR));
    if (!ui->game->player.isInviolable()) {
        ui->display->tft.drawLine(66, 312, 112, 266, 0xffff);
        ui->display->tft.drawLine(66, 313, 113, 266, 0xffff);
        ui->display->tft.drawLine(66, 314, 114, 266, 0xffff);
        ui->display->tft.drawLine(67, 314, 114, 267, 0xffff);
        ui->display->tft.drawLine(68, 314, 114, 268, 0xffff);
    }

    // Drawing Ammo Limit Symbol
    ui->display->tft.drawBitmap(126, 266, bitmapAmmo, 48, 48, ui->display->gammaCorrection(SDT_PRIMARY_COLOR));
    if (!ui->game->player.ammoLimit) {
        ui->display->tft.drawLine(126, 312, 172, 266, 0xffff);
        ui->display->tft.drawLine(126, 313, 173, 266, 0xffff);
        ui->display->tft.drawLine(126, 314, 174, 266, 0xffff);
        ui->display->tft.drawLine(127, 314, 174, 267, 0xffff);
        ui->display->tft.drawLine(128, 314, 174, 268, 0xffff);
    }

    // Drawing Additional Symbol field
    // TODO

    // Drawing msg box showing if you've hit someone
    if (hasHitMsgBoxUntil > 0) {
        ui->display->tft.fillRect(35, 100, 170, 80, ui->display->gammaCorrection(SDT_BG_COLOR));
        ui->display->tft.drawRect(35, 100, 170, 80, ui->display->gammaCorrection(SDT_SECONDARY_COLOR));

        // Drawing player name
        ui->display->setFont(SDT_HEADER_FONT);
        ui->display->setTextColor(SDT_PRIMARY_COLOR);
        ui->display->centerText("Hit!", 140, 1);

        ui->display->setFont(SDT_SUBHEADER_FONT);
        ui->display->setTextColor(SDT_TEXT_COLOR);
        ui->display->centerText(ui->game->player.hasHitName, 170, 1);
    }

}