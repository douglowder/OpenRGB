/*---------------------------------------------------------*\
| RGBController_CorsairBragi.h                              |
|                                                           |
| RGBController for Corsair K65 Plus Wireless keyboard      |
|                                                           |
| This file is part of the OpenRGB project                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "CorsairBragiController.h"

class RGBController_CorsairBragi : public RGBController
{
public:
    RGBController_CorsairBragi(CorsairBragiController* controller_ptr);
    ~RGBController_CorsairBragi();

    void SetupZones();
    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();
    void KeepaliveThread();

private:
    CorsairBragiController*                 controller;

    RGBColor                                null_color              = 0;
    std::vector<RGBColor *>                 buffer_map;

    std::thread*                            keepalive_thread;
    std::atomic<bool>                       keepalive_thread_run;
    std::chrono::time_point
        <std::chrono::steady_clock>         last_update_time;
};
