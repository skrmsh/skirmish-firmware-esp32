/*
Skirmish ESP32 Firmware

Time util

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>
#include <inc/log.h>

bool wasTimeSet = false;

uint32_t baseTS;
uint32_t baseMillis;

uint32_t currentTS;
uint32_t currentMillis;

/**
 * Sets the current unixtimestamp
 * 
 * @param ts Timestamp (in seconds since 01.01.1970)
*/
void setCurrentTS(uint32_t ts) {
    baseTS = ts;
    baseMillis = millis();
    wasTimeSet = true;
    logDebug("Set BaseTS to %d", baseTS);
    logDebug("Set BaseMS to %d", baseMillis);
} 

/**
 * Returns the current timestamp
 * 
 * @return Timestamp (in seconds since 01.01.1970)
 * @return 0 if the time was never set
*/
uint32_t getCurrentTS() {
    if (wasTimeSet) {
        currentMillis = millis();
        currentTS = baseTS + ((currentMillis - baseMillis) / 1000);
        return currentTS;
    }
    else {
        return 0;
    }
}