/*---------------------------------------------------------*\
| CorsairBragiDevices.cpp                                   |
|                                                           |
| Device list for Corsair Bragi protocol keyboards          |
|                                                           |
| This file is part of the OpenRGB project                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include "CorsairBragiDevices.h"

/*-------------------------------------------------------------*\
|  Corsair K65 Plus Wireless 1B1C:2B10 (via dongle 2B07)        |
|                                                               |
|  Zone "Keyboard"                                              |
|       Matrix                                                  |
|       6 Rows, 16 Columns                                      |
\*-------------------------------------------------------------*/
std::vector<unsigned int> corsair_k65_plus_values =
{
    /* ESC          F1    F2    F3    F4    F5    F6    F7    F8    F9   F10   F11   F12   */
        41,         58,   59,   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,
    /* BKTK    1     2     3     4     5     6     7     8     9     0     -     =   BSPC  */
        53,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   45,   46,   42,
    /* TAB     Q     W     E     R     T     Y     U     I     O     P     [     ]     \   */
        43,   20,   26,    8,   21,   23,   28,   24,   12,   18,   19,   47,   48,   49,
    /* CPLK    A     S     D     F     G     H     J     K     L     ;     "     #   ENTR  */
        57,    4,   22,    7,    9,   10,   11,   13,   14,   15,   51,   52,   50,   40,
    /* LSFT  ISO\    Z     X     C     V     B     N     M     ,     .     /   RSFT        */
       106,  100,   29,   27,    6,   25,    5,   17,   16,   54,   55,   56,  110,
    /* LCTL  LWIN  LALT               SPC              RALT  RFNC  RMNU  RCTL              */
       105,  108,  107,               44,              111,  122,  101,  109,
};

keyboard_keymap_overlay_values corsair_k65_plus_layout
{
    KEYBOARD_SIZE::KEYBOARD_SIZE_SEVENTY_FIVE,
    {
        corsair_k65_plus_values,
        {
            /* Add more regional layout fixes here */
        }
    },
    {
        /*-------------------------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                                             |
        |   Zone,   Row,    Column,     Value,      Name,                       Alternate Name,             OpCode                              |
        \*-------------------------------------------------------------------------------------------------------------------------------------*/
        {   0,      0,      14,         76,         KEY_EN_DELETE,              KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Nav column: Delete
        {   0,      1,      14,         74,         KEY_EN_HOME,               KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Nav column: Home
        {   0,      2,      14,         75,         KEY_EN_PAGE_UP,            KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Nav column: Page Up
        {   0,      3,      14,         78,         KEY_EN_PAGE_DOWN,          KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Nav column: Page Down
        {   0,      5,      12,          0,         KEY_EN_UNUSED,             KEY_EN_UNUSED,              KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Menu key (K65 Plus doesn't have it)
        {   0,      4,      14,         82,         KEY_EN_UP_ARROW,           KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Arrow Up
        {   0,      5,      13,         80,         KEY_EN_LEFT_ARROW,         KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Arrow Left
        {   0,      5,      14,         81,         KEY_EN_DOWN_ARROW,         KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Arrow Down
        {   0,      5,      15,         79,         KEY_EN_RIGHT_ARROW,        KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Arrow Right
    }
};

static const corsair_v2_zone k65_plus_kbd_zone =
{
    ZONE_EN_KEYBOARD,
    ZONE_TYPE_MATRIX,
    6,
    16
};

static const corsair_v2_device k65_plus_na_device =
{
    CORSAIR_BRAGI_K65_PLUS_NA_PID,
    DEVICE_TYPE_KEYBOARD,
    6,
    16,
    {
        &k65_plus_kbd_zone,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    &corsair_k65_plus_layout
};

static const corsair_v2_device k65_plus_uk_device =
{
    CORSAIR_BRAGI_K65_PLUS_UK_PID,
    DEVICE_TYPE_KEYBOARD,
    6,
    16,
    {
        &k65_plus_kbd_zone,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    &corsair_k65_plus_layout
};

static const corsair_v2_device k65_plus_jp_device =
{
    CORSAIR_BRAGI_K65_PLUS_JP_PID,
    DEVICE_TYPE_KEYBOARD,
    6,
    16,
    {
        &k65_plus_kbd_zone,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    &corsair_k65_plus_layout
};

static const corsair_v2_device k65_plus_v2_na_device =
{
    CORSAIR_BRAGI_K65_PLUS_V2_NA_PID,
    DEVICE_TYPE_KEYBOARD,
    6,
    16,
    {
        &k65_plus_kbd_zone,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    &corsair_k65_plus_layout
};

static const corsair_v2_device k65_plus_v2_uk_device =
{
    CORSAIR_BRAGI_K65_PLUS_V2_UK_PID,
    DEVICE_TYPE_KEYBOARD,
    6,
    16,
    {
        &k65_plus_kbd_zone,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    &corsair_k65_plus_layout
};

static const corsair_v2_device k65_plus_v2_jp_device =
{
    CORSAIR_BRAGI_K65_PLUS_V2_JP_PID,
    DEVICE_TYPE_KEYBOARD,
    6,
    16,
    {
        &k65_plus_kbd_zone,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    },
    &corsair_k65_plus_layout
};

/*-------------------------------------------------------------------------*\
|  DEVICE MASTER LIST                                                       |
\*-------------------------------------------------------------------------*/
static const corsair_v2_device* corsair_bragi_device_list_data[] =
{
    &k65_plus_na_device,
    &k65_plus_uk_device,
    &k65_plus_jp_device,
    &k65_plus_v2_na_device,
    &k65_plus_v2_uk_device,
    &k65_plus_v2_jp_device,
};

const unsigned int CORSAIR_BRAGI_DEVICE_COUNT = (sizeof(corsair_bragi_device_list_data) / sizeof(corsair_bragi_device_list_data[0]));
const corsair_v2_device** corsair_bragi_device_list = corsair_bragi_device_list_data;

/*-------------------------------------------------------------------------*\
|  DONGLE PID LIST                                                          |
|  Used by CorsairBragiController to detect dongle vs direct connection     |
\*-------------------------------------------------------------------------*/
static const uint16_t corsair_bragi_dongle_pids_data[] =
{
    CORSAIR_BRAGI_K65_PLUS_DONGLE_PID,
};

const unsigned int CORSAIR_BRAGI_DONGLE_COUNT = (sizeof(corsair_bragi_dongle_pids_data) / sizeof(corsair_bragi_dongle_pids_data[0]));
const uint16_t* corsair_bragi_dongle_pids = corsair_bragi_dongle_pids_data;
