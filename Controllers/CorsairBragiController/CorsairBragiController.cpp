/*---------------------------------------------------------*\
| CorsairBragiController.cpp                                |
|                                                           |
|   Driver for Corsair K65 Plus Wireless keyboard           |
|   (Bragi protocol over Slipstream dongle or USB)          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "CorsairBragiController.h"
#include "CorsairBragiDevices.h"
#include "LogManager.h"
#include "StringUtils.h"
#include <cstring>

namespace
{
/* 83 entries, HID usage IDs in the order used by K65 lighting payloads. */
static const uint8_t k65_led_indices[] =
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

constexpr uint16_t k65_num_indices = sizeof(k65_led_indices);
}

CorsairBragiController::CorsairBragiController(hid_device* dev_handle, const char* path, std::string name)
{
    dev             = dev_handle;
    location        = path;
    device_name     = name;

    /*---------------------------------------------------------*\
    | Drain any stale data from the HID read buffer              |
    \*---------------------------------------------------------*/
    uint8_t drain_buf[BRAGI_BUFFER_SIZE];
    int drain_count = 0;
    while(hid_read_timeout(dev, drain_buf, sizeof(drain_buf), BRAGI_TIMEOUT_SHORT) > 0)
    {
        drain_count++;
        if(drain_count > 20)
        {
            break;
        }
    }

    /*---------------------------------------------------------*\
    | Detect dongle and route to keyboard subdevice              |
    \*---------------------------------------------------------*/
    DetectSubdevice();

    /*---------------------------------------------------------*\
    | Get the actual device PID (keyboard behind dongle)         |
    \*---------------------------------------------------------*/
    unsigned int pid = GetProperty(BRAGI_PROP_PID);

    /*---------------------------------------------------------*\
    | Find the device in our device list                         |
    \*---------------------------------------------------------*/
    bool found = false;
    for(uint16_t i = 0; i < CORSAIR_BRAGI_DEVICE_COUNT; i++)
    {
        if(corsair_bragi_device_list[i]->pid == pid)
        {
            device_index = i;
            found = true;
            break;
        }
    }

    if(!found)
    {
        LOG_ERROR("[%s] Bragi device PID %04X not found in device list", device_name.c_str(), pid);
        return;
    }

    /*---------------------------------------------------------*\
    | Initialize: set software mode and open lighting            |
    |                                                           |
    | Wired:    persistent ALT_LIGHTING handle on handle 0       |
    |           for per-key RGB via PacketIndex format.           |
    | Wireless: resource-based 0x6D60 path only (solid color).   |
    |           ALT_LIGHTING opens without error wirelessly but   |
    |           LEDs do not respond — handle_open stays false.    |
    \*---------------------------------------------------------*/
    SendSessionStart();
    SetRenderMode(BRAGI_MODE_SW);

    if(!via_dongle)
    {
        int err = OpenHandle(BRAGI_LIGHTING_HANDLE, BRAGI_RES_ALT_LIGHTING);
        if(err == 0)
        {
            handle_open = true;
        }
        else
        {
            LOG_ERROR("[%s] ALT_LIGHTING handle failed (err=%d)", device_name.c_str(), err);
        }
    }

    device_ready = true;
}

CorsairBragiController::~CorsairBragiController()
{
    if(handle_open)
    {
        CloseHandle(BRAGI_LIGHTING_HANDLE);
    }
    if(device_ready)
    {
        SetRenderMode(BRAGI_MODE_HW);
    }
    hid_close(dev);
}

/*---------------------------------------------------------*\
| Public getters                                             |
\*---------------------------------------------------------*/

bool CorsairBragiController::IsReady()
{
    return device_ready;
}

const corsair_v2_device* CorsairBragiController::GetDeviceData()
{
    return corsair_bragi_device_list[device_index];
}

std::string CorsairBragiController::GetDeviceLocation()
{
    return "HID: " + location;
}

std::string CorsairBragiController::GetFirmwareString()
{
    return "";
}

std::string CorsairBragiController::GetName()
{
    return device_name;
}

std::string CorsairBragiController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return "";
    }

    return StringUtils::wstring_to_string(serial_string);
}

/*---------------------------------------------------------*\
| Low-level HID communication                               |
|                                                           |
| K65 Plus uses interface 1 with no HID report IDs:         |
|   Write: [0x00, data...] 65 bytes total                    |
|   Read:  [data...] - use as-is, no stripping               |
\*---------------------------------------------------------*/

