 /*
Skirmish ESP32 Firmware

User Interface

Copyright (C) 2023 Ole Lange
*/

#include <inc/log.h>
#include <inc/ui.h>
#include <inc/hardware_control.h>
#include <inc/hitpoint.h>
#include <theme.h>

#include <inc/scenes/scene.h>
#include <inc/scenes/splash.h>
#include <inc/scenes/joined_game.h>
#include <inc/scenes/countdown.h>
#include <inc/scenes/game.h>

/**
 * User Interface Class Constructor.
 * 
 * @param display pointer to a initialized SkirmishDisplay object
 * @param bluetooth pointer to the bluetooth driver
 * @param game pointer to the game object
*/
SkirmishUI::SkirmishUI(SkirmishDisplay *display,
                       SkirmishBluetooth *bluetooth,
                       Game *game) {
    this->display = display;
    this->game = game;
    this->bluetooth = bluetooth;

    splashscreenScene = new SplashscreenScene(this);
    joinedGameScene = new JoinedGameScene(this);
    countdownScene = new CountdownScene(this);
    gameScene = new GameScene(this);

    currentScene = splashscreenScene;
}

/**
 * Sets that the next time the render() method is called
 * a rendering of the current scene is required. If it's
 * not called the scene might be rendered anyways depending
 * on scheduleing or changed statusOverlay data
*/
void SkirmishUI::setRenderingRequired() {
    renderRequired = true;
}

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

    if (scene == SCENE_SPLASHSCREEN)
        currentScene = splashscreenScene;
    if (scene == SCENE_BLE_CONNECT)
        currentScene = splashscreenScene;
    if (scene == SCENE_NO_GAME)
        currentScene = splashscreenScene;
    if (scene == SCENE_JOINED_GAME)
        currentScene = joinedGameScene;
    if (scene == SCENE_COUNTDOWN)
        currentScene = countdownScene;
    if (scene == SCENE_GAME)
        currentScene = gameScene;

    currentScene->onSet(scene);

    display->clear();
    setRenderingRequired();

    logDebug("Changed to scene %d", scene);
}

/**
 * Updates the user interface
*/
void SkirmishUI::update() {
    // Reacting to bluetooth connection chnages
    bluetoothIsConnected = bluetooth->getConnectionState();

    // When the device just connected
    if (bluetoothIsConnected == true && prevBluetoothIsConnected == false) {
        // Display the "please join a game scene"
        setScene(SCENE_NO_GAME);
    }

    // When the device just disconnected
    if (bluetoothIsConnected == false && prevBluetoothIsConnected == true) {
        // Display the "waiting for connection scene"
        setScene(SCENE_BLE_CONNECT);
    }
    prevBluetoothIsConnected = bluetoothIsConnected;

    if (currentScene->update()) {
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

    renderStatusOverlay();
    currentScene->render();
}


/**
 * Renders the status overlay (top edge)
*/
void SkirmishUI::renderStatusOverlay() {
    // Draw the battery status
    float batteryPercent = hardwareBatteryPercent();
    uint8_t batRectWidth = batteryPercent * 18;
    // Outline + "Battery Contact"
    display->tft.drawRect(210, 3, 20, 10, display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawRect(230, 5, 2, 6, display->gammaCorrection(SDT_TEXT_COLOR));
    // Fill depending on charge level
    uint16_t batteryColor;
    if (batteryPercent > 0.7) batteryColor = SDT_BATTERY_HIGH_COLOR;
    else if (batteryPercent > 0.3) batteryColor = SDT_BATTERY_MID_COLOR;
    else batteryColor = SDT_BATTERY_LOW_COLOR;
    display->tft.fillRect(211, 4, 18, 8, display->gammaCorrection(SDT_BG_COLOR));
    display->tft.fillRect(211, 4, batRectWidth, 8, display->gammaCorrection(batteryColor));

    // Draw the device name
    display->setFont(SDT_TEXT_FONT); 
    display->setTextColor(SDT_TEXT_COLOR);
    display->centerText(bluetooth->getName(), 5, 1);

    // Drawing the bluetooth symbol
    uint8_t btx = 10;
    uint8_t bty = 1;
    display->tft.drawLine(btx, bty, btx, bty+12, display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx-3, bty+3, btx+3, bty+8, display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx+3, bty+3, btx-3, bty+8, display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx, bty, btx+3, bty+3, display->gammaCorrection(SDT_TEXT_COLOR));
    display->tft.drawLine(btx, bty+12, btx+3, bty+9, display->gammaCorrection(SDT_TEXT_COLOR));

    // Draw a cross over it if not connected
    if (!bluetooth->getConnectionState()) {
        display->tft.drawLine(btx-4, bty+13, btx+4, 0, display->gammaCorrection(SDT_PRIMARY_COLOR));
        display->tft.drawLine(btx-4, bty-1, btx+4, 14, display->gammaCorrection(SDT_PRIMARY_COLOR));
    }

    // Draw the Horizontal Rule
    display->tft.drawLine(0, 16, 240, 16, display->gammaCorrection(SDT_TEXT_COLOR));
}