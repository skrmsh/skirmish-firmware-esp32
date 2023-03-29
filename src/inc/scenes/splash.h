/*
Skirmish ESP32 Firmware

User Interface - Splashscreen scene

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <inc/scenes/scene.h>
#include <qrcode.h>
#include <stdint.h>

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