int CorsairBragiController::SendRecv(uint8_t* buffer, uint8_t* response)
{
    hid_write(dev, buffer, BRAGI_BUFFER_SIZE);

    uint8_t read_buf[BRAGI_BUFFER_SIZE];
    memset(read_buf, 0, sizeof(read_buf));

    int result = hid_read_timeout(dev, read_buf, sizeof(read_buf), BRAGI_TIMEOUT);

    if(result <= 0)
    {
        return result;
    }

    memcpy(response, read_buf, result);
    return result;
}

/*---------------------------------------------------------*\
| Bragi protocol operations                                  |
\*---------------------------------------------------------*/

unsigned int CorsairBragiController::GetProperty(uint8_t prop)
{
    uint8_t buffer[BRAGI_BUFFER_SIZE];
    uint8_t response[BRAGI_BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));

    buffer[1] = BRAGI_MAGIC | child_id;
    buffer[2] = BRAGI_CMD_GET;
    buffer[3] = prop;

    int result = SendRecv(buffer, response);

    if(result < 6)
    {
        return (unsigned int)-1;
    }

    /*---------------------------------------------------------*\
    | response[2] = error code (0 = success)                     |
    | response[3..5] = value (24-bit little-endian)              |
    \*---------------------------------------------------------*/
    if(response[2] != 0)
    {
        return (unsigned int)-1;
    }

    return response[3] | (response[4] << 8) | (response[5] << 16);
}

int CorsairBragiController::SetProperty(uint8_t prop, uint16_t value)
{
    uint8_t buffer[BRAGI_BUFFER_SIZE];
    uint8_t response[BRAGI_BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));

    buffer[1] = BRAGI_MAGIC | child_id;
    buffer[2] = BRAGI_CMD_SET;
    buffer[3] = prop;
    buffer[5] = value & 0xFF;
    buffer[6] = (value >> 8) & 0xFF;

    int result = SendRecv(buffer, response);

    if(result < 3)
    {
        return -1;
    }

    return response[2];
}

int CorsairBragiController::OpenHandle(uint8_t handle, uint16_t resource)
{
    uint8_t buffer[BRAGI_BUFFER_SIZE];
    uint8_t response[BRAGI_BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));

    buffer[1] = BRAGI_MAGIC | child_id;
    buffer[2] = BRAGI_CMD_OPEN_HANDLE;
    buffer[3] = handle;
    buffer[4] = resource & 0xFF;
    buffer[5] = (resource >> 8) & 0xFF;

    int result = SendRecv(buffer, response);

    if(result < 3)
    {
        return -1;
    }

    /*---------------------------------------------------------*\
    | Error 0x03 = handle already open, close and retry          |
    \*---------------------------------------------------------*/
    if(response[2] == 0x03)
    {
        LOG_DEBUG("[%s] Handle 0x%02X already open, closing and reopening", device_name.c_str(), handle);
        CloseHandle(handle);
        result = SendRecv(buffer, response);
        if(result < 3)
        {
            return -1;
        }
    }

    if(response[2] != 0)
    {
        LOG_ERROR("[%s] OpenHandle 0x%02X resource 0x%04X failed: error 0x%02X",
                  device_name.c_str(), handle, resource, response[2]);
    }

    return response[2];
}

void CorsairBragiController::CloseHandle(uint8_t handle)
{
    uint8_t buffer[BRAGI_BUFFER_SIZE];
    uint8_t response[BRAGI_BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));

    buffer[1] = BRAGI_MAGIC | child_id;
    buffer[2] = BRAGI_CMD_CLOSE_HANDLE;
    buffer[3] = 0x01;
    buffer[4] = handle;

    SendRecv(buffer, response);
}

