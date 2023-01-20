/*
Skirmish ESP32 Firmware

Infrared Driver

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <stdint.h>

void infraredInit();
void infraredTransmitShot(uint8_t playerID, uint32_t shotID);