/*
Skirmish ESP32 Firmware

Audio driver

Copyright (C) 2023 Ole Lange
*/

#include <SPIFFS.h>
#include <conf.h>
#include <inc/audio.h>
#include <inc/log.h>

#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

AudioGeneratorWAV *wav;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;

bool isPlaying = false;

/**
 * Initializes the audio driver
 *
 * @param gain audio gain [0-1]
 */
void audioInit(float gain) {
    logInfo("Init: Audio");

    pinMode(PIN_SPK_EN, OUTPUT);
    digitalWrite(PIN_SPK_EN, 1);
    logInfo("-> Enabled Speaker Amplifier");

    wav = new AudioGeneratorWAV();
    out = new AudioOutputI2S();
    out->SetPinout(PIN_SPK_BCLK, PIN_SPK_LRCLK, PIN_SPK_DIN);
    out->SetGain(gain);

    logDebug("-> Audio Init done");
}

/**
 * Begins playing the specified audio file
 *
 * @param filename spiffs path of the audio file
 */
void audioBegin(const char *filename) {
    if (isPlaying) {
        wav->stop();
        isPlaying = false;
    }
    file = new AudioFileSourceSPIFFS(filename);
    wav->begin(file, out);
    isPlaying = true;
}

/**
 * Keeps the current audio file playing.
 */
void audioLoopTask(void *param) {
    while (1) {
        if (!isPlaying) continue;

        if (!wav->isRunning()) {
            isPlaying = false;
            continue;
        }

        if (!wav->loop()) wav->stop();
    }
}