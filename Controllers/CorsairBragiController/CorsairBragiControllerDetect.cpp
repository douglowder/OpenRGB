/*---------------------------------------------------------*\
| CorsairBragiControllerDetect.cpp                          |
|                                                           |
| Detector for Corsair K65 Plus Wireless keyboard           |
|                                                           |
| This file is part of the OpenRGB project                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"
#include "CorsairBragiDevices.h"
#include "RGBController_CorsairBragi.h"

#define CORSAIR_VID                             0x1B1C

/*-----------------------------------------------------------------------------------------------------*\
| K65 Plus uses interface 1, no HID report IDs (65-byte packets with 0x00 prefix).                       |
\*-----------------------------------------------------------------------------------------------------*/
void DetectCorsairBragiControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        CorsairBragiController*     controller      = new CorsairBragiController(dev, info->path, name);

        if(controller->IsReady())
        {
            RGBController_CorsairBragi* rgb_controller  = new RGBController_CorsairBragi(controller);
            ResourceManager::get()->RegisterRGBController(rgb_controller);
        }
        else
        {
            delete controller;
        }
    }
}

/*-----------------------------------------------------------------------------------------------------*\
| Keyboards (via Slipstream dongle, interface 1, usage page 0xFF42)                                      |
\*-----------------------------------------------------------------------------------------------------*/
REGISTER_HID_DETECTOR_IP("Corsair K65 Plus Wireless",  DetectCorsairBragiControllers, CORSAIR_VID, CORSAIR_BRAGI_K65_PLUS_DONGLE_PID,    1, 0xFF42);

/*-----------------------------------------------------------------------------------------------------*\
| Keyboards (direct USB / wired, interface 1, usage page 0xFF42)                                         |
| These PIDs are also used as subdevice IDs behind the dongle, but here they appear as direct HID       |
| endpoints when the keyboard is connected over cable.                                                    |
\*-----------------------------------------------------------------------------------------------------*/
REGISTER_HID_DETECTOR_IP("Corsair K65 Plus (Wired)",   DetectCorsairBragiControllers, CORSAIR_VID, CORSAIR_BRAGI_K65_PLUS_NA_PID,        1, 0xFF42);
REGISTER_HID_DETECTOR_IP("Corsair K65 Plus (Wired)",   DetectCorsairBragiControllers, CORSAIR_VID, CORSAIR_BRAGI_K65_PLUS_UK_PID,        1, 0xFF42);
REGISTER_HID_DETECTOR_IP("Corsair K65 Plus (Wired)",   DetectCorsairBragiControllers, CORSAIR_VID, CORSAIR_BRAGI_K65_PLUS_JP_PID,        1, 0xFF42);
REGISTER_HID_DETECTOR_IP("Corsair K65 Plus V2 (Wired)",DetectCorsairBragiControllers, CORSAIR_VID, CORSAIR_BRAGI_K65_PLUS_V2_NA_PID,     1, 0xFF42);
REGISTER_HID_DETECTOR_IP("Corsair K65 Plus V2 (Wired)",DetectCorsairBragiControllers, CORSAIR_VID, CORSAIR_BRAGI_K65_PLUS_V2_UK_PID,     1, 0xFF42);
REGISTER_HID_DETECTOR_IP("Corsair K65 Plus V2 (Wired)",DetectCorsairBragiControllers, CORSAIR_VID, CORSAIR_BRAGI_K65_PLUS_V2_JP_PID,     1, 0xFF42);
