/*
Skirmish ESP32 Firmware

Logging utility

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <Arduino.h>

void logInit();

void logDebug(const char* val, ...);
void logInfo(const char* val, ...);
void logWarn(const char* val, ...);
void logError(const char* val, ...);
void logFatal(const char* val, ...);