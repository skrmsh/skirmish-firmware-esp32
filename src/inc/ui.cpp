/*
Skirmish ESP32 Firmware

User Interface

Copyright (C) 2023 Ole Lange
*/

#include "ui.h"

#include <stdlib.h>  // included for mocking, maybe required to remove
#include <string.h>  // included for mocking, maybe required to remove

#include "../fonts/skvec.h"
#include "../theme.h"
#include "hardware_control.h"
#include "hitpoint.h"
#include "log.h"
// #include "mocks.h"  // MOCK: REMOVE
#include "scenes/countdown.h"
#include "scenes/game.h"
#include "scenes/joined_game.h"
#include "scenes/scene.h"
#include "scenes/splash.h"
#include "time.h"

/**
 * User Interface Class Constructor.
 *
 * @param display pointer to a initialized SkirmishDisplay object
 * @param bluetooth pointer to the bluetooth driver
 * @param game pointer to the game object
 */
SkirmishUI::SkirmishUI(SkirmishDisplay *display, SkirmishBluetooth *bluetooth,
                       Game *game) {
    this->display = display;
    this->game = game;
    this->bluetooth = bluetooth;

    splashscreenScene = new SplashscreenScene(this);
    joinedGameScene = new JoinedGameScene(this);
    countdownScene = new CountdownScene(this);
    gameScene = new GameScene(this);

    currentScene = splashscreenScene;

    prevBluetoothIsConnected = this->bluetooth->getConnectionState();
}

/**
 * Sets that the next time the render() method is called
 * a rendering of the current scene is required. If it's
 * not called the scene might be rendered anyways depending
 * on scheduleing or changed statusOverlay data
 */
void SkirmishUI::setRenderingRequired() { renderRequired = true; }

/**
 * Selects the current scene. Will be rendered and updated
 * using the update() and render() methods.
 *
 * @param scene the new scene
 */
void SkirmishUI::setScene(uint8_t scene) {
    if (currentScene->getID() == scene) {
        return;
    }

    if (scene == SCENE_SPLASHSCREEN) currentScene = splashscreenScene;
    if (scene == SCENE_BLE_CONNECT) currentScene = splashscreenScene;
    if (scene == SCENE_BLE_RECONNECT) currentScene = splashscreenScene;
    if (scene == SCENE_NO_GAME) currentScene = splashscreenScene;
    if (scene == SCENE_JOINED_GAME) currentScene = joinedGameScene;
    if (scene == SCENE_COUNTDOWN) currentScene = countdownScene;
    if (scene == SCENE_GAME) currentScene = gameScene;

    currentScene->onSet(scene);

    setRenderingRequired();
    clearRequired = true;
    logDebug("Changed to scene %d", scene);
}

/**
 * Starts showing a msgbox
 *
 * @param heading Heading text
 * @param text msg text
 * @param timeout timeout in milliseconds how long the msgbox should be vis
 */
void SkirmishUI::msgBox(char *heading, char *text, uint32_t timeout) {
    msgBoxHeading = heading;
    msgBoxText = text;
    msgBoxVisibleUntil = millis() + timeout;
    msgBoxVisible = true;
    setRenderingRequired();
}

/**
 * Updates the user interface
 */
void SkirmishUI::update() {
    // Reacting to bluetooth connection chnages
    bluetoothIsConnected = bluetooth->getConnectionState();

    // When the device just connected
    if (bluetoothIsConnected == true && prevBluetoothIsConnected == false) {
        // Display the "please join a game scene" / change back to the game
        // scene. TODO: Doesn't work if re-connected
        uint32_t currentTS = getCurrentTS();
        if (game->gid[0] == 0) {  // No game
            setScene(SCENE_NO_GAME);
        } else if (strcmp(game->gid, "") != 0 &&  // joined
                   game->startTime == 0) {        // but not started game
            setScene(SCENE_JOINED_GAME);
        } else if (game->startTime > 0 &&           // started game
                   game->startTime >= currentTS) {  // but counting down
            setScene(SCENE_COUNTDOWN);
        } else if (game->startTime > 0 &&          // started game
                   game->startTime < currentTS) {  // but running
            setScene(SCENE_GAME);
        } else { /* Should never happen */
            setScene(SCENE_NO_SCENE);
        }
    }

    // When the device just disconnected
    if (bluetoothIsConnected == false && prevBluetoothIsConnected == true) {
        // Display the "waiting for connection scene"
        logDebug("Changed to reconnect scene");
        setScene(SCENE_BLE_RECONNECT);
    }
    prevBluetoothIsConnected = bluetoothIsConnected;

    if (currentScene->update()) {
        setRenderingRequired();
    }

    if (millis() > msgBoxVisibleUntil && msgBoxVisible) {
        msgBoxVisible = false;
        clearRequired = true;
        setRenderingRequired();
    }

    // Re-Render the UI every UI_REFRESH_INTERVAL milliseconds to keep
    // things like the status overlay up to date
    if (millis() > nextScheduledUpdate) {
        setRenderingRequired();
        nextScheduledUpdate = millis() + UI_REFRESH_INTERVAL;
    }
}

