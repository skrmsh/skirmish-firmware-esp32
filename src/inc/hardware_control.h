/*
Skirmish ESP32 Firmware

Hardware Control - Header file

This module controls hardware functions which are
not extensive enough to be controlled in a separate module.

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <Arduino.h>

bool hardwareWasTriggerPressed();

void hardwarePowerOff();
uint16_t hardwareReadVBAT();
float hardwareBatteryPercent();
void hardwareVibrate(uint16_t duration);

void hardwareLoop();
void hardwareInit();