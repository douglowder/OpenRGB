/*---------------------------------------------------------*\
| main_daemon.cpp                                           |
|                                                           |
|   Entry point for the OpenRGB daemon, a build with no      |
|   user interface and no Qt                                 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <thread>

#include "cli.h"
#include "DetectionManager.h"
#include "ResourceManager.h"
#include "NetworkServer.h"
#include "LogManager.h"
#include "startup.h"

#ifdef _MACOSX_X86_X64
#include "macUSPCIOAccess.h"
io_connect_t macUSPCIO_driver_connection;
#endif

using namespace std::chrono_literals;

/*---------------------------------------------------------*\
| WaitWhileServerOnline                                     |
|                                                           |
|   Wait while NetworkServer is online and return only when |
|   it has shut down                                        |
\*---------------------------------------------------------*/
static void WaitWhileServerOnline(NetworkServer* srv)
{
    while(srv->GetOnline())
    {
        std::this_thread::sleep_for(1s);
    }
}

/*---------------------------------------------------------*\
| main                                                      |
|                                                           |
|   Entry point for the daemon.  Mirrors the application    |
|   entry point, minus everything that needs a GUI.         |
\*---------------------------------------------------------*/
int main(int argc, char* argv[])
{
    /*-----------------------------------------------------*\
    | Mac x86/x64 only - Install SMBus Driver macUSPCIO     |
    \*-----------------------------------------------------*/
#ifdef _MACOSX_X86_X64
    InitMacUSPCIODriver();
#endif

    /*-----------------------------------------------------*\
    | Perform CLI pre-detection processing to get return    |
    | flags                                                 |
    \*-----------------------------------------------------*/
    unsigned int ret_flags = cli_pre_detection(argc, argv);

    /*-----------------------------------------------------*\
    | There is no GUI to start.  The command line parser    |
    | asks for one when it was given nothing else to do, so |
    | run a server instead.                                 |
    |                                                       |
    | This has to happen before Initialize(), which decides |
    | there whether to bring the server up.                 |
    \*-----------------------------------------------------*/
    if(ret_flags & RET_FLAG_START_GUI)
    {
        ret_flags &= ~(RET_FLAG_START_GUI | RET_FLAG_START_MINIMIZED | RET_FLAG_I2C_TOOLS);
        ret_flags |= RET_FLAG_START_SERVER;
    }

    /*-----------------------------------------------------*\
    | Initialize ResourceManager                            |
    \*-----------------------------------------------------*/
    ResourceManager::get()->Initialize(
        !(ret_flags & RET_FLAG_NO_AUTO_CONNECT),
        !(ret_flags & RET_FLAG_NO_DETECT),
        ret_flags & RET_FLAG_START_SERVER,
        ret_flags & RET_FLAG_CLI_POST_DETECTION);

    /*-----------------------------------------------------*\
    | Wait for initialization and report whether the server |
    | came up                                               |
    \*-----------------------------------------------------*/
    int exitval = startup_headless(ret_flags);

    /*-----------------------------------------------------*\
    | Run until the server shuts down                       |
    \*-----------------------------------------------------*/
    if(ret_flags & RET_FLAG_START_SERVER)
    {
        NetworkServer* server = ResourceManager::get()->GetServer();

        if(server)
        {
            WaitWhileServerOnline(server);
        }
    }

    /*-----------------------------------------------------*\
    | Clean up detected devices so destructors can run.     |
    \*-----------------------------------------------------*/
    DetectionManager::get()->Cleanup();

    LOG_TRACE("OpenRGB daemon finishing with exit code %d", exitval);

    /*-----------------------------------------------------*\
    | Mac x86/x64 only - Uninstall SMBus Driver macUSPCIO   |
    \*-----------------------------------------------------*/
#ifdef _MACOSX_X86_X64
    CloseMacUSPCIODriver();
#endif

    return exitval;
}
