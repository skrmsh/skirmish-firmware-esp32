/*
Skirmish ESP32 Firmware

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>

#include <SPIFFS.h>

#include <inc/log.h>
#include <inc/hitpoint.h>
#include <inc/const.h>
#include <inc/hardware_control.h>
#include <inc/display.h>
#include <inc/ui.h>
#include <theme.h>
#include <inc/bluetooth.h>
#include <inc/skirmcom.h>
#include <inc/audio.h>
#include <inc/game.h>
#include <inc/infrared.h>

TaskHandle_t audioTaskHandle;

Game *game;
SkirmCom *com;
SkirmishBluetooth *bluetoothDriver;

SkirmishDisplay display = SkirmishDisplay();
SkirmishUI *userInterface;

bool previousConnectionState = false;


void setup() {
    // Wait some time for the hitpoints to init
    delay(250);

    // Initializing all drivers / utils that require an initialisation
    logInit();
    logInfo("Logging initialized. Welcome!");

    display.init();

    hardwareInit();
    hitpointInit();
    infraredInit();

    logInfo("Init: SPIFFS");
    SPIFFS.begin();

    // Initing audio, when the pwr off button is pressed at boot
    // the audio gain will be set to 0 -> silence
    float audioGain = 0.6;
    if (digitalRead(PIN_PWR_OFF)) audioGain = 0;
    audioInit(audioGain);

    // Creating audio task and pinning it to core 0
    xTaskCreatePinnedToCore(audioLoopTask, "audioLoopTask", 10000, NULL, 0, &audioTaskHandle, 0);
    logDebug("Pinned Audio Task to Core 0");

    game = new Game();
    bluetoothDriver = new SkirmishBluetooth();
    userInterface = new SkirmishUI(&display, bluetoothDriver, game);
    com = new SkirmCom(bluetoothDriver, game, userInterface);
    bluetoothDriver->setCom(com);
    com->init();

    bluetoothDriver->init();
    userInterface->setScene(SCENE_BLE_CONNECT);

    logInfo("Current Battery Voltage is %d", hardwareReadVBAT());

    audioBegin("/bootup.wav");
}

uint16_t currentSid = 1;
uint32_t lastFiredShot = 0;
uint32_t mnow;

uint32_t lastReceivedShot = 0;
uint8_t pid;
uint16_t sid;

bool triggerPressed = false;
bool hitpointEvent = false;

void loop() {
    hardwareLoop();

    mnow = millis();

    triggerPressed = hardwareWasTriggerPressed();
    hitpointEvent = hitpointEventTriggered();

    if (game->isRunning()) {
        if (triggerPressed &&
            game->player.canFire() &&
            mnow - lastFiredShot > game->player.maxShotInterval) {
            
            infraredTransmitShot(game->player.pid, currentSid);
            audioBegin("/blaster.wav");
            hardwareVibrate(150);
            com->shotFired(currentSid);

            if (game->player.ammoLimit) {
                game->player.ammo -= 1;
                game->player.dataWasUpdated();
            }

            lastFiredShot = mnow;
            currentSid ++;
        }

        if (hitpointEvent && !game->player.isInviolable()) {
            lastReceivedShot = hitpointReadShotRaw(HP_ADDR_PHASER);
            logDebug("Received Hitpoint Data: %08x", lastReceivedShot);

            pid = getPIDFromShot(lastReceivedShot);
            sid = getSIDFromShot(lastReceivedShot);

            // If the hitpoint received a shot from it's player do not
            // notify the server.
            if (pid != game->player.pid) {
                com->gotHit(pid, sid);
            }

        }
    }
    
    userInterface->update();
    userInterface->render();
}