/*
Skirmish ESP32 Firmware

Hitpoint driver

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>
#include <Wire.h>

#include <inc/hitpoint.h>
#include <conf.h>
#include <inc/log.h>
#include <inc/const.h>

//
// ====== Infrared Shot Receiver Functions ======
//

// Is set true by when any hitpoint triggers the hitpointISR
// function to indicate that there is a new event (shot)
bool isHitpointEventTriggered = false;

/**
 * Interrupt Subroutine when the Hitpoints
 * requests the interrupt via IRQ pin. The hitpoint
 * should request an interrupt only when a new shot
 * is received via infrared.
*/
void IRAM_ATTR hitpointISR() {
    isHitpointEventTriggered = true;
}

/**
 * This function returns true if a hitpoint has requested an
 * interrupt since this function was called the last time. If
 * the return value is true, the shot data should be read f^rom
 * the hitpoint and get handled.
 * 
 * @return true if a event should be executed
*/
bool hitpointEventTriggered() {
    // store a local copy
    bool retVal = isHitpointEventTriggered;
    // reset the event trigger
    isHitpointEventTriggered = false;

    return retVal;
}

/**
 * Reads the last received shot data packet from the specified hitpoint
 * and returns it raw as a 32-Bit integer.
 * 
 * @param addr I2C address of the hitpoint
 * @return Raw shot data packet. If 0, the shot isn't valid.
*/
uint32_t hitpointReadShotRaw(uint8_t addr) {
    // Request 4 Bytes from the hitpoint
    uint8_t returned_bytes = Wire.requestFrom(addr, (uint8_t) 4);

    // Check if 4 Bytes are returned
    if (returned_bytes != 4) {
        // If not abort the operation without reading a shot
        return 0;
    }

    // Variable to store the packet
    uint32_t shot = 0x00000000;
    while(Wire.available()) {
        // Shift the current value of shot left by 8 bits and
        // OR it with the next byte of data. This adds the 
        // next byte of data to the end of shot, preserving
        // the existing data.
        shot = (shot << 8) | (uint8_t) Wire.read();
    }

    return shot;
}

/**
 * Reads the last received shot data from every connected hitpoint and
 * returns the first which is not 0x00000000
 * 
 * @param [out] addr address where the shot was received
 * @return Raw shot data packet. If 0, no shot was received.
*/
uint32_t hitpointReadShotRaw(uint8_t* addr) {
    // This function is typically called when one or more hitpoints triggered
    // the hitpoint interrupt, to make sure that the data is ready when read
    // this function will wait some time before starting to read the hitpoints
    delay(50);

    uint32_t retVal = 0;
    uint32_t hpVal = 0;
    
    // Read every hitpoint, but only write the first which isn't
    // zero to the return value. This causes that the first shot
    // is used but every hitpoint it reset
    for (uint8_t a : attachedHitpoints) {
        hpVal = hitpointReadShotRaw(a);
        if (retVal == 0 && hpVal != 0) {
            retVal = hpVal;
            *addr = a;
        }
    }

    return retVal;
}

/**
 * Returns the 8-Bit PlayerID from a raw shot
 * 
 * @param shot raw shot packet
*/
uint8_t getPIDFromShot(uint32_t shot) {
    return (shot >> 16) & 0xff;
}

/**
 * Returns the 16-Bit ShotID from a raw shot
 * 
 * @param shot raw shot packet
*/
uint16_t getSIDFromShot(uint32_t shot) {
    return shot & 0xffff;
}

//
// ====== LED Controlling Functions ======
//

// This table contains values to correct the RGB values for WS2812 LEDs
// (the LEDs on the hitpoints). Written by Adafruit, source:
// https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// Buffer containing the data which should be written to the hitpoint
// is allocated to 5 Bytes in the init function
uint8_t* i2cWriteBuffer;

/**
 * Writes n bytes from the i2cWriteBuffer to the hitpoint with
 * the given address.
 * 
 * @param n Amount of bytes which should be written
 * @param addr Hitpoint Address
*/
void writeBuffer(uint8_t n, uint8_t addr) {
    // Starts the transmission
    Wire.beginTransmission(addr);
    // Transfers the bytes
    Wire.write(i2cWriteBuffer, n);
    // Ends the transmission
    Wire.endTransmission();

    // And delays a short period of time.
    // I don't really remeber why this was implemented
    // in the old prototype code but I transfered it
    // to this code to prevent any bugs. TODO: find out
    // if it is really required!
    delay(50);
}

