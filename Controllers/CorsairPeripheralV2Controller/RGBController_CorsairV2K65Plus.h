/*---------------------------------------------------------*\
| RGBController_CorsairV2K65Plus.h                          |
|                                                           |
|   RGBController for the Corsair K65 Plus Wireless          |
|                                                           |
|   ToastKiste21                                28 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController_CorsairV2Software.h"

/*---------------------------------------------------------*\
| Behaves exactly like the other V2 software mode devices.   |
|   It exists so that the generated device list records the  |
|   brightness and save support that the other V2 devices    |
|   have not been tested for.                                |
\*---------------------------------------------------------*/
class RGBController_CorsairV2K65Plus : public RGBController_CorsairV2SW
{
public:
    RGBController_CorsairV2K65Plus(CorsairPeripheralV2Controller* controller_ptr);
};