void CorsairBragiController::WriteToHandle(uint8_t handle, uint8_t* data, uint16_t data_size)
{
    uint8_t buffer[BRAGI_BUFFER_SIZE];
    uint8_t response[BRAGI_BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));

    /*---------------------------------------------------------*\
    | First packet: [0x00, magic|child, WRITE, handle,            |
    |                len_b0, len_b1, len_b2, len_b3, data...]     |
    | Header = 8 bytes. 65 bytes total → 57 data bytes.           |
    \*---------------------------------------------------------*/
    buffer[1] = BRAGI_MAGIC | child_id;
    buffer[2] = BRAGI_CMD_WRITE_DATA;
    buffer[3] = handle;
    buffer[4] = data_size & 0xFF;
    buffer[5] = (data_size >> 8) & 0xFF;
    buffer[6] = (data_size >> 16) & 0xFF;
    buffer[7] = (data_size >> 24) & 0xFF;

    uint16_t first_chunk = BRAGI_BUFFER_SIZE - 8;
    if(first_chunk > data_size)
    {
        first_chunk = data_size;
    }

    memcpy(&buffer[8], data, first_chunk);

    SendRecv(buffer, response);

    /*---------------------------------------------------------*\
    | Continue packets if data doesn't fit in first packet       |
    | [0x00, magic|child, CONTINUE, handle, data...]              |
    | Header = 4 bytes. Data = 61 bytes per packet.               |
    \*---------------------------------------------------------*/
    uint16_t offset = first_chunk;
    while(offset < data_size)
    {
        memset(buffer, 0, sizeof(buffer));

        buffer[1] = BRAGI_MAGIC | child_id;
        buffer[2] = BRAGI_CMD_CONTINUE_WRITE;
        buffer[3] = handle;

        uint16_t chunk = BRAGI_BUFFER_SIZE - 4;
        if(chunk > data_size - offset)
        {
            chunk = data_size - offset;
        }

        memcpy(&buffer[4], &data[offset], chunk);

        SendRecv(buffer, response);
        offset += chunk;
    }
}

void CorsairBragiController::SetRenderMode(uint8_t mode)
{
    SetProperty(BRAGI_PROP_MODE, mode);
}

void CorsairBragiController::DetectSubdevice()
{
    /*---------------------------------------------------------*\
    | Query PID to check if we're talking to the dongle          |
    \*---------------------------------------------------------*/
    unsigned int pid = GetProperty(BRAGI_PROP_PID);

    if(pid == (unsigned int)-1)
    {
        LOG_ERROR("[%s] Cannot communicate with device", device_name.c_str());
        return;
    }

    /*---------------------------------------------------------*\
    | Check if this is a known dongle PID                        |
    \*---------------------------------------------------------*/
    bool is_dongle = false;
    for(uint16_t i = 0; i < CORSAIR_BRAGI_DONGLE_COUNT; i++)
    {
        if(corsair_bragi_dongle_pids[i] == pid)
        {
            is_dongle = true;
            break;
        }
    }

    if(!is_dongle)
    {
        /*-----------------------------------------------------*\
        | Direct USB (wired). K65 keyboards use child_id=1 on     |
        | interface 1. Probe child_id=1 first, fall back to 0.   |
        \*-----------------------------------------------------*/
        child_id = 1;
        SendSessionStart();

        if(GetProperty(BRAGI_PROP_PID) != (unsigned int)-1)
        {
            return;
        }

        child_id = 0;
        LOG_DEBUG("[%s] child_id=1 probe failed, falling back to child_id=0", device_name.c_str());
        return;
    }

    via_dongle = true;

    /*---------------------------------------------------------*\
    | Read subdevice bitmap (property 0x36)                      |
    \*---------------------------------------------------------*/
    unsigned int subdevs = GetProperty(BRAGI_PROP_SUBDEVICE_BITMAP);

    if(subdevs == (unsigned int)-1 || subdevs == 0)
    {
        LOG_ERROR("[%s] No subdevices found on dongle", device_name.c_str());
        return;
    }

    /*---------------------------------------------------------*\
    | Find the first connected subdevice (bits 1-7).              |
    | Send session start to wake it up, retry with increasing    |
    | timeouts. Do NOT send session start to child_id=0 — the    |
    | K65 dongle returns error 0x07 and kills all subsequent     |
    | communication.                                             |
    \*---------------------------------------------------------*/
    for(int i = 1; i < 8; i++)
    {
        if(!((subdevs >> i) & 1))
        {
            continue;
        }

        child_id = i;
        bool subdevice_found = false;

        for(int attempt = 0; attempt < 3 && !subdevice_found; attempt++)
        {
            int wake_timeout = (attempt + 1) * 1000;

            uint8_t wake_buf[BRAGI_BUFFER_SIZE];
            uint8_t wake_resp[BRAGI_BUFFER_SIZE];

            memset(wake_buf, 0, sizeof(wake_buf));
            wake_buf[1] = BRAGI_MAGIC | child_id;
            wake_buf[2] = BRAGI_CMD_SESSION_START;

            hid_write(dev, wake_buf, BRAGI_BUFFER_SIZE);

            memset(wake_resp, 0, sizeof(wake_resp));
            int r1 = hid_read_timeout(dev, wake_resp, sizeof(wake_resp), wake_timeout);

            if(r1 <= 0)
            {
                continue;
            }

            /*-------------------------------------------------*\
            | Drain any additional responses                      |
            \*-------------------------------------------------*/
            uint8_t drain_buf[BRAGI_BUFFER_SIZE];
            while(hid_read_timeout(dev, drain_buf, sizeof(drain_buf), BRAGI_TIMEOUT_SHORT) > 0)
            {
            }

            unsigned int sub_pid = GetProperty(BRAGI_PROP_PID);
            if(sub_pid != (unsigned int)-1)
            {
                subdevice_found = true;
            }
        }

        if(subdevice_found)
        {
            return;
        }

        LOG_ERROR("[%s] Subdevice %d not responding after 3 attempts", device_name.c_str(), i);
        child_id = 0;
    }
}

