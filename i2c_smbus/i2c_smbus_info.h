/*---------------------------------------------------------*\
| i2c_smbus_info.h                                          |
|                                                           |
|   Description of an SMBus interface                        |
|                                                           |
|   Split out of i2c_smbus.h so that a caller who only      |
|   needs to describe a bus does not also pull in the        |
|   transfer API, its u8/u16/u32/s32 typedefs, and the       |
|   platform headers behind them.                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

typedef struct
{
    char device_name[512];

    int port_id;
    int pci_device;
    int pci_vendor;
    int pci_subsystem_device;
    int pci_subsystem_vendor;

    int bus_id;
} i2c_smbus_info;
