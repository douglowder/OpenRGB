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
#include <csignal>
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
| How often the main thread checks whether it should stop.  |
| A service manager sends SIGTERM and then waits, so this   |
| is the delay it sees before shutdown begins.              |
\*---------------------------------------------------------*/
static const std::chrono::milliseconds SHUTDOWN_POLL_INTERVAL = 100ms;

/*---------------------------------------------------------*\
| Set by the signal handler, read by the main thread.  A    |
| handler may only write a volatile sig_atomic_t, so the    |
| signal number leaves through one of those instead of the  |
| handler doing the work of shutting down itself.           |
\*---------------------------------------------------------*/
static volatile std::sig_atomic_t shutdown_signal = 0;

/*---------------------------------------------------------*\
| DaemonSignalHandler                                       |
|                                                           |
|   Records that shutdown was requested.  Nothing else      |
|   happens here: closing sockets and running device        |
|   destructors takes mutexes and writes log messages, and  |
|   none of that is safe inside a signal handler.           |
\*---------------------------------------------------------*/
static void DaemonSignalHandler(int signal_number)
{
    /*-----------------------------------------------------*\
    | Restore the default disposition so a second signal    |
    | terminates on the spot.  Device detection holds the   |
    | main thread for several seconds, and a daemon stuck   |
    | there still has to be killable.                       |
    \*-----------------------------------------------------*/
    std::signal(signal_number, SIG_DFL);

    shutdown_signal = signal_number;
}

/*---------------------------------------------------------*\
| InstallSignalHandlers                                     |
|                                                           |
|   Takes over the signals a service manager or a terminal  |
|   uses to stop a process.  Their default disposition      |
|   terminates the daemon immediately, which skips the      |
|   shutdown path below: clients are dropped mid packet and |
|   device destructors never run, leaving hardware lit by   |
|   whatever the last write set.                            |
\*---------------------------------------------------------*/
static void InstallSignalHandlers()
{
    std::signal(SIGINT,  DaemonSignalHandler);
    std::signal(SIGTERM, DaemonSignalHandler);

#ifndef _WIN32
    /*-----------------------------------------------------*\
    | SIGHUP arrives when the controlling terminal goes     |
    | away.  Treat closing the terminal as a stop request   |
    | rather than letting it kill the process outright.     |
    \*-----------------------------------------------------*/
    std::signal(SIGHUP,  DaemonSignalHandler);

    /*-----------------------------------------------------*\
    | A client that disappears mid send must not be able to |
    | take the daemon down with it.  Every send() in the    |
    | server passes MSG_NOSIGNAL, so this covers the writes |
    | that do not, and any that a plugin makes.             |
    \*-----------------------------------------------------*/
    std::signal(SIGPIPE, SIG_IGN);
#endif
}

/*---------------------------------------------------------*\
| WaitForShutdown                                           |
|                                                           |
|   Returns once the server has gone offline or a signal    |
|   has asked the daemon to stop                            |
\*---------------------------------------------------------*/
static void WaitForShutdown(NetworkServer* srv)
{
    while(shutdown_signal == 0 && srv->GetOnline())
    {
        std::this_thread::sleep_for(SHUTDOWN_POLL_INTERVAL);
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
    | Install the handlers first.  A signal that arrives    |
    | during detection is then remembered instead of        |
    | killing the process, and acted on below.              |
    \*-----------------------------------------------------*/
    InstallSignalHandlers();

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

    NetworkServer* server = nullptr;

    if(ret_flags & RET_FLAG_START_SERVER)
    {
        server = ResourceManager::get()->GetServer();
    }

    /*-----------------------------------------------------*\
    | Run until the server shuts down or a signal arrives   |
    \*-----------------------------------------------------*/
    if(server)
    {
        WaitForShutdown(server);
    }

    if(shutdown_signal != 0)
    {
        LOG_INFO("[openrgbd] Signal %d received, shutting down", (int)shutdown_signal);
    }

    /*-----------------------------------------------------*\
    | Take the server down before the devices it serves, so |
    | no request can arrive for a controller that is being  |
    | destroyed.                                            |
    \*-----------------------------------------------------*/
    if(server)
    {
        server->StopServer();
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
