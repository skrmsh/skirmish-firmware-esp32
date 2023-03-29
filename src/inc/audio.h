/*
Skirmish ESP32 Firmware

Audio driver - Header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

void audioInit(float gain);
void audioBegin(const char* filename);
void audioLoopTask(void* param);