/*---------------------------------------------------------*\
| SetLedsDirect                                              |
|                                                           |
| Wired:    per-key RGB via persistent ALT_LIGHTING handle   |
|           (PacketIndex format, 373 bytes).                  |
| Wireless: solid color via resource 0x6D60 with             |
|           open/write/close + indication cycle.              |
|           Per-key not supported by wireless firmware.       |
\*---------------------------------------------------------*/

void CorsairBragiController::SetLedsDirect(std::vector<RGBColor *> colors)
{
    if(handle_open)
    {
        /*-----------------------------------------------------*\
        | Wired: per-key via persistent ALT_LIGHTING handle.      |
        | [0x12, 0x00, <371-byte PacketIndex RGB buffer>]         |
        | Each key at offset (buffer_value * 3).                  |
        \*-----------------------------------------------------*/
        uint16_t data_size = BRAGI_ALT_RGB_HEADER + BRAGI_K65_COLOR_BUF_SIZE;
        std::vector<uint8_t> buffer(data_size, 0);
        buffer[0] = 0x12;
        buffer[1] = 0x00;

        uint16_t num_entries = (uint16_t)colors.size();

        for(uint16_t i = 0; i < num_entries; i++)
        {
            uint16_t packet_idx = i * 3;
            if(packet_idx + 2 >= BRAGI_K65_COLOR_BUF_SIZE)
            {
                continue;
            }

            RGBColor color = *colors[i];
            buffer[BRAGI_ALT_RGB_HEADER + packet_idx + 0] = RGBGetRValue(color);
            buffer[BRAGI_ALT_RGB_HEADER + packet_idx + 1] = RGBGetGValue(color);
            buffer[BRAGI_ALT_RGB_HEADER + packet_idx + 2] = RGBGetBValue(color);
        }

        /*-----------------------------------------------------*\
        | Spacebar fix: usage 0 maps to spacebar color.           |
        | Usage 1 is unused — zero it out.                        |
        \*-----------------------------------------------------*/
        if(44 < num_entries)
        {
            RGBColor space_color = *colors[44];
            buffer[BRAGI_ALT_RGB_HEADER + 0] = RGBGetRValue(space_color);
            buffer[BRAGI_ALT_RGB_HEADER + 1] = RGBGetGValue(space_color);
            buffer[BRAGI_ALT_RGB_HEADER + 2] = RGBGetBValue(space_color);
        }
        buffer[BRAGI_ALT_RGB_HEADER + 3] = 0;
        buffer[BRAGI_ALT_RGB_HEADER + 4] = 0;
        buffer[BRAGI_ALT_RGB_HEADER + 5] = 0;

        WriteToHandle(BRAGI_LIGHTING_HANDLE, buffer.data(), data_size);
    }
    else
    {
        /*-----------------------------------------------------*\
        | Wireless: solid color via resource 0x6D60.               |
        \*-----------------------------------------------------*/
        SendK65LightingFrame(colors);
    }
}

void CorsairBragiController::SetBrightness(uint16_t value)
{
    SetProperty(BRAGI_PROP_BRIGHTNESS, value);
}

