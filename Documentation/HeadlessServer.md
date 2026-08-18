# Headless server and core library

OpenRGB builds as three targets. The device layer lives in a shared library that
links no Qt, so it can be driven by a server with no user interface, or linked
directly by an application built on any toolkit.

| Target | Artifact | Links Qt |
|---|---|---|
| `libopenrgb.pro` | `libopenrgb.<version>.dylib` / `.so` | no |
| `OpenRGB_gui.pro` | `OpenRGB` / `OpenRGB.app` | yes |
| `openrgbd.pro` | `openrgbd` | no |

`OpenRGB.pro` is a `subdirs` project over the three, so a plain `qmake && make`
still builds everything and packaging needs no special handling.

## Building

### Dependencies

macOS, via Homebrew:

```sh
brew install qt@5 hidapi libusb mbedtls@3
```

Linux, Debian and derivatives:

```sh
sudo apt install build-essential qtbase5-dev qttools5-dev-tools \
                 libusb-1.0-0-dev libhidapi-dev libmbedtls-dev pkg-config
```

Qt is only needed for the GUI target. The library and the daemon do not use it,
but `qmake` itself is part of the Qt tooling, so it is required to configure the
build either way.

### Build everything

```sh
qmake OpenRGB.pro
make -j$(nproc)          # macOS: make -j$(sysctl -n hw.ncpu)
```

This produces the library, `OpenRGB.app` (or `openrgb`), and `openrgbd`, all in
the project root.

### Build only the library and the daemon

`qmake` writes one makefile per target, so the GUI can be skipped:

```sh
qmake OpenRGB.pro
make -f Makefile.libopenrgb -j8
make -f Makefile.openrgbd  -j8
```

This still requires `qmake` to configure, but compiles no Qt code.

## Running the daemon

With no arguments, `openrgbd` detects devices and starts the SDK server:

```sh
./openrgbd
```

It has no user interface, so it runs until terminated.

### Options

`openrgbd` shares the OpenRGB command line parser, so `--help` lists the whole
option set including GUI options such as `--gui`, `--startminimized` and
`--i2c-tools`. **Those have no effect here.** A request for a GUI is turned into
a request for a server, which is what makes a bare `openrgbd` useful.

The options that matter:

| Option | Effect |
|---|---|
| `--server-port <1024-65535>` | Server port. Default 6742. |
| `--server-host <host>` | Bind address. **Default `0.0.0.0`, every interface.** |
| `--config <path>` | Use a specific configuration directory instead of the global one. |
| `--localconfig` | Use the working directory for configuration. |
| `--nodetect` | Do not detect hardware at startup. |
| `--noautoconnect` | Do not try to connect to an already running server. |
| `-l`, `--list-devices` | List detected devices, then exit. |
| `-d`, `-z`, `-c`, `-m`, `-b`, `-s` | Apply colours and modes directly, as with the GUI build. |
| `-p`, `-sp` | Load or save a profile. |
| `--loglevel <0-6>`, `-v`, `-vv` | Logging verbosity. |

> **The default bind address is every interface.** The SDK protocol has no
> authentication, so anything that can reach the port can control the lighting.
> Bind loopback explicitly unless remote control is what you want:
>
> ```sh
> ./openrgbd --server-host 127.0.0.1
> ```

### Connecting a client

Any OpenRGB SDK client works. The Qt build can act as one, which is a useful
check that the daemon is behaving:

```sh
./OpenRGB.app/Contents/MacOS/OpenRGB --client 127.0.0.1:6742 --list-devices
```

The protocol is documented in `Documentation/OpenRGBSDK.md`.

## Verifying a build

### Devices are detected

```sh
./openrgbd --list-devices
```

### No detector was lost

Device detectors register themselves through file scope static objects that
nothing references by name. If the library is ever built as a static archive
without whole-archive linking, the linker discards those objects and most device
support disappears with no error at build or run time.

`DetectionManager` writes every registered detector into the settings file
regardless of what hardware is attached, which makes this checkable without any
device:

```sh
mkdir -p /tmp/orgb && ./openrgbd --config /tmp/orgb --list-devices
python3 -c "import json; print(len(json.load(open('/tmp/orgb/OpenRGB.json'))['Detectors']['detectors']))"
```

Compare that count against a known good build. A drop means detectors were
dropped at link time.

### The core really is free of Qt

```sh
otool -L ./openrgbd | grep -i qt      # macOS, expect no output
ldd     ./openrgbd | grep -i qt       # Linux, expect no output
```

`openrgbd.pro` sets `CONFIG -= qt`, so a Qt symbol reaching the core is a link
failure rather than something to notice later. `scripts/check-qt-free.sh` checks
the sources for the same property.

## Using the library directly

An application can link `libopenrgb` instead of talking to a server.

```sh
make -f Makefile.libopenrgb install INSTALL_ROOT=/tmp/staged PREFIX=/usr
```

That installs the library, a `pkg-config` file, the public headers under
`include/openrgb/`, and the implementation headers they include under
`include/openrgb/internal/`.

```cpp
#include <openrgb/OpenRGB.h>

ResourceManager* rm = ResourceManager::get();

rm->Initialize(false, true, false, false);
rm->WaitForInitialization();
rm->WaitForDetection();

for(RGBController* dev : rm->GetRGBControllers())
{
    dev->SetAllLEDs(ToRGBColor(255, 0, 0));
    dev->UpdateLEDs();
}

DetectionManager::get()->Cleanup();
```

```sh
c++ $(pkg-config --cflags --libs openrgb) list_devices.cpp -o list_devices
```

`examples/minimal/` is a complete working version of the above, with a makefile
that builds against either an installed or a staged prefix.

Three things a caller has to know:

* **Devices are owned by the library.** Never delete an `RGBController`.
  `DetectionManager::get()->Cleanup()` destroys them.
* **Callbacks run on detection and network threads**, never on a UI thread. Do
  not block inside one; marshal to your own event loop.
* **`nlohmann::json` appears in the public headers**, pinned to the copy
  installed alongside them. A translation unit that includes OpenRGB headers
  must not also include a different version of `nlohmann/json.hpp`.

## Platform notes

**Windows is not supported yet for the library and daemon.** A shared library on
Windows needs `__declspec(dllexport)` across the exported surface, which has not
been done. The Qt application still builds as before.

**macOS `rpath` points at the build directory.** The library, the application and
the daemon all resolve `libopenrgb` through an rpath set at build time, which is
correct for development but not for a relocatable bundle. Distribution needs
`install_name_tool` or `macdeployqt`.

**macOS mbedtls.** The build asks Homebrew for `mbedtls@3`. If only `mbedtls@2`
is installed, the link still succeeds against `mbedtls@2` but emits a warning
about a missing `mbedtls@3` search path. Install `mbedtls@3` to silence it.

**Linux device permissions.** The daemon needs the same access the application
needs. Install the udev rules, or detection finds fewer devices than expected.
