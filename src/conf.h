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
#define LOG_LEVEL 0

// LED
// Settings maxium brightness for vests to 0.5
// -> vest hw prototype psu doesn't like all leds very bright
#if MODULE_TYPE == MODULE_CHEST
#define LED_MAX_BRIGHTNESS 0.7
#elif MODULE_TYPE == MODULE_PHASER
#define LED_MAX_BRIGHTNESS 1.0
#endif

// Hardware / Pinout config
// Phaser Module V1.0 (Proto) specific values

// Common
#define PIN_SDA 21
#define PIN_SCL 22

// Hitpoint
#define PIN_HP_IRQ 35  // interrupt request pin

// This is a list of all hitpoints attached to the device
// this firmware is compiled for.
#if MODULE_TYPE == MODULE_PHASER
const uint8_t attachedHitpoints[1] = {HP_ADDR_PHASER};
#elif MODULE_TYPE == MODULE_CHEST
const uint8_t attachedHitpoints[4] = {HP_ADDR_CHEST, HP_ADDR_SHOULDER_L,
                                      HP_ADDR_SHOULDER_R, HP_ADDR_BACK};
#endif

// Misc hardware pins
#define PIN_PWR_OFF 4
#define PIN_TRIGGER 19
#define PIN_VIBR_MOTOR 2
#define PIN_VBAT_MEASURE 32
#define PIN_IR_LED 33

#if MODULE_TYPE != MODULE_PHASER
#define NO_PHASER
#define NO_VIBR_MOTOR
#endif

// Battery voltage is measured with a voltage divider between vbat and gnd
// the most accurate way of calculating the voltage of the battery is simply
// by measuring adc values @3.0 and 4.2 volts and interpolating the other values
#define VBAT_SAMPLE_COUNT 100
#define ADC_AT_4200mV 2690.0
#define ADC_AT_3000mV 1910.0

// Display
#define DISPLAY_ROTATION 2  // -> Portrait with connector on the top side

#define PIN_TFT_SCK 14
#define PIN_TFT_DATA 13
#define PIN_TFT_CS 15
#define PIN_TFT_DC 5
#define PIN_TFT_RESET 16
#define PIN_TFT_BLCTRL 17

#if MODULE_TYPE != MODULE_PHASER
#define NO_DISPLAY
#endif

#define GAMMA_CORRECTION  // Apply gamma correction to the display#

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

// Communication
#define HW_STATUS_SEND_INTERVAL 15000