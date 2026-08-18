#-----------------------------------------------------------------------------------------------#
# core.pri                                                                                      #
#                                                                                               #
#   The headless core: managers, the device model, and hardware access.  Everything here must   #
#   stay free of Qt so that it can be built as a library and linked by apps using other         #
#   toolkits.  scripts/check-qt-free.sh enforces that.                                          #
#-----------------------------------------------------------------------------------------------#
OPENRGB_CORE_INCLUDES +=                                                                        \
    dmiinfo/                                                                                    \
    hidapi_wrapper/                                                                             \
    i2c_smbus/                                                                                  \
    i2c_tools/                                                                                  \
    interop/                                                                                    \
    net_port/                                                                                   \
    pci_ids/                                                                                    \
    scsiapi/                                                                                    \
    serial_port/                                                                                \
    super_io/                                                                                   \
    AutoStart/                                                                                  \
    KeyboardLayoutManager/                                                                      \
    RGBController/                                                                              \
    SPDAccessor/                                                                                \

OPENRGB_CORE_HEADERS +=                                                                         \
    Colors.h                                                                                    \
    JsonUtils.h                                                                                 \
    LogManager.h                                                                                \
    NetworkClient.h                                                                             \
    NetworkProtocol.h                                                                           \
    NetworkServer.h                                                                             \
    OpenRGBPluginAPIInterface.h                                                                 \
    ProfileManager.h                                                                            \
    ResourceManager.h                                                                           \
    ResourceManagerCallback.h                                                                   \
    SettingsManager.h                                                                           \
    DetectionManager.h                                                                          \
    dmiinfo/dmiinfo.h                                                                           \
    filesystem.h                                                                                \
    hidapi_wrapper/hidapi_wrapper.h                                                             \
    i2c_smbus/i2c_smbus.h                                                                       \
    i2c_tools/i2c_tools.h                                                                       \
    interop/DeviceGuard.h                                                                       \
    interop/DeviceGuardLock.h                                                                   \
    interop/DeviceGuardManager.h                                                                \
    net_port/net_port.h                                                                         \
    pci_ids/pci_ids.h                                                                           \
    scsiapi/scsiapi.h                                                                           \
    serial_port/find_usb_serial_port.h                                                          \
    serial_port/serial_port.h                                                                   \
    super_io/super_io.h                                                                         \
    MathUtils.h                                                                                 \
    StringUtils.h                                                                               \
    AutoStart/AutoStart.h                                                                       \
    KeyboardLayoutManager/KeyboardLayoutManager.h                                               \
    RGBController/hsv.h                                                                         \
    RGBController/RGBController.h                                                               \
    RGBController/RGBController_Dummy.h                                                         \
    RGBController/RGBControllerKeyNames.h                                                       \
    RGBController/RGBController_Network.h                                                       \
    startup/startup.h                                                                           \

OPENRGB_CORE_SOURCES +=                                                                         \
    cli.cpp                                                                                     \
    DetectionManager.cpp                                                                        \
    dmiinfo/dmiinfo.cpp                                                                         \
    JsonUtils.cpp                                                                               \
    LogManager.cpp                                                                              \
    NetworkClient.cpp                                                                           \
    NetworkProtocol.cpp                                                                         \
    NetworkServer.cpp                                                                           \
    OpenRGBPluginAPI.cpp                                                                        \
    ProfileManager.cpp                                                                          \
    ResourceManager.cpp                                                                         \
    SettingsManager.cpp                                                                         \
    SPDAccessor/DDR4DirectAccessor.cpp                                                          \
    SPDAccessor/DDR5DirectAccessor.cpp                                                          \
    SPDAccessor/SPDAccessor.cpp                                                                 \
    SPDAccessor/SPDDetector.cpp                                                                 \
    SPDAccessor/SPDWrapper.cpp                                                                  \
    i2c_smbus/i2c_smbus.cpp                                                                     \
    i2c_tools/i2c_tools.cpp                                                                     \
    interop/DeviceGuard.cpp                                                                     \
    interop/DeviceGuardLock.cpp                                                                 \
    interop/DeviceGuardManager.cpp                                                              \
    net_port/net_port.cpp                                                                       \
    serial_port/serial_port.cpp                                                                 \
    MathUtils.cpp                                                                               \
    StringUtils.cpp                                                                             \
    AutoStart/AutoStart.cpp                                                                     \
    KeyboardLayoutManager/KeyboardLayoutManager.cpp                                             \
    RGBController/hsv.cpp                                                                       \
    RGBController/RGBController.cpp                                                             \
    RGBController/RGBController_Dummy.cpp                                                       \
    RGBController/RGBControllerKeyNames.cpp                                                     \
    RGBController/RGBController_Network.cpp                                                     \
    RGBController/RGBController_Virtual.cpp                                                     \
    startup/startup_headless.cpp                                                                \

