# openrgbd

`openrgbd` is the OpenRGB SDK server with no user interface. It links `libopenrgb` and no Qt, so it runs on a machine with no display and no Qt installed.

```
openrgbd --server                     # serve on the default port, 6742
openrgbd --server --profile expo      # apply a profile, then serve
openrgbd --help                       # the same options the application accepts
```

Given no arguments it starts a server, because there is no GUI to fall back to.

## Signals

| Signal | Behaviour |
| --- | --- |
| `SIGTERM` | Stop cleanly. This is what `launchctl`, `systemd` and a plain `kill` send. |
| `SIGINT` | Stop cleanly. This is Ctrl-C in a terminal. |
| `SIGHUP` | Stop cleanly. Arrives when the controlling terminal goes away. |
| a second signal of any of the above | Terminate immediately. |
| `SIGPIPE` | Ignored, so a client that disappears mid-send cannot take the daemon down. |

A clean stop closes the listening sockets, disconnects clients, and runs the device destructors, which release each device's USB or HID handle. It then exits zero.

Two details are worth knowing:

- A stop request that arrives while device detection is running is remembered, not acted on immediately. Detection finishes first, because interrupting a USB transfer part-way is worse than waiting for it. On a machine with many I2C or SMBus devices that wait can run to several seconds.
- Because the first signal restores the default disposition, a second one terminates the process outright. That is the escape hatch when the wait above is not converging.

## Running under launchd on macOS

`scripts/openrgbd-launchd.sh` installs `openrgbd` as a LaunchAgent, so it starts at login and restarts if it fails.

```
scripts/openrgbd-launchd.sh install                     # serve with no profile
scripts/openrgbd-launchd.sh install --profile expo      # apply a profile at startup
scripts/openrgbd-launchd.sh install --binary /usr/bin/openrgbd
scripts/openrgbd-launchd.sh stop
scripts/openrgbd-launchd.sh restart
scripts/openrgbd-launchd.sh uninstall
scripts/openrgbd-launchd.sh status
```

Anything after `--` is passed through to `openrgbd`.

`stop` sends SIGTERM and waits for the process to go. The agent stays loaded, so `restart` brings it back, as does the next login; `uninstall` is the way to stop it for good. It stays down in the meantime because a clean stop exits zero and `KeepAlive` is set to `SuccessfulExit=false`. Stopping an already-stopped agent is not an error.

Without `--binary`, the script uses `openrgbd` from the build tree, then `openrgbd` on `PATH`. A build tree binary finds `libopenrgb` through an rpath into that tree, so moving or deleting the tree breaks the agent; `make install` produces a binary that does not depend on it.

| | |
| --- | --- |
| Agent | `~/Library/LaunchAgents/org.openrgb.openrgbd.plist`, generated from `mac/org.openrgb.openrgbd.plist.in` |
| launchd's stdio | `~/Library/Logs/OpenRGB/openrgbd.launchd.log` |
| OpenRGB's own log | `~/.config/OpenRGB/logs/` |

An agent rather than a system daemon, for two reasons: `openrgbd` keeps its configuration and profiles under the user's home directory, and reaching USB HID devices needs a logged-in user rather than root.

`install` over a running agent replaces it. `launchctl bootout` returns before the service is really gone, and a `bootstrap` that races the teardown fails with `Input/output error`, so the script waits for the old service to disappear first.

The agent restarts only after a failure: `KeepAlive` is set to `SuccessfulExit=false`, and a clean stop exits zero, so `stop`, `uninstall` and `launchctl bootout` all stay stopped.

## Running under systemd on Linux

`qt/openrgb.service` runs the application binary with `--server`. `openrgbd` is a drop-in replacement for it in `ExecStart` and needs no display. `Restart=` and `systemctl stop` both work, for the same reason `launchctl` does: SIGTERM now stops the process cleanly instead of killing it.
