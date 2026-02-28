/*---------------------------------------------------------*\
| CorsairBragiDevices.h                                     |
|                                                           |
| Device list for Corsair Bragi protocol keyboards          |
|                                                           |
| This file is part of the OpenRGB project                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#pragma once

#include "CorsairPeripheralV2Devices.h"

/*-----------------------------------------------------*\
| Corsair Bragi Protocol Keyboards                      |
\*-----------------------------------------------------*/
#define CORSAIR_BRAGI_K65_PLUS_DONGLE_PID       0x2B07
#define CORSAIR_BRAGI_K65_PLUS_NA_PID           0x2B10
#define CORSAIR_BRAGI_K65_PLUS_UK_PID           0x2B11
#define CORSAIR_BRAGI_K65_PLUS_JP_PID           0x2B12
#define CORSAIR_BRAGI_K65_PLUS_V2_NA_PID        0x2B23
#define CORSAIR_BRAGI_K65_PLUS_V2_UK_PID        0x2B26
#define CORSAIR_BRAGI_K65_PLUS_V2_JP_PID        0x2B37

/*-----------------------------------------------------*\
| These constant values are defined in                  |
| CorsairBragiDevices.cpp                               |
\*-----------------------------------------------------*/
extern const unsigned int       CORSAIR_BRAGI_DEVICE_COUNT;
extern const corsair_v2_device** corsair_bragi_device_list;

extern const unsigned int       CORSAIR_BRAGI_DONGLE_COUNT;
extern const uint16_t*          corsair_bragi_dongle_pids;