unsigned int CorsairBragiController::GetKeyboardLayout()
{
    const corsair_v2_device* device = GetDeviceData();

    switch(device->pid)
    {
        case CORSAIR_BRAGI_K65_PLUS_UK_PID:
        case CORSAIR_BRAGI_K65_PLUS_V2_UK_PID:
            return CORSAIR_V2_KB_LAYOUT_ISO;

        case CORSAIR_BRAGI_K65_PLUS_JP_PID:
        case CORSAIR_BRAGI_K65_PLUS_V2_JP_PID:
            return CORSAIR_V2_KB_LAYOUT_JIS;

        default:
            return CORSAIR_V2_KB_LAYOUT_ANSI;
    }
}

/*---------------------------------------------------------*\
| K65 Plus keyboard lighting methods                        |
\*---------------------------------------------------------*/

void CorsairBragiController::SendSessionStart()
{
    uint8_t buffer[BRAGI_BUFFER_SIZE];
    uint8_t response[BRAGI_BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));
    buffer[1] = BRAGI_MAGIC | child_id;
    buffer[2] = BRAGI_CMD_SESSION_START;
    SendRecv(buffer, response);

    /*-----------------------------------------------------*\
    | NOTE: Do NOT send session start to child_id=0.          |
    | The K65 dongle returns error 0x07 and stops responding  |
    | to all subsequent commands until re-plugged.            |
    \*-----------------------------------------------------*/
}

void CorsairBragiController::SendK65LightingFrame(std::vector<RGBColor *>& colors)
{
    /*---------------------------------------------------------*\
    | K65 Plus wireless lighting frame (resource 0x6D60):        |
    |   Payload: [header(2), type(2), params(2), num_colors(1),  |
    |             ABGR(4), LED indices(83)]                       |
    | Each frame: open → write → close → indication 0x0013       |
    |                                                           |
    | Solid color only — multi-group payloads tested but          |
    | firmware ignores all groups beyond the first.              |
    \*---------------------------------------------------------*/
    uint16_t num_leds = (uint16_t)colors.size();
    RGBColor solid = 0;
    for(uint16_t i = 0; i < num_leds; i++)
    {
        if(*colors[i] != 0)
        {
            solid = *colors[i];
            break;
        }
    }

    uint16_t data_size = 7 + 4 + k65_num_indices;  /* header(7) + ABGR(4) + indices(83) = 94 */
    std::vector<uint8_t> payload(data_size, 0);

    payload[0] = 0x7E;                         /* header flags */
    payload[1] = 0x20;
    payload[2] = 0x01;                         /* effect type = static */
    payload[3] = 0x00;
    payload[4] = 0x00;                         /* params */
    payload[5] = 0x00;
    payload[6] = 0x01;                         /* 1 color group */
    payload[7] = 0xFF;                         /* Alpha */
    payload[8] = RGBGetBValue(solid);          /* Blue  (ABGR order!) */
    payload[9] = RGBGetGValue(solid);          /* Green */
    payload[10] = RGBGetRValue(solid);         /* Red   */

    memcpy(&payload[11], k65_led_indices, k65_num_indices);

    if(OpenHandle(BRAGI_K65_LIGHTING_HANDLE, BRAGI_K65_RES_LIGHTING) != 0)
    {
        SendSessionStart();
        SetRenderMode(BRAGI_MODE_SW);
        if(OpenHandle(BRAGI_K65_LIGHTING_HANDLE, BRAGI_K65_RES_LIGHTING) != 0)
        {
            return;
        }
    }

    WriteToHandle(BRAGI_K65_LIGHTING_HANDLE, payload.data(), data_size);
    CloseHandle(BRAGI_K65_LIGHTING_HANDLE);

    WriteIndicationHandle(BRAGI_K65_RES_LIGHTING);
}

void CorsairBragiController::WriteIndicationHandle(uint16_t resource)
{
    if(OpenHandle(BRAGI_K65_LIGHTING_HANDLE, BRAGI_K65_RES_INDICATION) != 0)
    {
        return;
    }

    uint8_t indication_data[8] =
    {
        0x69,
        0x6C,
        0x01,
        0x00,
        0x08,
        0x00,
        (uint8_t)(resource & 0xFF),
        (uint8_t)((resource >> 8) & 0xFF),
    };
    WriteToHandle(BRAGI_K65_LIGHTING_HANDLE, indication_data, sizeof(indication_data));
    CloseHandle(BRAGI_K65_LIGHTING_HANDLE);
}
