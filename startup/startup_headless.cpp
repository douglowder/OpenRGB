/*---------------------------------------------------------*\
| startup_headless.cpp                                      |
|                                                           |
|   Startup path for OpenRGB without a GUI                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdlib>

#include "cli.h"
#include "NetworkServer.h"
#include "ResourceManager.h"
#include "startup.h"

/******************************************************************************************\
*                                                                                          *
*   startup_headless                                                                       *
*                                                                                          *
*       Runs OpenRGB with no user interface.  Background threads are left to run for as    *
*       long as they need, but initialization must at least be allowed to finish.          *
*                                                                                          *
*       Contains no Qt code so that a headless build can use it directly.                  *
*                                                                                          *
\******************************************************************************************/
int startup_headless(unsigned int ret_flags)
{
    ResourceManager::get()->WaitForInitialization();

    if(ret_flags & RET_FLAG_START_SERVER)
    {
        NetworkServer* server = ResourceManager::get()->GetServer();

        if(server)
        {
            return(!server->GetOnline());
        }

        return(EXIT_FAILURE);
    }

    return(EXIT_SUCCESS);
}
