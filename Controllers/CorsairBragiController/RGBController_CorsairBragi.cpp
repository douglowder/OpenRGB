/*---------------------------------------------------------*\
| RGBController_CorsairBragi.cpp                            |
|                                                           |
|   RGBController for Corsair K65 Plus Wireless keyboard    |
|                                                           |
|   ToastKiste21                                28 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "RGBController_CorsairBragi.h"

using namespace std::chrono_literals;

/**------------------------------------------------------------------*\
    @name Corsair K65 Plus Wireless
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectCorsairBragiControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_CorsairBragi::RGBController_CorsairBragi(CorsairBragiController *controller_ptr)
{
    controller                          = controller_ptr;
    const corsair_v2_device* corsair    = controller->GetDeviceData();

    name                                = controller->GetName();
    vendor                              = "Corsair";
    description                         = "Corsair K65 Plus Wireless Keyboard";
    type                                = corsair->type;
    version                             = controller->GetFirmwareString();
    location                            = controller->GetDeviceLocation();
    serial                              = controller->GetSerialString();

    mode Direct;
    Direct.name                     = "Direct";
    Direct.value                    = 0;
    Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    Direct.brightness_min           = 0;
    Direct.brightness_max           = 100;
    Direct.brightness               = 100;
    modes.push_back(Direct);

    SetupZones();

    controller->SetBrightness(1000);

    /*-----------------------------------------------------*\
    | Bragi requires periodic updates to maintain software    |
    | control mode. Start a keepalive thread.                 |
    \*-----------------------------------------------------*/
    keepalive_thread_run                = true;
    keepalive_thread                    = new std::thread(&RGBController_CorsairBragi::KeepaliveThread, this);
}

RGBController_CorsairBragi::~RGBController_CorsairBragi()
{
    Shutdown();

    /*-----------------------------------------------------*\
    | Close keepalive thread                                |
    \*-----------------------------------------------------*/
    keepalive_thread_run = false;
    keepalive_thread->join();
    delete keepalive_thread;

    delete controller;
}

void RGBController_CorsairBragi::SetupZones()
{
    unsigned int max_led_value              = 0;
    const corsair_v2_device* corsair        = controller->GetDeviceData();
    unsigned int layout                     = controller->GetKeyboardLayout();

    KEYBOARD_LAYOUT new_layout;

    switch(layout)
    {
        case CORSAIR_V2_KB_LAYOUT_ISO:
            new_layout = KEYBOARD_LAYOUT_ISO_QWERTY;
            break;

        case CORSAIR_V2_KB_LAYOUT_JIS:
            new_layout = KEYBOARD_LAYOUT_JIS;
            break;

        case CORSAIR_V2_KB_LAYOUT_ANSI:
        default:
            new_layout = KEYBOARD_LAYOUT_ANSI_QWERTY;
            break;
    }

    /*---------------------------------------------------------*\
    | Fill in zones from the device data                        |
    \*---------------------------------------------------------*/
    for(size_t i = 0; i < CORSAIR_ZONES_MAX; i++)
    {
        if(corsair->zones[i] == NULL)
        {
            break;
        }

        zone new_zone;

        new_zone.name                   = corsair->zones[i]->name;
        new_zone.type                   = corsair->zones[i]->type;

        if(new_zone.type == ZONE_TYPE_MATRIX)
        {
            KeyboardLayoutManager new_kb(new_layout, corsair->layout_new->base_size, corsair->layout_new->key_values);

            if(corsair->layout_new->base_size != KEYBOARD_SIZE_EMPTY)
            {
                keyboard_keymap_overlay_values* temp    = corsair->layout_new;
                new_kb.ChangeKeys(*temp);

                new_zone.matrix_map                     = new_kb.GetKeyMap(KEYBOARD_MAP_FILL_TYPE_COUNT, corsair->zones[i]->rows, corsair->zones[i]->cols);

                new_zone.leds_count                     = new_kb.GetKeyCount();

                LOG_DEBUG("[%s] Created KB matrix with %d rows and %d columns containing %d keys",
                          controller->GetName().c_str(), new_kb.GetRowCount(), new_kb.GetColumnCount(), new_zone.leds_count);

                for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
                {
                    led new_led;

                    new_led.name                = new_kb.GetKeyNameAt(led_idx);
                    new_led.value               = new_kb.GetKeyValueAt(led_idx);
                    max_led_value               = std::max(max_led_value, new_led.value);
                    leds.push_back(new_led);
                }
            }

            max_led_value++;
        }
        else
        {
            new_zone.leds_count             = corsair->zones[i]->rows * corsair->zones[i]->cols;

            for(size_t led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
            {
                led new_led;

                new_led.name                = new_zone.name + " ";
                new_led.name.append(std::to_string(led_idx));
                new_led.value               = (unsigned int)leds.size();

                leds.push_back(new_led);
            }

            max_led_value                   = std::max(max_led_value, (unsigned int)leds.size());
        }

        new_zone.leds_min               = new_zone.leds_count;
        new_zone.leds_max               = new_zone.leds_count;
        zones.push_back(new_zone);
    }

    SetupColors();

    /*---------------------------------------------------------*\
    | Create a buffer map of pointers which contains the        |
    |   layout order of colors the device expects.              |
    \*---------------------------------------------------------*/
    for(size_t led_idx = 0; led_idx < max_led_value; led_idx++)
    {
        buffer_map.push_back(&null_color);
    }

    for(size_t led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        buffer_map[leds[led_idx].value] = &colors[led_idx];
    }
}

void RGBController_CorsairBragi::DeviceUpdateLEDs()
{
    last_update_time = std::chrono::steady_clock::now();

    uint16_t hw_brightness = (uint16_t)(modes[active_mode].brightness * 10);
    controller->SetBrightness(hw_brightness);
    controller->SetLedsDirect(buffer_map);
}

void RGBController_CorsairBragi::DeviceUpdateZoneLEDs(int /*zone*/)
{
    controller->SetLedsDirect(buffer_map);
}

void RGBController_CorsairBragi::DeviceUpdateSingleLED(int /*led*/)
{
    controller->SetLedsDirect(buffer_map);
}

void RGBController_CorsairBragi::DeviceUpdateMode()
{
    uint16_t hw_brightness = (uint16_t)(modes[active_mode].brightness * 10);
    controller->SetBrightness(hw_brightness);
}

void RGBController_CorsairBragi::KeepaliveThread()
{
    while(keepalive_thread_run.load())
    {
        if(active_mode < (int)modes.size())
        {
            if((std::chrono::steady_clock::now() - last_update_time) >
                std::chrono::milliseconds(BRAGI_UPDATE_PERIOD))
            {
                DeviceUpdateLEDs();
            }
        }
        std::this_thread::sleep_for(BRAGI_SLEEP_PERIOD);
    }
}
