/*---------------------------------------------------------*\
| OpenRGBPluginAPIInterface.h                               |
|                                                           |
|   OpenRGB Plugin API, the interface OpenRGB provides to    |
|   plugins.  Split out of OpenRGBPluginInterface.h so that  |
|   it can live in the headless core, which must not depend  |
|   on Qt.  The plugin-provided half stays in                |
|   OpenRGBPluginInterface.h alongside the GUI.              |
|                                                           |
|   herosilas12 (CoffeeIsLife)                  11 Dec 2020 |
|   Adam Honse (CalcProgrammer1)                05 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <nlohmann/json.hpp>
#include "filesystem.h"
#include "RGBControllerInterface.h"

class OpenRGBPluginAPIInterface
{
public:
    /*-----------------------------------------------------*\
    | LogManager APIs                                       |
    \*-----------------------------------------------------*/
    virtual void                                    LogEntry(const char* filename, int line, unsigned int level, const char* fmt, ...)                      = 0;

    /*-----------------------------------------------------*\
    | PluginManager APIs                                    |
    \*-----------------------------------------------------*/
    virtual RGBControllerInterface*                 CreateVirtualRGBController(RGBController_Setup* setup)                                                  = 0;
    virtual void                                    DeleteVirtualRGBController(RGBControllerInterface* rgb_controller)                                      = 0;
    virtual void                                    RegisterVirtualRGBController(RGBControllerInterface* rgb_controller)                                    = 0;
    virtual void                                    RegisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller)                            = 0;
    virtual void                                    UnregisterVirtualRGBController(RGBControllerInterface* rgb_controller)                                  = 0;
    virtual void                                    UpdateVirtualRGBController(RGBControllerInterface* rgb_controller, RGBController_Setup* setup)          = 0;
    virtual void                                    UnregisterVirtualRGBControllerInThread(RGBControllerInterface* rgb_controller)                          = 0;

    /*-----------------------------------------------------*\
    | ProfileManager APIs                                   |
    \*-----------------------------------------------------*/
    virtual void                                    ClearActiveProfile()                                                                                    = 0;
    virtual std::vector<std::string>                GetProfileList()                                                                                        = 0;
    virtual bool                                    LoadProfile(std::string profile_name)                                                                   = 0;
    virtual bool                                    SaveProfileFromPlugin(std::string profile_name, std::string plugin_name, nlohmann::json plugin_data)    = 0;

    /*-----------------------------------------------------*\
    | ResourceManager APIs                                  |
    \*-----------------------------------------------------*/
    virtual filesystem::path                        GetConfigurationDirectory()                                                                             = 0;
    virtual bool                                    GetDetectionEnabled()                                                                                   = 0;
    virtual unsigned int                            GetDetectionPercent()                                                                                   = 0;
    virtual std::string                             GetDetectionString()                                                                                    = 0;
    virtual void                                    RescanDevices()                                                                                         = 0;
    virtual void                                    WaitForDetection()                                                                                      = 0;
    virtual std::vector<RGBControllerInterface*>    GetRGBControllers()                                                                                     = 0;

    /*-----------------------------------------------------*\
    | RGBController APIs                                    |
    \*-----------------------------------------------------*/
    virtual nlohmann::json                          GetDeviceDescriptionJSON(RGBControllerInterface* controller)                                            = 0;
    virtual nlohmann::json                          GetLEDDescriptionJSON(led led)                                                                          = 0;
    virtual nlohmann::json                          GetMatrixMapDescriptionJSON(matrix_map_type matrix_map)                                                 = 0;
    virtual nlohmann::json                          GetModeDescriptionJSON(mode mode)                                                                       = 0;
    virtual nlohmann::json                          GetSegmentDescriptionJSON(segment segment)                                                              = 0;
    virtual nlohmann::json                          GetZoneDescriptionJSON(zone zone)                                                                       = 0;

    virtual RGBControllerInterface*                 SetDeviceDescriptionJSON(nlohmann::json controller_json)                                                = 0;
    virtual led                                     SetLEDDescriptionJSON(nlohmann::json led_json)                                                          = 0;
    virtual matrix_map_type                         SetMatrixMapDescriptionJSON(nlohmann::json matrix_map_json)                                             = 0;
    virtual mode                                    SetModeDescriptionJSON(nlohmann::json mode_json)                                                        = 0;
    virtual segment                                 SetSegmentDescriptionJSON(nlohmann::json segment_json)                                                  = 0;
    virtual zone                                    SetZoneDescriptionJSON(nlohmann::json zone_json)                                                        = 0;

    virtual bool                                    CompareControllers(RGBControllerInterface* controller_1, RGBControllerInterface* controller_2)          = 0;
    virtual std::string                             DeviceTypeToString(device_type type)                                                                    = 0;
    virtual bool                                    SetModeValuesFromMode(mode& destination, mode& source)                                                  = 0;

    /*-----------------------------------------------------*\
    | SettingsManager APIs                                  |
    \*-----------------------------------------------------*/
    virtual nlohmann::json                          GetSettings(std::string settings_key)                                                                   = 0;
    virtual void                                    SaveSettings()                                                                                          = 0;
    virtual void                                    SetSettings(std::string settings_key, nlohmann::json new_settings)                                      = 0;
};
