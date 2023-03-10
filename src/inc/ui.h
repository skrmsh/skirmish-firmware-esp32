/*
Skirmish ESP32 Firmware

User Interface - Header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <inc/display.h>
#include <inc/game.h>
#include <inc/bluetooth.h>
#include <stdint.h>

/**
 * Forward definitions of scene classes
*/
class SkirmishUIScene;
class SplashscreenScene;
class JoinedGameScene;
class CountdownScene;
class GameScene;

/**
 * This class renders the user interface on 
*/
class SkirmishUI {
    private:
        bool renderRequired = true;
        uint32_t nextScheduledUpdate = 0;

        void renderStatusOverlay();

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

        bool clearRequired = false;

        SkirmishUI(SkirmishDisplay *display,
                   SkirmishBluetooth *bluetooth,
                   Game *game);

        void setRenderingRequired();
        void setScene(uint8_t scene);

        void update();
        void render();

};