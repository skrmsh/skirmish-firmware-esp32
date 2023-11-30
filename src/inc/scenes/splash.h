/*
Skirmish ESP32 Firmware

User Interface - Splashscreen scene

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <stdint.h>

// #include "../qrcode.h"  // MOCK: REMOVE
#include "qrcode.h"  // MOCK: ADD
#include "scene.h"

class SplashscreenScene : public SkirmishUIScene {
   private:
    char* splashText;

    QRCode nameQR;
    uint8_t* qrBytes;

   public:
    SplashscreenScene(SkirmishUI* ui);

    void onSet(uint8_t id);
    bool update();
    void render();
};
