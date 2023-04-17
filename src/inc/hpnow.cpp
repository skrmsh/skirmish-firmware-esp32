/*
Skirmish ESP32 Firmware

ESPNOW Hitpoint Communication driver

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include <conf.h>
#include <inc/hpnow.h>
#include <inc/log.h>

esp_now_peer_info_t broadcastReceiver;

bool receivedGotHit = false;
uint8_t gotHitHPMode = 0;
uint8_t gotHitPID = 0;
uint16_t gotHitSID = 0;

/**
 * Initialises the ESPNOW driver.
 * If it fails it tries again a second time if (isRetry is false).
 *
 * @param isRetry - doesnt retry again if it's already a retry
 */
void initESPNow(bool isRetry) {
    if (esp_now_init() == ESP_OK) {
        logInfo("-> ESPNOW Initialised!");
    } else {
        if (!isRetry) {
            logError("-> ESPNOW Init failed! retrying in 1 sec...");
            delay(1000);
            initESPNow(true);
        } else {
            logFatal("-> ESPNOW Init failed!");
            // TODO: How to handle this?
        }
    }
}

/**
 * Callback for new received ESPNOW data.
 */
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    if (data_len == 6) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        char dataStr[18];
        snprintf(dataStr, sizeof(dataStr), "%02x,%02x,%02x,%02x,%02x,%02x",
                 data[0], data[1], data[2], data[3], data[4], data[5]);

        logDebug("Received %s from %s", dataStr, macStr);

        if (data[0] == CMD_GOT_HIT) {
            receivedGotHit = true;
            gotHitHPMode = data[1];
            gotHitPID = data[2];
            gotHitSID = data[3] | (data[4] << 8);
        }
    }
}

/**
 * Returns if a GOT_HIT was received since the last time called.
 */
bool hpnowGotHit() {
    if (receivedGotHit) {
        receivedGotHit = false;
        return true;
    }
    return false;
}

/**
 * Returns the last received hpmode
 */
uint8_t hpnowGotHitHPMode() {
    return gotHitHPMode;
}

/**
 * Returns the last received PID
 */
uint8_t hpnowGotHitPID() {
    return gotHitPID;
}

/**
 * Returns the last received SID
 */
uint16_t hpnowGotHitSID() {
    return gotHitSID;
}

/**
 * Sending hpnow SYS_INIT command to nearby hitpoints
 */
void hpnowSysInit(uint8_t hpMode, uint8_t color_r, uint8_t color_g, uint8_t color_b) {
    const uint8_t peer_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t data[6] = {
        CMD_SYS_INIT,
        hpMode,
        color_r,
        color_g,
        color_b,
        0};
    esp_err_t result = esp_now_send(peer_addr, data, sizeof(data));
    logDebug("Send CMD SYS INIT with result: %d (%d -> OK)", result, ESP_OK);
}

/**
 * Sending hpnow HIT_VALID command to nearby hitpoints
 */
void hpnowHitValid(uint8_t hpMode, uint8_t pid, uint16_t sid, uint8_t cooldown) {
    const uint8_t peer_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t data[6] = {
        CMD_HIT_VALID,
        hpMode,
        pid,
        sid & 0xff,
        (sid >> 8) & 0xff,
        cooldown};
    esp_err_t result = esp_now_send(peer_addr, data, sizeof(data));
    logDebug("Send CMD GOT HIT with result: %d (%d -> OK)", result, ESP_OK);
}

/**
 * Init hpnow/ESPNOW driver
 */
void hpnowInit() {
    logInfo("Init: ESPNOW Communication");
    WiFi.mode(WIFI_STA);
    logDebug("-> STA MAC: %s", WiFi.macAddress().c_str());
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    logDebug("-> WiFi Channel set to %d", ESPNOW_CHANNEL);
    initESPNow(false);
    logDebug("-> ESP NOW Init done");

    esp_now_register_recv_cb(onReceive);
    logDebug("-> Recv CB set");

    memset(&broadcastReceiver, 0, sizeof(broadcastReceiver));
    for (int i = 0; i < 6; i++) {
        broadcastReceiver.peer_addr[i] = 0xff;
    }
    broadcastReceiver.channel = ESPNOW_CHANNEL;
    broadcastReceiver.encrypt = 0;

    esp_now_add_peer(&broadcastReceiver);
    logDebug("-> Peer added");
}