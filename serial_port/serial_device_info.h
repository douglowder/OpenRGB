/*---------------------------------------------------------*\
| serial_device_info.h                                      |
|                                                           |
|   Description of a USB serial device                       |
|                                                           |
|   Split out of find_usb_serial_port.h so that a caller     |
|   who only needs the description does not also get the     |
|   lookup functions in the global namespace.                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>

struct SerialDeviceInfo
{
    unsigned short  vendor_id;
    unsigned short  product_id;
    std::string     port_path;
    std::string     usb_path;
};