#-----------------------------------------------------------------------------------------------#
# Windows                                                                                       #
#-----------------------------------------------------------------------------------------------#
win32 {
    OPENRGB_CORE_INCLUDES +=                                                                    \
    i2c_smbus/Windows                                                                           \
    wmi/                                                                                        \

    OPENRGB_CORE_HEADERS +=                                                                     \
    i2c_smbus/Windows/i2c_smbus_amdadl.h                                                        \
    i2c_smbus/Windows/i2c_smbus_nvapi.h                                                         \
    i2c_smbus/Windows/i2c_smbus_pawnio.h                                                        \
    wmi/wmi.h                                                                                   \
    AutoStart/AutoStart-Windows.h                                                               \

    OPENRGB_CORE_SOURCES +=                                                                     \
    i2c_smbus/Windows/i2c_smbus_amdadl.cpp                                                      \
    i2c_smbus/Windows/i2c_smbus_nvapi.cpp                                                       \
    scsiapi/scsiapi_windows.c                                                                   \
    serial_port/find_usb_serial_port_win.cpp                                                    \
    wmi/wmi.cpp                                                                                 \
    AutoStart/AutoStart-Windows.cpp                                                             \
}

win32:contains(QMAKE_TARGET.arch, x86_64) {
    OPENRGB_CORE_SOURCES +=                                                                     \
    i2c_smbus/Windows/i2c_smbus_pawnio.cpp                                                      \
    super_io/super_io_pawnio.cpp                                                                \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    OPENRGB_CORE_SOURCES +=                                                                     \
    super_io/super_io.cpp                                                                       \
}

#-----------------------------------------------------------------------------------------------#
# Linux                                                                                         #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, linux) {
    OPENRGB_CORE_INCLUDES +=                                                                    \
    i2c_smbus/Linux                                                                             \

    OPENRGB_CORE_HEADERS +=                                                                     \
    i2c_smbus/Linux/i2c_smbus_linux.h                                                           \
    AutoStart/AutoStart-Linux.h                                                                 \
    SPDAccessor/EE1004Accessor_Linux.h                                                          \
    SPDAccessor/SPD5118Accessor_Linux.h                                                         \
    super_io/super_io.h                                                                         \

    OPENRGB_CORE_SOURCES +=                                                                     \
    i2c_smbus/Linux/i2c_smbus_linux.cpp                                                         \
    scsiapi/scsiapi_linux.c                                                                     \
    serial_port/find_usb_serial_port_linux.cpp                                                  \
    AutoStart/AutoStart-Linux.cpp                                                               \
    SPDAccessor/EE1004Accessor_Linux.cpp                                                        \
    SPDAccessor/SPD5118Accessor_Linux.cpp                                                       \
    super_io/super_io.cpp                                                                       \
}

#-----------------------------------------------------------------------------------------------#
# FreeBSD                                                                                       #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, freebsd) {
    OPENRGB_CORE_HEADERS +=                                                                     \
    AutoStart/AutoStart-FreeBSD.h                                                               \
    super_io/super_io.h                                                                         \

    OPENRGB_CORE_SOURCES +=                                                                     \
    serial_port/find_usb_serial_port_linux.cpp                                                  \
    AutoStart/AutoStart-FreeBSD.cpp                                                             \
    super_io/super_io.cpp                                                                       \
}

#-----------------------------------------------------------------------------------------------#
# macOS                                                                                         #
#-----------------------------------------------------------------------------------------------#
macx {
    OPENRGB_CORE_HEADERS +=                                                                     \
    AutoStart/AutoStart-MacOS.h                                                                 \

    OPENRGB_CORE_SOURCES +=                                                                     \
    serial_port/find_usb_serial_port_macos.cpp                                                  \
    AutoStart/AutoStart-MacOS.cpp                                                               \
}

macx:contains(QMAKE_HOST.arch, arm64) {
    OPENRGB_CORE_HEADERS +=                                                                     \
    super_io/super_io.h                                                                         \

    OPENRGB_CORE_SOURCES +=                                                                     \
    scsiapi/scsiapi_macos.c                                                                     \
    super_io/super_io.cpp                                                                       \
}

macx:contains(QMAKE_HOST.arch, x86_64) {
    OPENRGB_CORE_INCLUDES +=                                                                    \
    i2c_smbus/MacOS                                                                             \

    OPENRGB_CORE_HEADERS +=                                                                     \
    i2c_smbus/MacOS/i2c_smbus_i801.h                                                            \
    i2c_smbus/MacOS/i2c_smbus_nct6775.h                                                         \
    i2c_smbus/MacOS/i2c_smbus_piix4.h                                                           \
    super_io/super_io.h                                                                         \

    OPENRGB_CORE_SOURCES +=                                                                     \
    i2c_smbus/MacOS/i2c_smbus_i801.cpp                                                          \
    i2c_smbus/MacOS/i2c_smbus_nct6775.cpp                                                       \
    i2c_smbus/MacOS/i2c_smbus_piix4.cpp                                                         \
    scsiapi/scsiapi_macos.c                                                                     \
    super_io/super_io.cpp                                                                       \
}
