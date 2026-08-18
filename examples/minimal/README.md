# Minimal OpenRGB consumer

A console program that uses the OpenRGB core library with no Qt and no
toolkit of any kind. It initialises the library, waits for detection and
prints the devices it found.

## Build

Against an installed OpenRGB:

    make

Against a staged install:

    make PREFIX=/tmp/openrgb-prefix/usr

## Notes

* Devices are owned by the library. Never delete an `RGBController`;
  `DetectionManager::get()->Cleanup()` destroys them.
* Callbacks run on detection and network threads, not on any UI thread.
  Marshal to your own event loop and do not block inside a callback.
* The public headers expose `nlohmann::json`, pinned to the copy installed
  alongside them. A translation unit that includes OpenRGB headers must not
  also include a different `nlohmann/json.hpp`.
* Device access needs the same permissions the OpenRGB application needs.
  On Linux that means the udev rules; without them detection finds less.
