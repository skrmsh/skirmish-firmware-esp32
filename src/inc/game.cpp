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
Team::Team() {}

/**
 * Reset the team data
 */
void Team::reset() {}

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
    if (root->containsKey("g_gid"))
        strcpy(gid, (const char *)root->operator[]("g_gid"));

    P_SET_IF_CONTAINED(playerCount, "g_player_count");
    P_SET_IF_CONTAINED(teamCount, "g_team_count");
    P_SET_IF_CONTAINED(startTime, "g_start_time");

    // Updating Team Data
    T_SET_IF_CONTAINED(team.tid, "t_tid");
    T_SET_IF_CONTAINED(team.playerCount, "t_player_count");
    T_SET_IF_CONTAINED(team.points, "t_points");
    T_SET_IF_CONTAINED(team.rank, "t_rank");

    // Updating Player Data
    if (root->containsKey("p_name")) {
        strcpy(player.name, (const char *)root->operator[]("p_name"));
        player.dataWasUpdated();
    }
    P_SET_IF_CONTAINED(player.pid, "p_pid");
    P_SET_IF_CONTAINED(player.health, "p_health");
    P_SET_IF_CONTAINED(player.points, "p_points");
    P_SET_IF_CONTAINED(player.color_r, "p_color_r");
    P_SET_IF_CONTAINED(player.color_g, "p_color_g");
    P_SET_IF_CONTAINED(player.color_b, "p_color_b");
    P_SET_IF_CONTAINED(player.colorBeforeGame, "p_color_before_game");
    P_SET_IF_CONTAINED(player.ammoLimit, "p_ammo_limit");
    P_SET_IF_CONTAINED(player.ammo, "p_ammo");
    P_SET_IF_CONTAINED(player.phaserEnable, "p_phaser_enable");
    P_SET_IF_CONTAINED(player.phaserDisableUntil, "p_phaser_disable_until");
    P_SET_IF_CONTAINED(player.maxShotInterval, "p_max_shot_interval");
    P_SET_IF_CONTAINED(player.rank, "p_rank");
    P_SET_IF_CONTAINED(player.inviolable, "p_inviolable");
    P_SET_IF_CONTAINED(player.inviolableUntil, "p_inviolable_until");

    afterDataUpdate();
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