/*
Skirmish ESP32 Firmware

Skirmish Communication

Copyright (C) 2023 Ole Lange
*/

#include <ArduinoJson.h>
#include <inc/bluetooth.h>
#include <inc/const.h>
#include <inc/hardware_control.h>
#ifndef NO_HPNOW
#include <inc/hpnow.h>
#endif
#include <inc/log.h>
#include <inc/skirmcom.h>
#include <inc/time.h>
#include <inc/ui.h>

/**
 * Constructor
 */
SkirmCom::SkirmCom(SkirmishBluetooth *bleDriver, Game *game, SkirmishUI *ui) {
    logDebug("Init: SkirmCom");

    this->game = game;
    this->bleDriver = bleDriver;
    this->ui = ui;
}

/**
 *
 */
void SkirmCom::init() {
    this->bleDriver->setOnReceiveCallback(SkirmCom::onReceiveCallback);
    this->bleDriver->setOnConnectCallback(SkirmCom::onConnectCallback);
    this->bleDriver->setOnDisconnectCallback(SkirmCom::onDisconnectCallback);

    jsonOutDocument = new DynamicJsonDocument(512);
    logDebug("Set onReceive callback!");
}

/**
 * Class Member function that is called by the static onReceiveCallback function
 * to handle the received data
 *
 * @param data the received data
 */
void SkirmCom::onReceive(DynamicJsonDocument *data) {
    // Data must contain a field "a" containing a list of actions, else it's
    // invalid and will not be handled. "a" may be an empty list.
    if (!data->containsKey("a")) {
        return;
    }
    JsonObject root = data->as<JsonObject>();

    bool debugPrintJson = true;

    // Updating PGT data
    game->updatePGTData(&root);

    // Iterating over the "a" array and executing all actions
    for (int action : root["a"].as<JsonArray>()) {
        if (action != 0) {  // Prevent spamming keep-alive actions on the log
            logDebug("Action %d:", action);
        } else {
            debugPrintJson = false;
        }

        // Keep-Alive action, doesnt really do anything
        if (action == ACTION_KEEP_ALIVE) {
            continue;
        }

        // Timesync action. Take parameter TS and sets the
        // current unix timestamp of the devices rtc to the
        // value. Not implemented in another function because
        // the logic is too simple and doesn't require that overhead
        if (action == ACTION_TIMESYNC) {
            uint32_t ts = root["TS"];
            logDebug("-> Param TS: %d", ts);
            setCurrentTS(ts);
            continue;
        }

        // This action is called when the clien successfully joined a game
        if (action == ACTION_JOINED_GAME) {
            // Changing the UI scene
            ui->setScene(SCENE_JOINED_GAME);
            continue;
        }

        // This action is called when the game is closed
        if (action == ACTION_GAME_CLOSED) {
            game->reset();
            ui->setScene(SCENE_NO_GAME);
            continue;
        }

        // This action is called when the player was hit
        if (action == ACTION_HIT_VALID) {
            game->player.wasHit = true;

            if (root.containsKey("name")) {
                strcpy(game->player.wasHitBy, root["name"]);
            }
        }

        // This action is called when a shot that was send hitted another player
        if (action == ACTION_SHOT_HIT) {
            game->player.hasHit = true;

            if (root.containsKey("name")) {
                strcpy(game->player.hasHitName, root["name"]);
            }
        }

        if (action == ACTION_POWER_OFF) {
            logInfo("Good Bye!");
            hardwarePowerOff();
        }
#ifndef NO_HPNOW
        if (action == ACTION_HP_INIT) {
            if (root.containsKey("hpmode") && root.containsKey("color_r") &&
                root.containsKey("color_g") && root.containsKey("color_b")) {
                uint8_t hpmode = root["hpmode"];
                uint8_t color_r = root["color_r"];
                uint8_t color_g = root["color_g"];
                uint8_t color_b = root["color_b"];
                hpnowSysInit(hpmode, color_r, color_g, color_b);
            }
        }

        if (action == ACTION_HP_HIT_VALID) {
            if (root.containsKey("hpmode") && root.containsKey("pid") &&
                root.containsKey("sid") && root.containsKey("cooldown")) {
                uint8_t hpmode = root["hpmode"];
                uint8_t pid = root["pid"];
                uint16_t sid = root["sid"];
                uint8_t cooldown = root["cooldown"];
                hpnowHitValid(hpmode, pid, sid, cooldown);
            }
        }
#endif
    }

    if (debugPrintJson) {
        logDebug("Received JSON Data:");
        serializeJson(root, Serial);
        Serial.println();
    }
}

