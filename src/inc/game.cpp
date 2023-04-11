/*
Skirmish ESP32 Firmware

Game logic

Copyright (C) 2023 Ole Lange
*/

#include <inc/game.h>
#include <inc/log.h>
#include <inc/time.h>
#include <string.h>

// Macro to set a variable to a specific value from a json object called "root"
// if the key is not contained nothing will happen. Does not work for strings
// SET_IF_CONTAINED -> SET IF CONTAINED
#define SET_IF_CONTAINED(var, key, updateCallback) \
    if (root->containsKey((key))) {                \
        var = (root->operator[]((key)));           \
        updateCallback();                          \
    }

#define P_SET_IF_CONTAINED(var, key) \
    SET_IF_CONTAINED(var, key, player.dataWasUpdated)
#define G_SET_IF_CONTAINED(var, key) SET_IF_CONTAINED(var, key, dataWasUpdated)
#define T_SET_IF_CONTAINED(var, key) \
    SET_IF_CONTAINED(var, key, team.dataWasUpdated)

/**
 * Le constructeur
 */
Team::Team() { name = (char *)malloc(33 * sizeof(char)); }

/**
 * Reset the team data
 */
void Team::reset() { strcpy(name, ""); }

/**
 * Function that is called when a field is changed while data update
 */
void Team::dataWasUpdated() { wasDataUpdated = true; }

/**
 * Function that is called after the data update is finished
 */
void Team::afterDataUpdate() {
    if (wasDataUpdated) {
        // Execute code that handles new data hereeeee
    }
    wasDataUpdated = false;
}

/**
 * Der Baumeister
 */
Player::Player() {
    // Allocating 33 bytes of memory for the player name
    name = (char *)malloc(33 * sizeof(char));
    wasHitBy = (char *)malloc(33 * sizeof(char));
    hasHitName = (char *)malloc(33 * sizeof(char));
    this->reset();
}

/**
 * Reset player data
 */
void Player::reset() {
    strcpy(name, "");
    strcpy(wasHitBy, "");
    strcpy(hasHitName, "");

    pid = 0;
    health = 0;
    points = 0;
    color_r = 0;
    color_g = 0;
    color_b = 0;
    colorBeforeGame = false;
    ammoLimit = false;
    ammo = 0;
    phaserEnable = false;
    phaserDisableUntil = 0;
    maxShotInterval = 0;
    rank = 0;
    inviolable = true;
    inviolableUntil = 0;
    wasHit = false;
    hasHit = false;

    currentSid = 1;
}

/**
 * Function that is called when a field is changed while data update
 */
void Player::dataWasUpdated() { wasDataUpdated = true; }

/**
 * Function that is called after the data update is finished
 */
void Player::afterDataUpdate() {
    if (wasDataUpdated) {
        // Execute code that handles new data hereeeee
    }
    wasDataUpdated = false;
}

/**
 * This method returns if the player can currently fire
 */
bool Player::canFire() {
    uint32_t now = getCurrentTS();
    if (phaserDisableUntil > now) return false;

    if (ammoLimit) {
        if (ammo == 0) return false;
    }

    return phaserEnable;
}

/**
 * This method returns if the player is currently inviolable
 */
bool Player::isInviolable() {
    uint32_t now = getCurrentTS();
    if (inviolableUntil > now) return true;
    return inviolable;
}

/**
 * Constructur
 */
Game::Game() {
    // Allocating 33 bytes of memory for gameID strings
    gid = (char *)malloc(33 * sizeof(char));
    team = Team();
    player = Player();
}

/**
 * Reset the game instance to default data
 */
void Game::reset() {
    strcpy(gid, "");
    team.reset();
    player.reset();
}

/**
 * Function that is called when a field is changed while data update
 */
void Game::dataWasUpdated() { wasDataUpdated = true; }

/**
 * Function that is called after the data update is finished
 */
void Game::afterDataUpdate() {
    if (wasDataUpdated) {
        // Execute code that handles new data hereeeee
    }
    wasDataUpdated = false;
}

/**
 * Updates game, player and team object with the values contained
 * by the json obect
 *
 * @param root pointer to a json object containing pgt data
 */
void Game::updatePGTData(JsonObject *root) {
    // Updating Game Data
    if (root->containsKey("g_id"))
        strcpy(gid, (const char *)root->operator[]("g_id"));

    G_SET_IF_CONTAINED(playerCount, "g_pc");
    G_SET_IF_CONTAINED(teamCount, "g_tc");
    G_SET_IF_CONTAINED(startTime, "g_st");

    // Updating Team Data
    if (root->containsKey("t_n")) {
        strcpy(team.name, (const char *)root->operator[]("t_n"));
        team.dataWasUpdated();
    }
    T_SET_IF_CONTAINED(team.tid, "t_id");
    T_SET_IF_CONTAINED(team.playerCount, "t_pc");
    T_SET_IF_CONTAINED(team.points, "t_p");
    T_SET_IF_CONTAINED(team.rank, "t_r");

    // Updating Player Data
    if (root->containsKey("p_n")) {
        strcpy(player.name, (const char *)root->operator[]("p_n"));
        player.dataWasUpdated();
    }
    P_SET_IF_CONTAINED(player.pid, "p_id");
    P_SET_IF_CONTAINED(player.health, "p_h");
    P_SET_IF_CONTAINED(player.points, "p_p");
    P_SET_IF_CONTAINED(player.color_r, "p_cr");
    P_SET_IF_CONTAINED(player.color_g, "p_cg");
    P_SET_IF_CONTAINED(player.color_b, "p_cb");
    P_SET_IF_CONTAINED(player.colorBeforeGame, "p_cbg");
    P_SET_IF_CONTAINED(player.ammoLimit, "p_al");
    P_SET_IF_CONTAINED(player.ammo, "p_a");
    P_SET_IF_CONTAINED(player.phaserEnable, "p_pe");
    P_SET_IF_CONTAINED(player.phaserDisableUntil, "p_pdu");
    P_SET_IF_CONTAINED(player.maxShotInterval, "p_msi");
    P_SET_IF_CONTAINED(player.rank, "p_r");
    P_SET_IF_CONTAINED(player.inviolable, "p_i");
    P_SET_IF_CONTAINED(player.inviolableUntil, "p_iu");
    P_SET_IF_CONTAINED(player.inviolableLightsOff, "p_ilo");

    afterDataUpdate();
    player.afterDataUpdate();
    team.afterDataUpdate();
}

/**
 * Returns if the game is currently running
 */
bool Game::isRunning() {
    uint32_t now = getCurrentTS();
    if (startTime == 0) return false;
    if (startTime < now) return true;
    return false;  // startTime >= now
}