/*
Skirmish ESP32 Firmware

Logging utility

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>

#include <inc/const.h>
#include <conf.h>

// ANSI Escape Sequence colors
const char* ANSI_BLACK   = "\u001b[30m";
const char* ANSI_RED     = "\u001b[31m";
const char* ANSI_GREEN   = "\u001b[32m";
const char* ANSI_YELLOW  = "\u001b[33m";
const char* ANSI_BLUE    = "\u001b[34m";
const char* ANSI_MAGENTA = "\u001b[35m";
const char* ANSI_CYAN    = "\u001b[36m";
const char* ANSI_WHITE   = "\u001b[37m";
const char* ANSI_RESET   = "\u001b[0m";

// Buffer for the current log line
char* currentLogLine;

/**
 * Initialize the logging utility
 */
void logInit() {
    Serial.begin(LOG_SERIAL_SPEED);

    // Allocate memory for the log line buffer
    currentLogLine = (char*) malloc(256 * sizeof(char));
}

// Macro for the generic logging function.
#define LOG(color, levelString) { \
    va_list args; va_start(args, val); \
    vsprintf(currentLogLine, val, args); \
    Serial.printf("[%s%s%s]%s\t%10lu%s - %s\r\n", \
        color, \
        levelString, \
        ANSI_RESET, \
        ANSI_CYAN, \
        millis(), \
        ANSI_RESET, \
        currentLogLine); \
}

/**
 * Log a debug message. Use like printf
 * 
 * @param val Format string
*/
void logDebug(const char* val, ...) {
    #if LOGLEVEL <= LOGLEVEL_DEBUG
        LOG(ANSI_GREEN, "DEBUG");
    #endif
}

/**
 * Log a info message. Use like printf
 * 
 * @param val Format string
*/
void logInfo(const char* val, ...) {
    #if LOGLEVEL <= LOGLEVEL_INFO
        LOG(ANSI_BLUE, "INFO");
    #endif
}

/**
 * Log a warning message. Use like printf
 * 
 * @param val Format string
*/
void logWarn(const char* val, ...) {
    #if LOGLEVEL <= LOGLEVEL_INFO
        LOG(ANSI_YELLOW, "WARNING");
    #endif
}

/**
 * Log a error message. Use like printf
 * 
 * @param val Format string
*/
void logError(const char* val, ...) {
    #if LOGLEVEL <= LOGLEVEL_INFO
        LOG(ANSI_RED, "ERROR");
    #endif
}

/**
 * Log a fatal error message. Use like printf
 * 
 * @param val Format string
*/
void logFatal(const char* val, ...) {
    #if LOGLEVEL <= LOGLEVEL_INFO
        LOG(ANSI_MAGENTA, "FATAL");
    #endif
}