/**
 * Renders the user interface
 */
void SkirmishUI::render() {
    if (!renderRequired) return;

    renderRequired = false;

    if (clearRequired) {
        clearRequired = false;
#ifndef NO_DISPLAY
        display->clear();
#endif
    }

    currentScene->render();

#ifndef NO_DISPLAY
    // Draw border (type game on game scene, secondary color on ble_reconnect
    // scene)
    if (currentScene->getID() == SCENE_GAME) {
        border(BORDER_TYPE_GAME,
               display->color(game->player.color_r, game->player.color_g,
                              game->player.color_b));
    } else {
        border(BORDER_TYPE_DEFAULT,
               (currentScene->getID() == SCENE_BLE_RECONNECT)
                   ? SDT_SECONDARY_COLOR
                   : SDT_PRIMARY_COLOR);
    }
    renderStatusOverlay();
#endif

#ifndef NO_DISPLAY
    if (msgBoxVisible) {
        renderMsgBox();
    }
#endif
}

#ifndef NO_DISPLAY
/**
 * Renders the status overlay (top edge)
 */
void SkirmishUI::renderStatusOverlay() {
    // Draw the battery status
    float batteryPercent = hardwareBatteryPercent();
    uint8_t batRectWidth = batteryPercent * 18;

    // Unfilled battery symbol
    display->drawVec(skvec_battery, 210, 3, SDT_BG_COLOR);

    // Fill depending on charge level
    uint16_t batteryColor;
    if (batteryPercent > 0.7)
        batteryColor = SDT_BATTERY_HIGH_COLOR;
    else if (batteryPercent > 0.3)
        batteryColor = SDT_BATTERY_MID_COLOR;
    else
        batteryColor = SDT_BATTERY_LOW_COLOR;
    display->tft.fillRect(211, 4, 18, 8,
                          display->gammaCorrection(SDT_BG_COLOR));
    display->tft.fillRect(211, 4, batRectWidth, 8,
                          display->gammaCorrection(batteryColor));

    // Draw the device name
    display->setFont(SDT_TEXT_FONT);
    display->setTextColor(SDT_BG_COLOR);
    display->centerText(bluetooth->getName(), 5, 1);

    // Drawing the bluetooth symbol (if connected)
    if (bluetooth->getConnectionState()) {
        display->drawVec(skvec_bluetooth, 7, 1, SDT_BG_COLOR);
    }
}

void SkirmishUI::renderMsgBox() {
    display->tft.fillRect(30, 100, 180, 100,
                          display->gammaCorrection(SDT_BG_COLOR));
    display->tft.drawRect(30, 100, 180, 100,
                          display->gammaCorrection(SDT_SECONDARY_COLOR));

    // Drawing player name
    if (strlen(msgBoxHeading) > 6) {
        display->setFont(SDT_SUBHEADER_FONT);
    } else {
        display->setFont(SDT_HEADER_FONT);
    }
    display->setTextColor(SDT_PRIMARY_COLOR);
    display->centerText(msgBoxHeading, 140, 1);

    display->setFont(SDT_SUBHEADER_FONT);
    display->setTextColor(SDT_TEXT_COLOR);
    display->centerText(msgBoxText, 170, 1);
}

void SkirmishUI::border(uint8_t type, uint16_t color) {
    display->tft.fillRect(0, 0, 240, 16, display->gammaCorrection(color));
    display->drawVec(skvec_genericBorder, 0, 16, color);
    if (type == BORDER_TYPE_DEFAULT) {
        display->drawVec(skvec_borderTypeDefault, 0, 16, color);

    } else if (BORDER_TYPE_GAME) {
        display->drawVec(skvec_borderTypeGame, 0, 16, color);
        // Drawing dotted bottom line
        int16_t x = -5;
        for (uint8_t i = 0; i < 11; i++) {
            display->tft.fillRect(x, 258, 20, 3,
                                  display->gammaCorrection(color));
            x += 23;
        }
    }
}
#endif
