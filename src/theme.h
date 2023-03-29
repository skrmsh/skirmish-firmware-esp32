/*
Skirmish ESP32 Firmware

Default theme file

Copyright (C) 2023 Ole Lange
*/

#include <fonts/theNeueBlack10pt.h>
#include <fonts/theNeueBlack18pt.h>

// Themeing (SDT -> Skirmish Display Theme)
#define SDT_BG_COLOR 0x2945

#define SDT_TEXT_COLOR 0xDEFB

#define SDT_HEADER_FONT &TheNeue_Black18pt7b
#define SDT_HEADER_FONT_SIZE 1

#define SDT_SUBHEADER_FONT &TheNeue_Black10pt7b

#define SDT_TEXT_FONT NULL
#define SDT_TEXT_FONT_SIZE 2

#define SDT_PRIMARY_COLOR 0xE8EC
#define SDT_PRIMARY_COLOR_RGB 233, 30, 99

#define SDT_SECONDARY_COLOR 0x5ADD
#define SDT_SECONDARY_COLOR_RGB 90, 90, 233

#define SDT_BATTERY_LOW_COLOR 0xF9E4
#define SDT_BATTERY_MID_COLOR 0xFDE4
#define SDT_BATTERY_HIGH_COLOR 0x6743

#define SDT_HEALTH_COLOR 0x6743
#define SDT_HEALTH_BG_COLOR 0x632C