/*
Skirmish ESP32 Firmware

User Interface

Copyright (C) 2023 Ole Lange
*/

#include <inc/hardware_control.h>
#include <inc/hitpoint.h>
#include <inc/log.h>
#include <inc/scenes/countdown.h>
#include <inc/scenes/game.h>
#include <inc/scenes/joined_game.h>
#include <inc/scenes/scene.h>
#include <inc/scenes/splash.h>
#include <inc/time.h>
#include <inc/ui.h>
#include <theme.h>

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

#ifndef NO_DISPLAY
    display->clear();
#endif

    setRenderingRequired();

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

#ifndef NO_DISPLAY
    renderStatusOverlay();
#endif
    currentScene->render();
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
    // Outline + "Battery Contact"
    display->tft.drawRect(210, 3, 20, 10,
                          display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawRect(230, 5, 2, 6,
                          display->gammaCorrection(SDT_TEXT_COLOR));
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
    display->setTextColor(SDT_TEXT_COLOR);
    display->centerText(bluetooth->getName(), 5, 1);

    // Drawing the bluetooth symbol
    uint8_t btx = 10;
    uint8_t bty = 1;
    display->tft.drawLine(btx, bty, btx, bty + 12,
                          display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx - 3, bty + 3, btx + 3, bty + 8,
                          display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx + 3, bty + 3, btx - 3, bty + 8,
                          display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx, bty, btx + 3, bty + 3,
                          display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx, bty + 12, btx + 3, bty + 9,
                          display->gammaCorrection(SDT_TEXT_COLOR));

    // Draw a cross over it if not connected
    if (!bluetooth->getConnectionState()) {
        display->tft.drawLine(btx - 4, bty + 13, btx + 4, 0,
                              display->gammaCorrection(SDT_PRIMARY_COLOR));
        display->tft.drawLine(btx - 4, bty - 1, btx + 4, 14,
                              display->gammaCorrection(SDT_PRIMARY_COLOR));
    }

    // Draw the Horizontal Rule
    display->tft.drawLine(0, 16, 240, 16,
                          display->gammaCorrection(SDT_TEXT_COLOR));
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
#endif