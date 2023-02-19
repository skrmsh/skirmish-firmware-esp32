/*
Skirmish ESP32 Firmware

Constant values

Copyright (C) 2023 Ole Lange
*/

// Hardware Module identifiers
#define MODULE_PHASER   0b000
#define MODULE_CHEST   0b001
#define MODULE_HITPOINT 0b110

// Logging levels
#define LOGLEVEL_DEBUG 0
#define LOGLEVEL_INFO  1
#define LOGLEVEL_WARN  2
#define LOGLEVEL_ERROR 3
#define LOGLEVEL_FATAL 4
#define LOGLEVEL_OFF   5

// Hitpoint Addresses
#define HP_ADDR_PHASER     0x50
#define HP_ADDR_CHEST      0x51
#define HP_ADDR_BACK       0x52
#define HP_ADDR_SHOULDER_L 0x53
#define HP_ADDR_SHOULDER_R 0x54
#define HP_ADDR_HEAD       0x55
#define HP_ADDR_HITPOINT   0x56
#define HP_ADDR_UNDEFINED  0x57

// Hitpoint Animations
#define HP_ANIM_SOLID   0
#define HP_ANIM_BLINK   1
#define HP_ANIM_ROTATE  2
#define HP_ANIM_BREATHE 3

// Display
#define SPI_MAX_FREQ 80000000

// Skirmish Protocol
#define ACTION_KEEP_ALIVE          0
#define ACTION_TIMESYNC            1
#define ACTION_JOIN_GAME           2
#define ACTION_JOINED_GAME         3
#define ACTION_LEAVE_GAME          4
#define ACTION_GAME_CLOSED         5
#define ACTION_GOT_HIT             6
#define ACTION_SEND_SHOT           7
#define ACTION_HIT_VALID           8
#define ACTION_SHOT_HIT            9
#define ACTION_ADD_AMMO            10
#define ACTION_JOINED_SERVER       11
#define ACTION_FULL_DATA_UPDATE    12
#define ACTION_SERVER_JOIN_DENIED  13
#define ACTION_INVALID_GAME        14
#define ACTION_POWER_OFF           15

// UI Scenes
#define SCENE_NO_SCENE      0
#define SCENE_SPLASHSCREEN  1
#define SCENE_BLE_CONNECT   2
#define SCENE_NO_GAME       3
#define SCENE_JOINED_GAME   4
#define SCENE_COUNTDOWN     5
#define SCENE_GAME          6