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
void IRAM_ATTR triggerISR() {
    wasTriggerPressed = true;
}

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
    return (float)(vbat - VBAT_MIN) / (VBAT_MAX - VBAT_MIN);
}

/**
 * Turns the vibration motor on and stores the time when it
 * should be shut down. It requires that hardwareLoop() is called
 * periodically to turn of the motor.
 * 
 * @param duration Duration the motor should vibrate (in milliseconds)
*/
void hardwareVibrate(uint16_t duration) {
    logDebug("Vibrating for %d milliseconds", duration);
    digitalWrite(PIN_VIBR_MOTOR, HIGH);
    vibrateUntil = millis() + duration;
}

/**
 * Hardware Loop function. Call periodically!
*/
void hardwareLoop() {
    // If enabled turn off the vibration motor
    if (vibrateUntil > 0 && millis() >= vibrateUntil) {
        digitalWrite(PIN_VIBR_MOTOR, LOW);
        vibrateUntil = 0;
    }
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
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
    pinMode(PIN_VIBR_MOTOR, OUTPUT);

    // Configure ADC
    analogReadResolution(adcResolution);
    analogSetAttenuation(ADC_2_5db);

    // Attaching an interrupt to the trigger pin
    attachInterrupt(PIN_TRIGGER, triggerISR, FALLING);

}