# OpenRGB — notes for agents

A fork of OpenRGB. The work on `doug/headless-library` splits the device layer
out of the Qt application so it can be used as a library and run headless.

## Layout

Three qmake targets, driven by `OpenRGB.pro` as a subdirs project:

| Target | File | Notes |
| --- | --- | --- |
| `libopenrgb` | `libopenrgb.pro` | The core: drivers, device model, managers, SDK server and client. Links no Qt. |
| `OpenRGB_gui` | `OpenRGB_gui.pro` | The Qt application, linking the core. |
| `openrgbd` | `openrgbd.pro` | Headless SDK server. `CONFIG -= qt`. |

Shared settings live in `qmake/*.pri` (`version`, `sources`, `drivers`, `core`,
`dependencies`, `gui`).

**The core must stay a shared library.** Around 1500 device detectors register
through file-scope static objects that nothing references by name. A static
archive would let the linker discard them, silently removing most device
support.

**`CONFIG -= qt` in `openrgbd.pro` is the enforcement mechanism**, not a
comment. If anything in the core reaches for a Qt symbol the daemon fails to
link. `scripts/check-qt-free.sh` reports which files use Qt and checks them
against a GUI allow-list.

`$$files()` and `$$system()` resolve relative to the file that calls them, so
source globs have to stay rooted at an absolute path — see `qmake/sources.pri`.

## Building

```bash
qmake OpenRGB.pro && make -j18       # everything
make sub-openrgbd-pro                # just the daemon (and the core it needs)
./scripts/check-qt-free.sh           # the Qt boundary still holds
otool -L openrgbd | grep -ci qt      # must be 0 on macOS
```

**After changing git state, run `make distclean` first.** `VERSION_STRING` is a
compile-time define built from `git describe`, and `git commit` does not change
file mtimes, so an incremental build happily reports a stale version.

The project root holds ~1074 stale `.o` files and an old
`libopenrgb.0.9.2279.dylib` from the pre-split single-target layout. They are
gitignored, unused, and `make distclean` does not remove them. Ignore them.

## The daemon

`startup/main_daemon.cpp` is the entry point; `startup/startup_headless.cpp` is
shared with the GUI's no-GUI path.

Signals: SIGTERM, SIGINT and SIGHUP request a clean stop; a second signal
terminates immediately because the handler restores the default disposition;
SIGPIPE is ignored. The handler only sets a `volatile sig_atomic_t` — the real
work happens on the main thread, which polls it every 100ms.

A signal arriving during device detection is remembered and acted on once
detection finishes. Interrupting a USB transfer part way is worse than waiting.

`scripts/openrgbd-launchd.sh` installs the daemon as a macOS LaunchAgent under
the label `org.openrgb.openrgbd`. See `Documentation/Daemon.md`.

## SDK protocol traps

Read the encoders in `RGBController.cpp`, **not**
`Documentation/OpenRGBSDK.md`. The document's Device Data and Zone Data tables
both omit the protocol-6 `display_name` that the code writes
(`RGBController.cpp:2607` and `:3217`). A client written from the tables
desynchronises its parse on every device.

- **Colours are `0x00BBGGRR`.** Red is the low byte, not `0xRRGGBB`.
- **Matrix map cells hold a zone-relative LED index.** The global index is
  `map[r * width + c] + zone.start_idx`, and `0xFFFFFFFF` means no key.
- **`zone.start_idx` is never transmitted.** The server derives it as a running
  sum of `leds_count` (`RGBController.cpp:2286`); a client must do the same.
- **Profile writes need local-client status**, granted only when the client
  sets `NET_CLIENT_FLAG_REQUEST_LOCAL_CLIENT` *and* connects over loopback
  (`NetworkServer.cpp:3727`).
- **`SAVE_PROFILE` does not set the active profile.** Only the load path calls
  `SetActiveProfile` (`ProfileManager.cpp:1552`), and there is no packet to set
  it without loading.
- **The profile list is cached.** A file dropped into the profiles directory is
  loadable by name but will not appear in `GET_PROFILE_LIST` until
  `UpdateProfileList()` runs, which happens on save, delete and upload.
- **ACKs can arrive out of order.** Inline ACKs and ACKs from the queued worker
  threads come from different threads, so match on `acked_pkt_id` plus
  `pkt_dev_id`, never arrival order.
- `openrgbd` loads **no plugins**. `PluginManager` is in `qmake/gui.pri`, and
  the loader is `QPluginLoader`.

## Device driver traps

A destructor that joins a worker thread must be able to wake it. The Corsair V2
controllers slept 12.5s uninterruptibly, so a clean shutdown took 11 seconds
until the sleep became a condition-variable wait. The same pattern still exists
in `CorsairK55RGBPROXT`, `CorsairK65Mini` (3s each) and `E131`.

`sample <pid>` is the fastest way to find a stall like that.

## Hardware testing

A Corsair K65 Plus V2 (wired) is attached: one `ZONE_TYPE_MATRIX` zone, 81
LEDs, 6x16 matrix. Profiles live in `~/.config/OpenRGB/profiles/`.

The GUI and the daemon both claim the HID device, so quit one before starting
the other.

The keyboard must be **wired**. Behind the Slipstream dongle the firmware
renders a single colour for the whole board, so per-key work cannot be tested
or saved.

### Photographing the result

`imagesnap -w 2 out.jpg` captures the Mac's camera and is the only way to
confirm what actually reached the hardware, as opposed to what the server says
it holds. Two things must be true first, and **neither is something an agent can
arrange**:

1. `imagesnap` is installed. Check `command -v imagesnap`; it is not a default
   tool and is not guaranteed to be present.
2. The laptop is physically positioned with the keyboard in the camera's view,
   and the keyboard is plugged in and lit.

**Ask the user to confirm that setup before capturing, every time.** Do not
assume an arrangement from earlier in a session, or from a previous session,
still holds — laptops get closed and moved. A photo of an empty desk or a dark
room is worse than no photo at all, because it still looks like evidence and
invites a confident, wrong conclusion.

When a photo is available, verify it properly: decode the profile's `colors[]`
against `leds[]`, then check the lit keys group by group against the image.
Eyeballing "looks about right" has missed real bugs here.

There is also a TypeScript client with a hardware probe in the sibling
`ExpoRGBMenu` checkout: `yarn probe` prints the decoded matrix map as ASCII.
