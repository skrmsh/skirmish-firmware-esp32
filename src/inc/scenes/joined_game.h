/*
Skirmish ESP32 Firmware

User Interface - Joined Game scene

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <inc/scenes/scene.h>

class JoinedGameScene : public SkirmishUIScene {
    public:

    JoinedGameScene(SkirmishUI *ui);

    void onSet(uint8_t id);
    bool update();
    void render();

};