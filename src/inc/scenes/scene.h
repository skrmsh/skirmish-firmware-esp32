/*
Skirmish ESP32 Firmware

Scene base class

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <inc/ui.h>

/**
 * Classes inherited from this class are drawing the
 * scenes
 */
class SkirmishUIScene {
   protected:
    uint8_t id;

   public:
    SkirmishUIScene(SkirmishUI *ui);

    uint8_t getID();

    SkirmishUI *ui;

    virtual void onSet(uint8_t id);
    virtual bool update();
    virtual void render();
};
