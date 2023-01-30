/*
Skirmish ESP32 Firmware

Skirmish Communication - header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <ArduinoJson.h>

#include <inc/bluetooth.h>
#include <inc/game.h>
#include <inc/ui.h>

class SkirmCom {

    private:
        SkirmishBluetooth *bleDriver;
        Game *game;
        SkirmishUI *ui;

        DynamicJsonDocument *jsonOutDocument;

    public:
        SkirmCom(SkirmishBluetooth *bleDriver, Game *game, SkirmishUI *ui);

        void init();
        
        static void onReceiveCallback(void *context, DynamicJsonDocument *data);
        void onReceive(DynamicJsonDocument *data);

        static void onConnectCallback(void *context);
        void onConnect();

        static void onDisconnectCallback(void *context);
        void onDisconnect();

        void shotFired(uint16_t sid);
        void gotHit(uint8_t pid, uint16_t sid, uint8_t hitLocation);

};