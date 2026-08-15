/*---------------------------------------------------------*\
| CorsairPeripheralV2K65PlusController.cpp                  |
|                                                           |
|   Driver for the Corsair K65 Plus Wireless keyboard,      |
|   wired or behind the Slipstream dongle                   |
|                                                           |
|   ToastKiste21                                28 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "LogManager.h"
#include "CorsairPeripheralV2K65PlusController.h"

namespace
{
/*---------------------------------------------------------*\
| HID usage IDs in the order the wireless lighting frame     |
|   expects them.  83 entries.                               |
\*---------------------------------------------------------*/
static const uint8_t k65_plus_led_indices[] =
{
    82, 27, 11, 63, 64, 78, 16,  7, 67, 31,
    82, 21, 29, 20, 22, 69, 53, 44, 75, 48,
    10,  9, 74,  8, 62, 13, 35,  4, 57,105,
     6, 80, 14, 79, 15,122,108, 56, 52,  5,
   107, 60, 24, 23,110, 41, 26, 59, 51, 42,
    66, 39,106, 76, 30,111, 46, 17, 19, 68,
    18, 43, 33, 58, 37, 25, 45,109, 50, 28,
    47, 54, 61, 32,100, 38, 36, 55, 40, 12,
    65, 81, 34,
};

constexpr uint16_t k65_plus_num_indices = sizeof(k65_plus_led_indices);
}

/*---------------------------------------------------------*\
| The K65 Plus rejects START_TX until it is already in       |
|   software mode, so the base class endpoint probe is       |
|   skipped and CORSAIR_V2_LIGHT_CTRL2 selected directly.    |
\*---------------------------------------------------------*/
CorsairPeripheralV2K65PlusController::CorsairPeripheralV2K65PlusController(hid_device* dev_handle, const char* path, std::string name)
    : CorsairPeripheralV2Controller(dev_handle, path, name, false)
{
    SetRenderMode(CORSAIR_V2_MODE_SW);

    /*---------------------------------------------------------*\
    | Wired, the keyboard holds one lighting handle open for the |
    |   life of the session and every frame is written into it.  |
    |   Behind the dongle the same handle opens without error    |
    |   but the LEDs do not respond, so the wireless path is     |
    |   used instead.                                            |
    \*---------------------------------------------------------*/
    if(write_cmd == CORSAIR_V2_WRITE_WIRED_ID)
    {
        light_ctrl  = CORSAIR_V2_LIGHT_CTRL2;

        if(StartTransaction(0, light_ctrl) == 0)
        {
            handle_open = true;
        }
        else
        {
            LOG_ERROR("[%s] Could not open lighting endpoint %02X", device_name.c_str(), light_ctrl);
        }
    }
}

CorsairPeripheralV2K65PlusController::~CorsairPeripheralV2K65PlusController()
{
    if(handle_open)
    {
        StopTransaction(0);
    }

    SetRenderMode(CORSAIR_V2_MODE_HW);
}

/*---------------------------------------------------------*\
| Each regional layout has its own PID, so the layout is     |
|   taken from the device entry rather than queried.         |
\*---------------------------------------------------------*/
unsigned int CorsairPeripheralV2K65PlusController::GetKeyboardLayout()
{
    switch(GetDeviceData()->pid)
    {
        case CORSAIR_K65_PLUS_UK_PID:
        case CORSAIR_K65_PLUS_V2_UK_PID:
            return CORSAIR_V2_KB_LAYOUT_ISO;

        case CORSAIR_K65_PLUS_JP_PID:
        case CORSAIR_K65_PLUS_V2_JP_PID:
            return CORSAIR_V2_KB_LAYOUT_JIS;

        default:
            return CORSAIR_V2_KB_LAYOUT_ANSI;
    }
}

void CorsairPeripheralV2K65PlusController::SetLedsDirect(std::vector<RGBColor *> colors)
{
    if(handle_open)
    {
        SetLedsDirectWired(colors);
    }
    else
    {
        SetLedsDirectWireless(colors);
    }
}

