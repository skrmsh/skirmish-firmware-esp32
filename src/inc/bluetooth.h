/*
Skirmish ESP32 Firmware

Bluetooth Low Energy driver - header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <ArduinoJson.h>


class SkirmishBluetooth {

    private:
        BLECharacteristic *writeCharacteristic;
        BLECharacteristic *readCharacteristic;
        BLECharacteristic *imageCharacteristic;

        BLEServer *server;

        char* bluetoothName;
        bool isConnected = false;

        char dataBuffer[512];

    public:
        SkirmishBluetooth();
        void init();

        void setCom(void *com);

        void startAdvertising();

        bool getConnectionState();
        void setConnectionState(bool newState);

        char* getName();
        void writeJsonData(DynamicJsonDocument *data);

        void *com;
        void (*onReceiveCallback)(void *context, DynamicJsonDocument *);
        void setOnReceiveCallback(void(* callback)(void *context, DynamicJsonDocument*));
};