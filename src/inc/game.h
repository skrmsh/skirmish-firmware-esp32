/*
Skirmish ESP32 Firmware

Game logic - header file

Copyright (C) 2023 Ole Lange
*/

#pragma once

#include <ArduinoJson.h>

class Player {

    private:

    public:
        Player();

        void reset();

        uint8_t pid;
        char *name;
        float health;
        uint32_t points;
        uint8_t color_r;
        uint8_t color_g;
        uint8_t color_b;
        bool colorBeforeGame;
        bool ammoLimit;
        uint16_t ammo;
        bool phaserEnable;
        uint32_t phaserDisableUntil;
        uint16_t maxShotInterval;
        uint8_t rank;
        bool inviolable;
        uint32_t inviolableUntil;

        bool wasHit = false;
        char* wasHitBy;

        bool hasHit = false;
        char* hasHitName;

        bool wasDataUpdated = false;
        void dataWasUpdated();
        void afterDataUpdate();

        bool canFire();
        bool isInviolable();

};

class Team {

    private:

    public:
        Team();

        void reset();

        uint8_t tid = 0;
        uint8_t playerCount;
        uint32_t points;
        uint8_t rank;
        char* name;

        bool wasDataUpdated = false;
        void dataWasUpdated();
        void afterDataUpdate();

};


class Game {

    private:

    public:
        Game();

        void reset();

        Player player;
        Team team;

        char* gid;
        uint8_t playerCount;
        uint8_t teamCount;
        uint32_t startTime;
        
        bool wasDataUpdated = false;
        void dataWasUpdated();
        void afterDataUpdate();

        void updatePGTData(JsonObject *root);

        bool isRunning();

};