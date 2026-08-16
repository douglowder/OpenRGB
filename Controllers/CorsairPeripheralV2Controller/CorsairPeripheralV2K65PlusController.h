/*---------------------------------------------------------*\
| CorsairPeripheralV2K65PlusController.h                    |
|                                                           |
|   Driver for the Corsair K65 Plus Wireless keyboard,      |
|   wired or behind the Slipstream dongle                   |
|                                                           |
|   ToastKiste21                                28 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "CorsairPeripheralV2Controller.h"

/*---------------------------------------------------------*\
| The K65 Plus keeps its lighting handle open across frames  |
|   instead of opening one per update, and addresses keys by |
|   packet index rather than by a packed colour block.       |
\*---------------------------------------------------------*/
#define CORSAIR_K65_PLUS_RGB_HEADER         2       /* 0x12, 0x00 ahead of the colours  */
#define CORSAIR_K65_PLUS_COLOR_BUF_SIZE     371     /* packet-index addressed RGB       */
#define CORSAIR_K65_PLUS_SPACEBAR_LED       44      /* usage 0 mirrors the spacebar     */

/*---------------------------------------------------------*\
| Wireless lighting goes through a separate resource that    |
|   is opened, written, closed and then acknowledged.        |
\*---------------------------------------------------------*/
#define CORSAIR_K65_PLUS_RES_LIGHTING       0x6D60
#define CORSAIR_K65_PLUS_RES_INDICATION     0x0013
#define CORSAIR_K65_PLUS_WIRELESS_HANDLE    0x01

/*---------------------------------------------------------*\
| Device wide brightness, 0 to 1000.                         |
\*---------------------------------------------------------*/
#define CORSAIR_K65_PLUS_PROP_BRIGHTNESS    0x02
#define CORSAIR_K65_PLUS_BRIGHTNESS_MAX     1000

class CorsairPeripheralV2K65PlusController : public CorsairPeripheralV2Controller
{
public:
    CorsairPeripheralV2K65PlusController(hid_device* dev_handle, const char* path, std::string name);
    ~CorsairPeripheralV2K65PlusController();

    unsigned int                    GetKeyboardLayout()                             override;
    bool                            SupportsBrightness()                            override;
    bool                            SupportsSave()                                  override;
    void                            SaveLedsDirect(std::vector<RGBColor *> colors)   override;
    void                            SetBrightness(uint8_t percent)                  override;
    void                            SetLedsDirect(std::vector<RGBColor *> colors)    override;

private:
    void                            SetLedsDirectWired(std::vector<RGBColor *>& colors);
    void                            WriteStaticEffect(RGBColor color);
    void                            WriteIndication(uint16_t resource);
    RGBColor                        FirstLitColor(std::vector<RGBColor *>& colors);
    void                            OpenLightingHandle();

    bool                            handle_open     = false;
};
