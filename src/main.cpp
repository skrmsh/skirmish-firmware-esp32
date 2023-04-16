/*
Skirmish ESP32 Firmware

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>
#include <SPIFFS.h>
#include <inc/const.h>
#include <inc/hardware_control.h>
#include <inc/hitpoint.h>
#include <inc/log.h>
#ifndef NO_DISPLAY
#include <inc/display.h>
#endif
#include <inc/bluetooth.h>
#include <inc/skirmcom.h>
#include <inc/ui.h>
#include <theme.h>
#ifndef NO_AUDIO
#include <inc/audio.h>
#endif
#include <inc/game.h>
#ifndef NO_PHASER
#include <inc/infrared.h>
#endif
#include <inc/hpnow.h>

TaskHandle_t audioTaskHandle;

Game *game;
SkirmCom *com;
SkirmishBluetooth *bluetoothDriver;

#ifndef NO_DISPLAY
SkirmishDisplay display = SkirmishDisplay();
#endif
SkirmishUI *userInterface;

bool previousConnectionState = false;

void handleHPNowData(const uint8_t *data) {
    uint8_t cmd = data[0];
    uint8_t mode = data[1];

    if (cmd == 0x03) {
        uint8_t rpid = data[2];
        uint16_t rsid = data[3] | (data[4] << 8);
        logInfo("Hitpoint was hit by %d (%d)", rpid, rsid);
    }
}

void setup() {
    // Wait some time for the hitpoints to init
    delay(250);

    // Initializing all drivers / utils that require an initialisation
    logInit();
    logInfo("Logging initialized. Welcome!");

#ifndef NO_DISPLAY
    display.init();
#endif

    hardwareInit();
    hitpointInit();
#ifndef NO_PHASER
    infraredInit();
#endif

    logInfo("Init: SPIFFS");
    SPIFFS.begin();

// Initing audio, when the pwr off button is pressed at boot
// the audio gain will be set to 0 -> silence
#ifndef NO_AUDIO
    float audioGain = 0.6;
    if (digitalRead(PIN_PWR_OFF))
        audioGain = 0;
    audioInit(audioGain);

    // Creating audio task and pinning it to core 0
    xTaskCreatePinnedToCore(audioLoopTask, "audioLoopTask", 10000, NULL, 0,
                            &audioTaskHandle, 0);
    logDebug("Pinned Audio Task to Core 0");
#endif

    game = new Game();
    bluetoothDriver = new SkirmishBluetooth();

#ifndef NO_DISPLAY
    userInterface = new SkirmishUI(&display, bluetoothDriver, game);
#else
    userInterface = new SkirmishUI(NULL, bluetoothDriver, game);
#endif

    com = new SkirmCom(bluetoothDriver, game, userInterface);
    bluetoothDriver->setCom(com);
    com->init();

    bluetoothDriver->init();
    userInterface->setScene(SCENE_BLE_CONNECT);

    hpnowInit(&handleHPNowData);
    hpnowSysInit(0, 0, 0, 255);

    logInfo("Current Battery Voltage is %d", hardwareReadVBAT());

#ifndef NO_AUDIO
    audioBegin("/bootup.wav");
#endif
}

uint32_t hwStatusLastSend = 0;

uint32_t lastFiredShot = 0;
uint32_t mnow;

uint32_t lastReceivedShot = 0;
uint8_t pid;
uint16_t sid;

uint8_t hitLocation;

bool triggerPressed = false;
bool hitpointEvent = false;

void loop() {
    hardwareLoop();

    mnow = millis();

    if (mnow - hwStatusLastSend > HW_STATUS_SEND_INTERVAL) {
        com->hwStatus(hardwareBatteryPercent());
        hwStatusLastSend = mnow;
    }

    // Turn of the phaser if it was not connected for a while
    if (mnow - bluetoothDriver->lastDisconnectedTime >
            (NOT_CONNECTED_POWER_OFF_TIMEOUT * 60000) &&
        !bluetoothDriver->getConnectionState()) {
        hardwarePowerOff();
    }

    triggerPressed = hardwareWasTriggerPressed();
    hitpointEvent = hitpointEventTriggered();

    if (game->isRunning()) {
#ifndef NO_PHASER
        if (triggerPressed && game->player.canFire() &&
            mnow - lastFiredShot > game->player.maxShotInterval) {
            infraredTransmitShot(game->player.pid, game->player.currentSid);
#ifndef NO_AUDIO
            audioBegin("/blaster.wav");
#endif
#ifndef NO_VIBR_MOTOR
            hardwareVibrate(150);
#endif
            com->shotFired(game->player.currentSid);

            if (game->player.ammoLimit) {
                game->player.ammo -= 1;
                game->player.dataWasUpdated();
            }

            lastFiredShot = mnow;
            game->player.currentSid++;
        }
#endif

        if (hitpointEvent && !game->player.isInviolable()) {
            lastReceivedShot = hitpointReadShotRaw(&hitLocation);
            hitLocation = hitLocation ^ 0x50;
            logDebug("Received Hitpoint Data: %08x @ %d", lastReceivedShot,
                     hitLocation);

            if (lastReceivedShot != 0) { // Shot really contains data
                pid = getPIDFromShot(lastReceivedShot);
                sid = getSIDFromShot(lastReceivedShot);

                // If the hitpoint received a shot from it's player do not
                // notify the server.
                if (pid != game->player.pid) {
                    com->gotHit(pid, sid, hitLocation);
                }
            }
        }
    }

    userInterface->update();
    userInterface->render();
}