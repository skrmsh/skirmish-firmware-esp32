/*
Skirmish ESP32 Firmware

User Interface - Header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <stdint.h>

#include "bluetooth.h"
#include "display.h"
#include "game.h"

/**
 * Forward definitions of scene classes
 */
class SkirmishUIScene;
class SplashscreenScene;
class JoinedGameScene;
class CountdownScene;
class GameScene;

#define BORDER_TYPE_DEFAULT 0
#define BORDER_TYPE_GAME 1

/**
 * This class renders the user interface on
 */
class SkirmishUI {
   private:
    bool renderRequired = true;
    uint32_t nextScheduledUpdate = 0;

    uint32_t msgBoxVisibleUntil = 0;
    char *msgBoxText;
    char *msgBoxHeading;

    void renderStatusOverlay();
    void renderMsgBox();

    bool bluetoothIsConnected;
    bool prevBluetoothIsConnected;

    // scenes
    SkirmishUIScene *currentScene;
    SplashscreenScene *splashscreenScene;
    JoinedGameScene *joinedGameScene;
    CountdownScene *countdownScene;
    GameScene *gameScene;

   public:
    SkirmishDisplay *display;
    Game *game;
    SkirmishBluetooth *bluetooth;

    uint8_t stbR, stbG, stbB;

    bool clearRequired = false;

    SkirmishUI(SkirmishDisplay *display, SkirmishBluetooth *bluetooth,
               Game *game);

    void setRenderingRequired();
    void setScene(uint8_t scene);

    void setStandbyColor(uint8_t r, uint8_t g, uint8_t b);

    void border(uint8_t type, uint16_t color);

    void msgBox(char *heading, char *text, uint32_t timeout);
    bool msgBoxVisible = false;

    void update();
    void render();
};
