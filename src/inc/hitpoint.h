/*
Skirmish ESP32 Firmware

Hitpoint driver - Header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <Arduino.h>

// I2C Commands
#define HP_CMD_SELECT_ANIM 0x01
#define HP_CMD_SET_ANIM_SPEED 0x02
#define HP_CMD_SET_COLOR 0x03
#define HP_CMD_TIMESYNC 0x04

void hitpointInit();

bool hitpointEventTriggered();
uint32_t hitpointReadShotRaw(uint8_t addr);
uint32_t hitpointReadShotRaw(uint8_t *addr);

void hitpointSelectAnimation(uint8_t addr, uint8_t animation);
void hitpointSelectAnimation(uint8_t animation);
void hitpointSetAnimationSpeed(uint8_t addr, uint8_t speed);
void hitpointSetAnimationSpeed(uint8_t speed);
void hitpointSetColor(uint8_t addr, uint8_t r, uint8_t g, uint8_t b);
void hitpointSetColor(uint8_t r, uint8_t g, uint8_t b);
void hitpointSyncTime();

uint8_t getPIDFromShot(uint32_t shot);
uint16_t getSIDFromShot(uint32_t shot);