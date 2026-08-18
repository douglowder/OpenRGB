/*---------------------------------------------------------*\
| startup.h                                                 |
|                                                           |
|   Startup for the OpenRGB application                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

/*---------------------------------------------------------*\
| Opens the main window, or falls through to the headless    |
|   path below when no GUI was requested.  Requires Qt.      |
\*---------------------------------------------------------*/
int startup(int argc, char* argv[], unsigned int ret_flags);

/*---------------------------------------------------------*\
| Waits for initialization and reports whether the server    |
|   came online.  Contains no GUI code, so a headless build  |
|   can use it without linking Qt.                           |
\*---------------------------------------------------------*/
int startup_headless(unsigned int ret_flags);
