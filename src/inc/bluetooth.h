/*
Skirmish ESP32 Firmware

Bluetooth Low Energy driver - header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

class SkirmishBluetooth {
   private:
    BLECharacteristic *writeCharacteristic;
    BLECharacteristic *readCharacteristic;
    BLECharacteristic *imageCharacteristic;

    BLEServer *server;

    char *bluetoothName;
    bool isConnected = false;

    char dataBuffer[512];

   public:
    SkirmishBluetooth();
    void init();

    void setCom(void *com);

    void startAdvertising();

    bool getConnectionState();
    void setConnectionState(bool newState);

    uint32_t lastDisconnectedTime = 0;

    char *getName();
    void writeJsonData(DynamicJsonDocument *data);

    void *com;
    void (*onReceiveCallback)(void *context, DynamicJsonDocument *);
    void setOnReceiveCallback(void (*callback)(void *context,
                                               DynamicJsonDocument *));

    void (*onConnectCallback)(void *context);
    void setOnConnectCallback(void (*callback)(void *context));
    void (*onDisconnectCallback)(void *context);
    void setOnDisconnectCallback(void (*callback)(void *context));
};