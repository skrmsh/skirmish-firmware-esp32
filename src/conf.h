/*
Skirmish ESP32 Firmware

Default configuration file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <inc/const.h>
#include <stdint.h>

// Configuration values:

// Defining the module type, using the same schema as the hitpoint mcfg
#define MODULE_TYPE MODULE_PHASER

// Logging 
#define LOG_SERIAL_SPEED 115200
// (Levels are 0->5 Debug, Info, Warn, Error, Fatal, Off)
#define LOG_LEVEL        0


// Hardware / Pinout config
// Phaser Module V1.0 (Proto) specific values

// Common
#define PIN_SDA 21
#define PIN_SCL 22

// Hitpoint
#define PIN_HP_IRQ 35 // interrupt request pin

// This is a list of all hitpoints attached to the device
// this firmware is compiled for.
#if MODULE_TYPE == MODULE_PHASER
const uint8_t attachedHitpoints[1] = {HP_ADDR_PHASER};
#elif MODULE_TYPE == MODULE_BREAST
const uint8_t attachedHitpoints[4] = {HP_ADDR_BREAST, HP_ADDR_SHOULDER_L, HP_ADDR_SHOULDER_R, HP_ADDR_BACK};
#endif


// Misc hardware pins
#define PIN_PWR_OFF       4
#define PIN_TRIGGER      19
#define PIN_VIBR_MOTOR    2
#define PIN_VBAT_MEASURE 32
#define PIN_IR_LED 33

#if MODULE_TYPE != MODULE_PHASER
#define NO_PHASER
#define NO_VIBR_MOTOR
#endif

// voltage divider between the battery voltage and the vbat measure pin
// VBAT_RHI is the value of the resistor between VBAT_MEASURE and B+
// VBAT_RLO is the value of the reisitor between VBAT_MEASURE and B- / GND
#define VBAT_RHI 350000
#define VBAT_RLO 100000

// MIN and MAX Voltages
#define VBAT_MIN 3200
#define VBAT_MAX 4200

#define ADC_VREF 1400

// Display
#define DISPLAY_ROTATION 2 // -> Portrait with connector on the top side

#define PIN_TFT_SCK    14
#define PIN_TFT_DATA   13
#define PIN_TFT_CS     15
#define PIN_TFT_DC     5
#define PIN_TFT_RESET  16
#define PIN_TFT_BLCTRL 17

#if MODULE_TYPE != MODULE_PHASER
#define NO_DISPLAY
#endif

#define GAMMA_CORRECTION // Apply gamma correction to the display#

// Audio / Speaker Amp
#define PIN_SPK_EN 23
#define PIN_SPK_LRCLK 25
#define PIN_SPK_BCLK 26
#define PIN_SPK_DIN 27

#if MODULE_TYPE != MODULE_PHASER
#define NO_AUDIO
#endif

// UI
// The UI is refreshed allways when new data is available but to keep
// the battery indicator and other thing fresh the UI is also updated
// every UI_REFRESH_INTERVAL milliseconds
#define UI_REFRESH_INTERVAL 15000