/**
 * Select an animation on the specified hitpoint. Animations are
 * defined in the const.h file.
 * 
 * @param addr Hitpoint Address
 * @param animation The animation which the given hitpoint should run
*/
void hitpointSelectAnimation(uint8_t addr, uint8_t animation) {
    // Set Command
    i2cWriteBuffer[0] = HP_CMD_SELECT_ANIM;
    // Set animation parameter
    i2cWriteBuffer[1] = animation;
    // Set end value
    i2cWriteBuffer[2] = 0x00;

    // Write the data to the hitpoint
    writeBuffer(3, addr);

    logDebug("Set Hitpoint (0x%02x) animation: 0x%02x", addr, animation);
}

/**
 * Select an animation on every hitpoint. Animations are
 * defined in the const.h file.
 * 
 * @param animation The animation which the given hitpoint should run
*/
void hitpointSelectAnimation(uint8_t animation) {
    for (uint8_t addr : attachedHitpoints) hitpointSelectAnimation(addr, animation);
}

/**
 * Select the animation speed for the specified hitpoint.
 * The speed value depends on the selected animation:
 * 
 * - Solid: Speed does not apply
 * - Blink: (10 * speed) milliseconds per blink
 * - Rotate: Next pixel every (10 * speed) milliseconds
 * - Breathe: 1% fading in the current direction every speed milliseconds
 * 
 * @param addr Address of the hitpoint
 * @param speed Speed to set for the animations
 */
void hitpointSetAnimationSpeed(uint8_t addr, uint8_t speed) {
    // Set Command
    i2cWriteBuffer[0] = HP_CMD_SET_ANIM_SPEED;
    // Set speed parameter
    i2cWriteBuffer[1] = speed;
    // Set end value
    i2cWriteBuffer[2] = 0;

    // Write the data to the hitpoint
    writeBuffer(3, addr);

    logDebug("Set Hitpoint (0x%02x) animation speed to: %d", addr, speed);
}

/**
 * Select the animation speed for any hitpoint.
 * The speed value depends on the selected animation:
 * 
 * - Solid: Speed does not apply
 * - Blink: (10 * speed) milliseconds per blink
 * - Rotate: Next pixel every (10 * speed) milliseconds
 * - Breathe: 1% fading in the current direction every speed milliseconds
 * 
 * @param speed Speed to set for the animations
 */
void hitpointSetAnimationSpeed(uint8_t speed) {
    for (uint8_t addr : attachedHitpoints) hitpointSetAnimationSpeed(addr, speed);
}

/**
 * Set the color for the hitpoint animation.
 * 
 * @param addr Address of the hitpoint
 * @param r Red color value (0-255)
 * @param g Green color value (0-255)
 * @param b Blue color value (0-255)
*/
void hitpointSetColor(uint8_t addr, uint8_t r, uint8_t g, uint8_t b) {
    // Set Command
    i2cWriteBuffer[0] = HP_CMD_SET_COLOR;
    // Set r, g, b parameter
    uint8_t dim_r = r * LED_MAX_BRIGHTNESS;
    uint8_t dim_g = g * LED_MAX_BRIGHTNESS;
    uint8_t dim_b = b * LED_MAX_BRIGHTNESS;
    i2cWriteBuffer[1] = gamma8[dim_r];
    i2cWriteBuffer[2] = gamma8[dim_g];
    i2cWriteBuffer[3] = gamma8[dim_b];
    // Set end value
    i2cWriteBuffer[4] = 0;
    
    // Write the data to the hitpoint
    writeBuffer(5, addr);

    logDebug("Set Hitpoint (0x%02x) color to: #%02x%02x%02x", addr, r, g, b);
}


/**
 * Set the color for the hitpoint animation on every connected hitpoint
 * 
 * @param r Red color value (0-255)
 * @param g Green color value (0-255)
 * @param b Blue color value (0-255)
*/
void hitpointSetColor(uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t addr : attachedHitpoints) hitpointSetColor(addr, r, g, b);
}

/**
 * Initialize the Hitpoint Driver. Also initializes the I2C bus
 * using Wire.h with default frequency on pins PIN_SDA, PIN_SCL
 * which should be defined in the conf.h file. It also attaches
 * an interrupt to the PIN_HP_IRQ pin to give the hitpoint the
 * ability to trigger an interrupt on the phaser.
*/
void hitpointInit() {
    logInfo("Init: Hitpoint driver");

    // Initing I2C Communication
    Wire.begin(PIN_SDA, PIN_SCL);
    logDebug("-> I2C initialized");

    // Attaching interrupts to the configured IRQ pin
    pinMode(PIN_HP_IRQ, INPUT);
    attachInterrupt(PIN_HP_IRQ, hitpointISR, FALLING);
    logDebug("-> Hitpoint ISR attached");

    // Allocating memory for the i2c write buffer
    i2cWriteBuffer = (uint8_t*) malloc(5 * sizeof(uint8_t));
}