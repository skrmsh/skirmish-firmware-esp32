/*
Skirmish ESP32 Firmware

ESPNOW Hitpoint Communication driver

Copyright (C) 2023 Ole Lange
*/

#pragma once
#include <stdint.h>

void hpnowInit(void (*cb)(const uint8_t *data));

void hpnowSysInit(uint8_t hpMode, uint8_t color_r, uint8_t color_g, uint8_t color_b);
void hpnowHitSuccess(uint8_t hpMode, uint8_t pid, uint16_t sid, uint8_t cooldown);