/*---------------------------------------------------------*\
| Per key colours addressed by packet index.  Each LED sits  |
|   at (index * 3) inside the colour buffer.                 |
\*---------------------------------------------------------*/
void CorsairPeripheralV2K65PlusController::SetLedsDirectWired(std::vector<RGBColor *>& colors)
{
    uint16_t             data_size  = CORSAIR_K65_PLUS_RGB_HEADER + CORSAIR_K65_PLUS_COLOR_BUF_SIZE;
    std::vector<uint8_t> buffer(data_size, 0);
    uint16_t             count      = (uint16_t)colors.size();

    buffer[0]               = 0x12;
    buffer[1]               = 0x00;

    for(uint16_t i = 0; i < count; i++)
    {
        uint16_t idx        = i * 3;

        if(idx + 2 >= CORSAIR_K65_PLUS_COLOR_BUF_SIZE)
        {
            break;
        }

        RGBColor color      = *colors[i];

        buffer[CORSAIR_K65_PLUS_RGB_HEADER + idx]       = RGBGetRValue(color);
        buffer[CORSAIR_K65_PLUS_RGB_HEADER + idx + 1]   = RGBGetGValue(color);
        buffer[CORSAIR_K65_PLUS_RGB_HEADER + idx + 2]   = RGBGetBValue(color);
    }

    /*---------------------------------------------------------*\
    | Usage 0 is a second copy of the spacebar and usage 1 is    |
    |   unused.  Mirror the spacebar into the first and clear    |
    |   the second so it does not light at random.               |
    \*---------------------------------------------------------*/
    if(CORSAIR_K65_PLUS_SPACEBAR_LED < count)
    {
        RGBColor space      = *colors[CORSAIR_K65_PLUS_SPACEBAR_LED];

        buffer[CORSAIR_K65_PLUS_RGB_HEADER]     = RGBGetRValue(space);
        buffer[CORSAIR_K65_PLUS_RGB_HEADER + 1] = RGBGetGValue(space);
        buffer[CORSAIR_K65_PLUS_RGB_HEADER + 2] = RGBGetBValue(space);
    }

    buffer[CORSAIR_K65_PLUS_RGB_HEADER + 3]     = 0;
    buffer[CORSAIR_K65_PLUS_RGB_HEADER + 4]     = 0;
    buffer[CORSAIR_K65_PLUS_RGB_HEADER + 5]     = 0;

    WriteBlock(0, buffer.data(), data_size);
}

/*---------------------------------------------------------*\
| The wireless firmware only renders the first colour group  |
|   of the frame, so the whole keyboard takes one colour.    |
\*---------------------------------------------------------*/
void CorsairPeripheralV2K65PlusController::SetLedsDirectWireless(std::vector<RGBColor *>& colors)
{
    RGBColor solid          = 0;

    for(size_t i = 0; i < colors.size(); i++)
    {
        if(*colors[i] != 0)
        {
            solid           = *colors[i];
            break;
        }
    }

    uint16_t             data_size  = 11 + k65_plus_num_indices;
    std::vector<uint8_t> payload(data_size, 0);

    payload[0]              = 0x7E;                     /* header flags                 */
    payload[1]              = 0x20;
    payload[2]              = 0x01;                     /* effect type, static          */
    payload[6]              = 0x01;                     /* one colour group             */
    payload[7]              = 0xFF;                     /* colours are ABGR ordered     */
    payload[8]              = RGBGetBValue(solid);
    payload[9]              = RGBGetGValue(solid);
    payload[10]             = RGBGetRValue(solid);

    memcpy(&payload[11], k65_plus_led_indices, k65_plus_num_indices);

    if(StartTransaction(CORSAIR_K65_PLUS_WIRELESS_HANDLE, CORSAIR_K65_PLUS_RES_LIGHTING) != 0)
    {
        SessionStart();
        SetRenderMode(CORSAIR_V2_MODE_SW);

        if(StartTransaction(CORSAIR_K65_PLUS_WIRELESS_HANDLE, CORSAIR_K65_PLUS_RES_LIGHTING) != 0)
        {
            return;
        }
    }

    WriteBlock(CORSAIR_K65_PLUS_WIRELESS_HANDLE, payload.data(), data_size);
    StopTransaction(CORSAIR_K65_PLUS_WIRELESS_HANDLE);

    WriteIndication(CORSAIR_K65_PLUS_RES_LIGHTING);
}

/*---------------------------------------------------------*\
| The wireless path only takes effect once the write is      |
|   acknowledged on the indication resource.                 |
\*---------------------------------------------------------*/
void CorsairPeripheralV2K65PlusController::WriteIndication(uint16_t resource)
{
    if(StartTransaction(CORSAIR_K65_PLUS_WIRELESS_HANDLE, CORSAIR_K65_PLUS_RES_INDICATION) != 0)
    {
        return;
    }

    uint8_t indication[8] =
    {
        0x69, 0x6C, 0x01, 0x00, 0x08, 0x00,
        (uint8_t)(resource & 0xFF),
        (uint8_t)((resource >> 8) & 0xFF),
    };

    WriteBlock(CORSAIR_K65_PLUS_WIRELESS_HANDLE, indication, sizeof(indication));
    StopTransaction(CORSAIR_K65_PLUS_WIRELESS_HANDLE);
}
