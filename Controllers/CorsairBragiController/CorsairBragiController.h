/*---------------------------------------------------------*\
| CorsairBragiController.h                                  |
|                                                           |
|   Driver for Corsair K65 Plus Wireless keyboard           |
|   (Bragi protocol over Slipstream dongle or USB)          |
|                                                           |
|   ToastKiste21                                28 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"
#include "CorsairPeripheralV2Devices.h"

/*---------------------------------------------------------*\
| Bragi HID framing constants                               |
|   K65 Plus uses interface 1 with no HID report IDs:       |
|   Write: [0x00, data...] 65 bytes total                   |
|   Read:  [data...] - use as-is                            |
\*---------------------------------------------------------*/
#define BRAGI_WRITE_SIZE            64      /* base packet size                    */
#define BRAGI_BUFFER_SIZE           65      /* 1 (0x00 prefix) + 64 data bytes     */
#define BRAGI_TIMEOUT               250
#define BRAGI_TIMEOUT_SHORT         10

/*---------------------------------------------------------*\
| Bragi protocol constants                                  |
\*---------------------------------------------------------*/
#define BRAGI_MAGIC                 0x08

#define BRAGI_CMD_SET               0x01
#define BRAGI_CMD_GET               0x02
#define BRAGI_CMD_CLOSE_HANDLE      0x05
#define BRAGI_CMD_WRITE_DATA        0x06
#define BRAGI_CMD_CONTINUE_WRITE    0x07
#define BRAGI_CMD_OPEN_HANDLE       0x0D
#define BRAGI_CMD_SESSION_START     0x12

#define BRAGI_PROP_MODE             0x03
#define BRAGI_PROP_BRIGHTNESS       0x02
#define BRAGI_PROP_PID              0x12
#define BRAGI_PROP_VID              0x11
#define BRAGI_PROP_SUBDEVICE_BITMAP 0x36

#define BRAGI_MODE_HW               0x01
#define BRAGI_MODE_SW               0x02

#define BRAGI_RES_ALT_LIGHTING      0x22
#define BRAGI_LIGHTING_HANDLE       0x00

#define BRAGI_ALT_RGB_HEADER        2       /* 0x12, 0x00 header for ALT_LIGHTING  */
#define BRAGI_K65_COLOR_BUF_SIZE    371     /* PacketIndex-addressed RGB buffer    */

/*---------------------------------------------------------*\
| K65 Plus keyboard lighting constants                      |
\*---------------------------------------------------------*/
#define BRAGI_K65_RES_LIGHTING      0x6D60
#define BRAGI_K65_RES_INDICATION    0x0013
#define BRAGI_K65_LIGHTING_HANDLE   0x01

#define BRAGI_UPDATE_PERIOD         30000
#define BRAGI_SLEEP_PERIOD          12500ms

class CorsairBragiController
{
public:
    CorsairBragiController(hid_device* dev_handle, const char* path, std::string name);
    ~CorsairBragiController();

    std::string                     GetDeviceLocation();
    std::string                     GetFirmwareString();
    std::string                     GetName();
    std::string                     GetSerialString();
    const corsair_v2_device*        GetDeviceData();

    void                            SetLedsDirect(std::vector<RGBColor *> colors);
    void                            SetBrightness(uint16_t value);
    unsigned int                    GetKeyboardLayout();
    bool                            IsReady();

private:
    hid_device*                     dev;
    std::string                     device_name;
    std::string                     location;
    uint16_t                        device_index;
    uint8_t                         child_id        = 0;
    bool                            handle_open     = false;
    bool                            device_ready    = false;
    bool                            via_dongle      = false;

    /*---------------------------------------------------------*\
    | Low-level HID communication                               |
    \*---------------------------------------------------------*/
    int                             SendRecv(uint8_t* buffer, uint8_t* response);

    /*---------------------------------------------------------*\
    | Bragi protocol operations                                 |
    \*---------------------------------------------------------*/
    unsigned int                    GetProperty(uint8_t prop);
    int                             SetProperty(uint8_t prop, uint16_t value);
    int                             OpenHandle(uint8_t handle, uint16_t resource);
    void                            CloseHandle(uint8_t handle);
    void                            WriteToHandle(uint8_t handle, uint8_t* data, uint16_t data_size);
    void                            SendSessionStart();
    void                            SetRenderMode(uint8_t mode);
    void                            DetectSubdevice();
    void                            SendK65LightingFrame(std::vector<RGBColor *>& colors);
    void                            WriteIndicationHandle(uint16_t resource);
};