/**
 * Member function that is called by the onConnectCallback function
 */
void SkirmCom::onConnect() {}

/**
 * Member function that is called by the onDisconnectCallback function
 */
void SkirmCom::onDisconnect() {
    // Resetting game on disconnect
    // game->reset();
}

/**
 * onReceive callback for the bluetooth driver. Is called everytime the
 * modem (smartphone app) writes data to the specific characteristic and
 * notifies about the new data.
 *
 * @param data pointer to the received data
 */
void SkirmCom::onReceiveCallback(void *context, DynamicJsonDocument *data) {
    reinterpret_cast<SkirmCom *>(context)->onReceive(data);
}

/**
 * onConnect callback for the bluetooth driver. Is called when the
 * modem connects to the device.
 */
void SkirmCom::onConnectCallback(void *context) {
    reinterpret_cast<SkirmCom *>(context)->onConnect();
}

/**
 * onDisconnect callback for the bluetooth driver. Is called when the
 * modem is disconnected from the device.
 */
void SkirmCom::onDisconnectCallback(void *context) {
    reinterpret_cast<SkirmCom *>(context)->onDisconnect();
}

/**
 * This method tells the server that a shot was fired
 *
 * @param sid Shot ID
 */
void SkirmCom::shotFired(uint16_t sid) {
    // Clearing current data
    jsonOutDocument->clear();

    // Generating Json Data
    /*
    { "a": [ACTION_SEND_SHOT], "sid": sid }
    */
    JsonArray actions = jsonOutDocument->createNestedArray("a");
    actions.add(ACTION_SEND_SHOT);
    jsonOutDocument->operator[]("sid") = sid;

    // Sending data
    bleDriver->writeJsonData(jsonOutDocument);
}

/**
 * This method tells the server that a shot was received
 *
 * @param pid The received Player ID
 * @param sid The received Shot ID
 */
void SkirmCom::gotHit(uint8_t pid, uint16_t sid, uint8_t hitLocation) {
    // Clear current data
    jsonOutDocument->clear();

    // Generating Json Data
    /*
    { "a": [ACTION_GOT_HIT], "pid": pid, "sid": sid, "hp": hitLocation}
    */
    JsonArray actions = jsonOutDocument->createNestedArray("a");
    actions.add(ACTION_GOT_HIT);
    jsonOutDocument->operator[]("pid") = pid;
    jsonOutDocument->operator[]("sid") = sid;
    jsonOutDocument->operator[]("hp") = hitLocation;

    // Sending data
    bleDriver->writeJsonData(jsonOutDocument);
}

/**
 * This method tells the app/server the hardware status of this device
 *
 * @param battery The battery percentage
 */
void SkirmCom::hwStatus(float battery) {
    // Clear current data
    jsonOutDocument->clear();

    // Generating Json Data
    /*
    { "a": [ACTION_HW_STATUS], "battery": battery }
    */
    JsonArray actions = jsonOutDocument->createNestedArray("a");
    actions.add(ACTION_HW_STATUS);
    jsonOutDocument->operator[]("battery") = battery;
    jsonOutDocument->operator[]("d_id") = this->bleDriver->getName();

    // Sending data
    bleDriver->writeJsonData(jsonOutDocument);
}

/**
 * This method triggers the HP_GOT_HIT action oon the server.
 *
 * @param hpmode - received hpmode value
 * @param pid - received pid value
 * @param sid - received sid value
 */
void SkirmCom::hpGotHit(uint8_t hpmode, uint8_t pid, uint16_t sid) {
#ifndef NO_HPNOW
    // Clear current data
    jsonOutDocument->clear();

    // Generating Json Data
    /*
    { "a": [ACTION_HP_GOT_HIT], "hpmode": hpmode, "pid": pid, "sid": sid }
    */
    JsonArray actions = jsonOutDocument->createNestedArray("a");
    actions.add(ACITON_HP_GOT_HIT);
    jsonOutDocument->operator[]("hpmode") = hpmode;
    jsonOutDocument->operator[]("pid") = pid;
    jsonOutDocument->operator[]("sid") = sid;

    // Sending data
    bleDriver->writeJsonData(jsonOutDocument);
#endif
}