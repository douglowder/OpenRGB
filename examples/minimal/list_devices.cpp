/*---------------------------------------------------------*\
| list_devices.cpp                                          |
|                                                           |
|   A minimal consumer of the OpenRGB core library.  Uses    |
|   no Qt and no toolkit at all: it initialises the library, |
|   waits for detection, and prints what it found.           |
|                                                           |
|   Build against an installed prefix:                       |
|                                                           |
|       c++ $(pkg-config --cflags --libs openrgb) \          |
|           list_devices.cpp -o list_devices                 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdio>
#include <openrgb/OpenRGB.h>

int main()
{
    ResourceManager* rm = ResourceManager::get();

    /*-----------------------------------------------------*\
    | Do not auto-connect to a running server, do detect     |
    | devices, do not start a server, no CLI post options.   |
    \*-----------------------------------------------------*/
    rm->Initialize(false, true, false, false);
    rm->WaitForInitialization();
    rm->WaitForDetection();

    std::vector<RGBController*>& devices = rm->GetRGBControllers();

    printf("%zu device(s)\n", devices.size());

    for(std::size_t i = 0; i < devices.size(); i++)
    {
        RGBController* dev = devices[i];

        printf("  %zu: %s\n", i, dev->GetName().c_str());
        printf("      vendor      %s\n", dev->GetVendor().c_str());
        printf("      location    %s\n", dev->GetLocation().c_str());
        printf("      zones %u, leds %u, modes %u\n",
               (unsigned int)dev->GetZoneCount(),
               (unsigned int)dev->GetLEDCount(),
               (unsigned int)dev->GetModeCount());
    }

    /*-----------------------------------------------------*\
    | The library owns the devices; this destroys them.      |
    \*-----------------------------------------------------*/
    DetectionManager::get()->Cleanup();

    return 0;
}
