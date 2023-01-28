/*
Skirmish ESP32 Firmware

Hardware Control - Header file

This module controls hardware functions which are
not extensive enough to be controlled in a separate module.

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>

#include <inc/hardware_control.h>
#include <inc/log.h>
#include <conf.h>

// Amount of bits the adc should read and return
const uint8_t adcResolution = 12;
uint16_t lipoCurve[100] = {3270, 3338, 3407, 3476, 3544, 3610, 3626, 3643, 3659, 3675, 3690, 3694, 3698, 3702, 3706, 3710, 3714, 3718, 3722, 3726, 3730, 3734, 3738, 3742, 3746, 3751, 3755, 3759, 3763, 3767, 3771, 3775, 3779, 3783, 3787, 3790, 3792, 3794, 3796, 3798, 3801, 3805, 3809, 3813, 3817, 3821, 3825, 3829, 3833, 3837, 3841, 3843, 3845, 3847, 3849, 3852, 3856, 3860, 3864, 3868, 3874, 3882, 3891, 3899, 3907, 3915, 3923, 3931, 3939, 3947, 3954, 3960, 3966, 3972, 3978, 3986, 3994, 4002, 4010, 4018, 4029, 4041, 4053, 4066, 4078, 4085, 4091, 4097, 4103, 4109, 4117, 4125, 4133, 4141, 4149, 4159, 4169, 4179, 4189, 4200};

// Until which millis() value the vibration motor should
// be turned on
uint32_t vibrateUntil = 0;

// Is set to true by the trigger interrupt to indicate
// that the trigger button was pressed.
bool wasTriggerPressed = false;

/**
 * Interrupt subroutine which is called when the trigger
 * button is pressed and the falling edge interrupt fires.
*/
#ifndef NO_PHASER
void IRAM_ATTR triggerISR() {
    wasTriggerPressed = true;
}
#endif

/**
 * This function returns true if the trigger button was
 * pressed since this function was called the last time.
 * 
 * @return true if a event should be executed
*/
bool hardwareWasTriggerPressed() {
    // Store a copy
    bool retVal = wasTriggerPressed;
    // Reset the trigger
    wasTriggerPressed = false;
    // Return the copy
    return retVal;
}

/**
 * Turns of the system power supply
*/
void hardwarePowerOff() {
    logWarn("System will power off now!");
    digitalWrite(PIN_PWR_OFF, HIGH);
}


/**
 * Reads the battery voltage from the PIN_VBAT_MEASURE.
 * If the result of this function isn't accurate, please calibrate
 * the ADC_VREF value in conf.h
 * 
 * @return The battery voltage in millivolts
*/
uint16_t hardwareReadVBAT() {
    // Read the value on the adc pin
    uint16_t analogReadValue = analogRead(PIN_VBAT_MEASURE);
    // Convert it to a voltage on the pin
    float pinVoltage = (analogReadValue / (float)pow(2, adcResolution)) * (ADC_VREF);
    // Calculate the voltage before the resisitor divider which is vbat
    float batteryVoltage = (VBAT_RHI * pinVoltage + VBAT_RLO * pinVoltage) / VBAT_RLO;

    return batteryVoltage;
}

/**
 * Returns the current battery charging in percent (0-1)
*/
float hardwareBatteryPercent() {
    uint16_t vbat = hardwareReadVBAT();
    for (uint8_t i = 99; i >= 0; i --) {
        if (vbat >= lipoCurve[i]) return (i + 1) / 100.0;
    }
    return 0;
}

/**
 * Turns the vibration motor on and stores the time when it
 * should be shut down. It requires that hardwareLoop() is called
 * periodically to turn of the motor.
 * 
 * @param duration Duration the motor should vibrate (in milliseconds)
*/
void hardwareVibrate(uint16_t duration) {
    #ifndef NO_VIBR_MOTOR
    logDebug("Vibrating for %d milliseconds", duration);
    digitalWrite(PIN_VIBR_MOTOR, HIGH);
    vibrateUntil = millis() + duration;
    #endif
}

/**
 * Hardware Loop function. Call periodically!
*/
void hardwareLoop() {
    // If enabled turn off the vibration motor
    #ifndef NO_VIBR_MOTOR
    if (vibrateUntil > 0 && millis() >= vibrateUntil) {
        digitalWrite(PIN_VIBR_MOTOR, LOW);
        vibrateUntil = 0;
    }
    #endif
}

/**
 * Initializes all parts of the hardware which are controlled
 * via this module. Required pins should be defined in the conf.h file.
 * 
 * Pins required in this module are:
 * - PIN_PWR_OFF: Pin to turn off the power supply (active high)
 * - PIN_TRIGGER: Pin connected to the trigger button (active low)
 * - PIN_VIBR_MOTOR: Pin connected to the driver for the vibration motor.
 * - PIN_VBAT_MEASURE: Pin connected to the battery voltage (trough a voltage divider)
*/
void hardwareInit() {
    logInfo("Init: Hardware driver");

    // Setting pin modes
    pinMode(PIN_PWR_OFF, OUTPUT);
    #ifndef NO_PHASER
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
    #endif
    #ifndef NO_VIBR_MOTOR
    pinMode(PIN_VIBR_MOTOR, OUTPUT);
    #endif

    // Configure ADC
    analogReadResolution(adcResolution);
    analogSetAttenuation(ADC_2_5db);

    // Attaching an interrupt to the trigger pin
    #ifndef NO_PHASER
    attachInterrupt(PIN_TRIGGER, triggerISR, FALLING);
    #endif

}