/*
Skirmish ESP32 Firmware

User Interface - Game scene

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <inc/scenes/scene.h>

class GameScene : public SkirmishUIScene {
   public:
    GameScene(SkirmishUI *ui);

    void onSet(uint8_t id);
    bool update();
    void render();

    uint32_t hitBlinkUntil = 0;
    uint32_t hasHitMsgBoxUntil = 0;

    bool prevCanFire = false;
    bool canFire = false;
};