/*
Skirmish ESP32 Firmware

User Interface - Countdown scene

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <inc/scenes/scene.h>

class CountdownScene : public SkirmishUIScene {
    private:
    int16_t secLeft;
    int16_t prevSecLeft;
    
    char* countdown;

    public:

    CountdownScene(SkirmishUI *ui);

    void onSet(uint8_t id);
    bool update();
    void render();

};