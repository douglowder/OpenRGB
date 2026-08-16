/*---------------------------------------------------------*\
| RGBController_CorsairV2K65Plus.cpp                        |
|                                                           |
|   RGBController for the Corsair K65 Plus Wireless          |
|                                                           |
|   ToastKiste21                                28 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_CorsairV2K65Plus.h"

/**------------------------------------------------------------------*\
    @name Corsair K65 Plus Wireless
    @category Keyboard
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectCorsairV2K65PlusControllers
    @comment Wired gives per key colour. Behind the Slipstream dongle
        the firmware renders only one colour for the whole keyboard.
        Saving stores a single colour that the keyboard replays on its
        own over any transport, so a per key layout cannot be saved.
\*-------------------------------------------------------------------*/

RGBController_CorsairV2K65Plus::RGBController_CorsairV2K65Plus(CorsairPeripheralV2Controller *controller_ptr)
    : RGBController_CorsairV2SW(controller_ptr)
{
}
