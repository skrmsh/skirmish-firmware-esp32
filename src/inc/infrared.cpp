/*
Skirmish ESP32 Firmware

Infrared Driver

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>
#include <conf.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#include "inc/infrared.h"
#include "inc/log.h"

IRsend irsend(PIN_IR_LED);

void infraredInit() {
    logInfo("Init: Infrared TX");
    irsend.begin();
    logInfo(">     Init Done!");
}

/*
Transmits the given pid + sid + a checksum
via the infrared led using the nec protocol
*/
void infraredTransmitShot(uint8_t pid, uint32_t sid) {
    uint32_t irpack = (pid << 16) | (sid & 0xffff);
    uint8_t checksum = irpack % 0xff;
    irpack = (checksum << 24) | irpack;

    logDebug("Transmitting Shot:");
    logDebug("  playerID: %02x", pid);
    logDebug("  shotID:   %05x", sid);
    logDebug("  checksum: %02x", checksum);
    logDebug("  packet:   %08x", irpack);

    irsend.sendNEC(irpack);
}