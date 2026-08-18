/*---------------------------------------------------------*\
| callback_reentrancy.cpp                                   |
|                                                           |
|   Regression test for RGBController update callbacks.     |
|                                                           |
|   SignalUpdate() used to hold UpdateMutex across the      |
|   whole callback loop, so a callback that registered or   |
|   unregistered deadlocked.  The bookkeeping that lets     |
|   Shutdown() wait for callbacks to finish was also        |
|   disabled, so Shutdown() could return while a callback   |
|   was still running on another thread.                    |
|                                                           |
|   Every case below hangs forever against that version.    |
|   The harness fails the test rather than hanging.         |
|                                                           |
|   Build (against a built tree):                           |
|       c++ -std=c++17 tests/callback_reentrancy.cpp \      |
|           -I. -IRGBController $(pkg-config --cflags hidapi) \
|           -L. -lopenrgb -Wl,-rpath,. -o callback_test     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <future>
#include <thread>

#include "RGBController.h"

static int failures = 0;

/*---------------------------------------------------------*\
| Runs work on its own thread and fails the test if it does |
|   not finish in time, instead of hanging the run.         |
\*---------------------------------------------------------*/
static void MustFinish(const char* name, std::function<void()> work)
{
    std::packaged_task<void()> task(work);
    std::future<void>          done = task.get_future();
    std::thread                runner(std::move(task));

    if(done.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
        printf("FAIL  %s: deadlocked\n", name);
        failures++;
        runner.detach();
        return;
    }

    runner.join();
    printf("ok    %s\n", name);
}

/*---------------------------------------------------------*\
| A controller that talks to nothing.                        |
|                                                            |
|   RGBController_Dummy is not usable here: it calls         |
|   Shutdown() from its own constructor to stop the device   |
|   thread, which also stops it signalling.                  |
\*---------------------------------------------------------*/
class TestController : public RGBController
{
public:
    TestController()
    {
        name        = "Test Controller";
        vendor      = "OpenRGB";
        description = "Callback re-entrancy test fixture";
    }

    void DeviceUpdateLEDs()             override {}
    void DeviceUpdateZoneLEDs(int)      override {}
    void DeviceUpdateSingleLED(int)     override {}
    void DeviceUpdateMode()             override {}
};

static TestController* MakeController()
{
    return new TestController();
}

/*---------------------------------------------------------*\
| A callback that unregisters itself while it is running    |
\*---------------------------------------------------------*/
static TestController* unreg_dev  = nullptr;
static std::atomic<int>     unreg_hits{0};

static void UnregisterSelf(void* arg, unsigned int, void*)
{
    unreg_hits++;
    unreg_dev->UnregisterUpdateCallback(arg);
}

/*---------------------------------------------------------*\
| A callback that registers another callback while running  |
\*---------------------------------------------------------*/
static TestController* reg_dev = nullptr;
static std::atomic<int>     reg_hits{0};

static void Noop(void*, unsigned int, void*) {}

static void RegisterFromCallback(void*, unsigned int, void*)
{
    reg_hits++;
    reg_dev->RegisterUpdateCallback(Noop, (void*)0x1234);
}

/*---------------------------------------------------------*\
| A callback that clears the whole list while running       |
\*---------------------------------------------------------*/
static TestController* clear_dev = nullptr;

static void ClearFromCallback(void*, unsigned int, void*)
{
    clear_dev->ClearCallbacks();
}

int main()
{
    /*-----------------------------------------------------*\
    | Unregistering from inside a callback                  |
    \*-----------------------------------------------------*/
    MustFinish("unregister from inside a callback", []
    {
        unreg_dev = MakeController();
        unreg_dev->RegisterUpdateCallback(UnregisterSelf, (void*)0x1);
        unreg_dev->SignalUpdate(0);
        unreg_dev->SignalUpdate(0);
        unreg_dev->Shutdown();
        delete unreg_dev;
    });

    if(unreg_hits != 1)
    {
        printf("FAIL  callback ran %d times, expected 1 (it unregistered itself)\n", (int)unreg_hits);
        failures++;
    }

    /*-----------------------------------------------------*\
    | Registering from inside a callback                    |
    \*-----------------------------------------------------*/
    MustFinish("register from inside a callback", []
    {
        reg_dev = MakeController();
        reg_dev->RegisterUpdateCallback(RegisterFromCallback, (void*)0x2);
        reg_dev->SignalUpdate(0);
        reg_dev->Shutdown();
        delete reg_dev;
    });

    /*-----------------------------------------------------*\
    | Clearing the list from inside a callback              |
    \*-----------------------------------------------------*/
    MustFinish("clear callbacks from inside a callback", []
    {
        clear_dev = MakeController();
        clear_dev->RegisterUpdateCallback(ClearFromCallback, (void*)0x3);
        clear_dev->SignalUpdate(0);
        clear_dev->Shutdown();
        delete clear_dev;
    });

    /*-----------------------------------------------------*\
    | Unregistering from another thread while signalling.   |
    |   Once UnregisterUpdateCallback returns, the callback |
    |   must not run again, which is what makes it safe to  |
    |   free the object the arg points at.                  |
    \*-----------------------------------------------------*/
    MustFinish("unregister from another thread", []
    {
        TestController* dev = MakeController();

        static std::atomic<bool> unregistered{false};
        static std::atomic<bool> ran_after{false};

        dev->RegisterUpdateCallback([](void*, unsigned int, void*)
        {
            if(unregistered)
            {
                ran_after = true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }, (void*)0x4);

        std::atomic<bool> stop{false};
        std::thread signaller([&]
        {
            while(!stop)
            {
                dev->SignalUpdate(0);
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        dev->UnregisterUpdateCallback((void*)0x4);
        unregistered = true;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        stop = true;
        signaller.join();

        if(ran_after)
        {
            printf("FAIL  callback ran after unregister returned\n");
            failures++;
        }

        dev->Shutdown();
        delete dev;
    });

    /*-----------------------------------------------------*\
    | Shutdown must not return while a callback is running  |
    \*-----------------------------------------------------*/
    MustFinish("shutdown waits for a running callback", []
    {
        TestController* dev = MakeController();

        static std::atomic<bool> in_callback{false};
        static std::atomic<bool> callback_done{false};

        dev->RegisterUpdateCallback([](void*, unsigned int, void*)
        {
            in_callback = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            callback_done = true;
        }, (void*)0x5);

        std::thread signaller([&]{ dev->SignalUpdate(0); });

        while(!in_callback)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        dev->Shutdown();

        if(!callback_done)
        {
            printf("FAIL  Shutdown() returned while a callback was still running\n");
            failures++;
        }

        signaller.join();
        delete dev;
    });

    if(failures == 0)
    {
        printf("\nall callback re-entrancy tests passed\n");
        return 0;
    }

    printf("\n%d failure(s)\n", failures);
    return 1;
}
