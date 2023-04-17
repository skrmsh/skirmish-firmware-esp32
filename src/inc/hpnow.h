/*
Skirmish ESP32 Firmware

ESPNOW Hitpoint Communication driver

Copyright (C) 2023 Ole Lange
*/

#pragma once
#include <stdint.h>

#define CMD_SYS_INIT 0x01
#define CMD_HIT_VALID 0x02
#define CMD_GOT_HIT 0x03

void hpnowInit();

void hpnowSysInit(uint8_t hpMode, uint8_t color_r, uint8_t color_g, uint8_t color_b);
void hpnowHitValid(uint8_t hpMode, uint8_t pid, uint16_t sid, uint8_t cooldown);

bool hpnowGotHit();
uint8_t hpnowGotHitHPMode();
uint8_t hpnowGotHitPID();
uint16_t hpnowGotHitSID();