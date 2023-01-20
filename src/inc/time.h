/*
Skirmish ESP32 Firmware

Time util - header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <stdint.h>

void setCurrentTS(uint32_t ts);
uint32_t getCurrentTS();