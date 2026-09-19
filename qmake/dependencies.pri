#-----------------------------------------------------------------------------------------------#
# dependencies.pri                                                                              #
#                                                                                               #
#   Vendored and system dependencies.  These back the core rather than the GUI, with the one    #
#   exception of ColorWheel, which is a Qt widget and lives in gui.pri.                         #
#-----------------------------------------------------------------------------------------------#
OPENRGB_DEP_INCLUDES +=                                                                         \
    dependencies/CRCpp/                                                                         \
    dependencies/hueplusplus-1.2.0/include                                                      \
    dependencies/hueplusplus-1.2.0/include/hueplusplus                                          \
    dependencies/httplib                                                                        \
    dependencies/json/                                                                          \
    dependencies/mdns                                                                           \
    dependencies/stb/                                                                           \

OPENRGB_DEP_HEADERS +=                                                                          \
    dependencies/json/nlohmann/json.hpp                                                         \

OPENRGB_DEP_SOURCES +=                                                                          \
    dependencies/hueplusplus-1.2.0/src/Action.cpp                                               \
    dependencies/hueplusplus-1.2.0/src/APICache.cpp                                             \
    dependencies/hueplusplus-1.2.0/src/BaseDevice.cpp                                           \
    dependencies/hueplusplus-1.2.0/src/BaseHttpHandler.cpp                                      \
    dependencies/hueplusplus-1.2.0/src/Bridge.cpp                                               \
    dependencies/hueplusplus-1.2.0/src/BridgeConfig.cpp                                         \
    dependencies/hueplusplus-1.2.0/src/CLIPSensors.cpp                                          \
    dependencies/hueplusplus-1.2.0/src/ColorUnits.cpp                                           \
    dependencies/hueplusplus-1.2.0/src/EntertainmentMode.cpp                                    \
    dependencies/hueplusplus-1.2.0/src/ExtendedColorHueStrategy.cpp                             \
    dependencies/hueplusplus-1.2.0/src/ExtendedColorTemperatureStrategy.cpp                     \
    dependencies/hueplusplus-1.2.0/src/Group.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/HueCommandAPI.cpp                                        \
    dependencies/hueplusplus-1.2.0/src/HueDeviceTypes.cpp                                       \
    dependencies/hueplusplus-1.2.0/src/HueException.cpp                                         \
    dependencies/hueplusplus-1.2.0/src/Light.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/ModelPictures.cpp                                        \
    dependencies/hueplusplus-1.2.0/src/NewDeviceList.cpp                                        \
    dependencies/hueplusplus-1.2.0/src/Scene.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/Schedule.cpp                                             \
    dependencies/hueplusplus-1.2.0/src/Sensor.cpp                                               \
    dependencies/hueplusplus-1.2.0/src/SimpleBrightnessStrategy.cpp                             \
    dependencies/hueplusplus-1.2.0/src/SimpleColorHueStrategy.cpp                               \
    dependencies/hueplusplus-1.2.0/src/SimpleColorTemperatureStrategy.cpp                       \
    dependencies/hueplusplus-1.2.0/src/StateTransaction.cpp                                     \
    dependencies/hueplusplus-1.2.0/src/TimePattern.cpp                                          \
    dependencies/hueplusplus-1.2.0/src/UPnP.cpp                                                 \
    dependencies/hueplusplus-1.2.0/src/Utils.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/ZLLSensors.cpp                                           \

#-----------------------------------------------------------------------------------------------#
# libe131, in-tree or from the system                                                           #
#-----------------------------------------------------------------------------------------------#
!system_libe131:OPENRGB_DEP_SOURCES  += dependencies/libe131/src/e131.c
!system_libe131:OPENRGB_DEP_INCLUDES += dependencies/libe131/src/

unix {
    system_libe131 {
        CONFIG += link_pkgconfig
        PKGCONFIG += libe131
    }
}

#-----------------------------------------------------------------------------------------------#
# Windows                                                                                       #
#-----------------------------------------------------------------------------------------------#
win32 {
    OPENRGB_DEP_INCLUDES +=                                                                     \
    dependencies/display-library/include                                                        \
    dependencies/hidapi-hotplug-win/include                                                     \
    dependencies/libusb-1.0.27/include                                                          \
    dependencies/mbedtls-3.2.1/include                                                          \
    dependencies/NVFC                                                                           \
    dependencies/PawnIO                                                                         \

    OPENRGB_DEP_HEADERS +=                                                                      \
    dependencies/display-library/include/adl_defines.h                                          \
    dependencies/display-library/include/adl_sdk.h                                              \
    dependencies/display-library/include/adl_structures.h                                       \
    dependencies/NVFC/nvapi.h                                                                   \
    dependencies/PawnIO/PawnIOLib.h                                                             \

    OPENRGB_DEP_SOURCES +=                                                                      \
    dependencies/hueplusplus-1.2.0/src/WinHttpHandler.cpp                                       \
    dependencies/NVFC/nvapi.cpp                                                                 \

    DEFINES += HID_HOTPLUG_ENABLED=1
    HID_HOTPLUG_ENABLED = "true"

    DISTFILES +=                                                                                \
    dependencies/PawnIO/modules/SmbusPIIX4.bin                                                  \
    dependencies/PawnIO/modules/SmbusI801.bin                                                   \
    dependencies/PawnIO/modules/LpcIO.bin                                                       \
}

win32:contains(QMAKE_TARGET.arch, x86_64) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -liphlpapi                                                                              \
        -L"$$PWD/dependencies/libusb-1.0.27/VS2019/MS64/dll" -llibusb-1.0                       \
        -L"$$PWD/dependencies/hidapi-hotplug-win/x64/" -lhidapi-hotplug                         \
        -L"$$PWD/dependencies/mbedtls-3.2.1/lib/x64/" -lmbedcrypto -lmbedtls -lmbedx509         \
        -L"$$PWD/dependencies/PawnIO/" -lPawnIOLib                                              \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -liphlpapi                                                                              \
        -L"$$PWD/dependencies/libusb-1.0.27/VS2019/MS32/dll" -llibusb-1.0                       \
        -L"$$PWD/dependencies/hidapi-hotplug-win/x86/" -lhidapi-hotplug                         \
        -L"$$PWD/dependencies/mbedtls-3.2.1/lib/x86/" -lmbedcrypto -lmbedtls -lmbedx509         \
}

#-----------------------------------------------------------------------------------------------#
# Linux                                                                                         #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, linux) {
    CONFIG += link_pkgconfig

    PKGCONFIG +=                                                                                \
    libusb-1.0

    OPENRGB_DEP_INCLUDES +=                                                                     \
    dependencies/NVFC                                                                           \

    OPENRGB_DEP_HEADERS +=                                                                      \
    dependencies/NVFC/nvapi.h                                                                   \

    OPENRGB_DEP_SOURCES +=                                                                      \
    dependencies/hueplusplus-1.2.0/src/LinHttpHandler.cpp                                       \
    dependencies/NVFC/nvapi.cpp                                                                 \

    #-------------------------------------------------------------------------------------------#
    # mbedtls 3.x packages install to /usr/include/mbedtls3 and /usr/lib/mbedtls3 to            #
    #   avoid conflicting with other mbedtls versions. Prefer the mbedtls3 paths since          #
    #   OpenRGB depends on mbedtls 3.x and will not work with mbedtls 4.x.                      #
    #-------------------------------------------------------------------------------------------#
    exists(/usr/include/mbedtls3) {
        INCLUDEPATH += /usr/include/mbedtls3/
    } else {
        INCLUDEPATH += /usr/include/mbedtls/
    }

    exists(/usr/lib/mbedtls3) {
        LIBS += -L/usr/lib/mbedtls3/
    } else {
        LIBS += -L/usr/lib/mbedtls/
    }

    LIBS +=                                                                                     \
    -lmbedx509                                                                                  \
    -lmbedtls                                                                                   \
    -lmbedcrypto                                                                                \
    -ldl                                                                                        \

    COMPILER_VERSION = $$system($$QMAKE_CXX " -dumpversion")
    if (!versionAtLeast(COMPILER_VERSION, "9")) {
         LIBS += -lstdc++fs
    }

    QMAKE_CXXFLAGS += -Wno-implicit-fallthrough -Wno-psabi

    #-------------------------------------------------------------------------------------------#
    # Determine which hidapi to use based on availability                                       #
    #   Prefer hidraw backend, then libusb                                                      #
    #-------------------------------------------------------------------------------------------#
    packagesExist(hidapi-hotplug-hidraw) {
        PKGCONFIG += hidapi-hotplug-hidraw
        OPENRGB_HIDAPI_PKG = hidapi-hotplug-hidraw
        DEFINES   += HID_HOTPLUG_ENABLED=1
        HID_HOTPLUG_ENABLED = "true"
    } else {
        packagesExist(hidapi-hidraw) {
            PKGCONFIG += hidapi-hidraw
            OPENRGB_HIDAPI_PKG = hidapi-hidraw
        } else {
            packagesExist(hidapi-libusb) {
                PKGCONFIG += hidapi-libusb
                OPENRGB_HIDAPI_PKG = hidapi-libusb
            } else {
                PKGCONFIG += hidapi
                OPENRGB_HIDAPI_PKG = hidapi
            }
        }
    }
}

#-----------------------------------------------------------------------------------------------#
# FreeBSD                                                                                       #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, freebsd) {
    CONFIG += link_pkgconfig

    PKGCONFIG +=                                                                                \
    libusb-1.0

    OPENRGB_DEP_SOURCES +=                                                                      \
    dependencies/hueplusplus-1.2.0/src/LinHttpHandler.cpp                                       \

    LIBS +=                                                                                     \
    -lmbedx509                                                                                  \
    -lmbedtls                                                                                   \
    -lmbedcrypto                                                                                \

    COMPILER_VERSION = $$system($$QMAKE_CXX " -dumpversion")
    if (!versionAtLeast(COMPILER_VERSION, "9")) {
         LIBS += -lstdc++fs
    }

    #-------------------------------------------------------------------------------------------#
    # Determine which hidapi to use based on availability                                       #
    #   Prefer hidraw backend, then libusb                                                      #
    #-------------------------------------------------------------------------------------------#
    packagesExist(hidapi-hidraw) {
        PKGCONFIG += hidapi-hidraw
        OPENRGB_HIDAPI_PKG = hidapi-hidraw
    } else {
        packagesExist(hidapi-libusb) {
            PKGCONFIG += hidapi-libusb
            OPENRGB_HIDAPI_PKG = hidapi-libusb
        } else {
            PKGCONFIG += hidapi
            OPENRGB_HIDAPI_PKG = hidapi
        }
    }
}

#-----------------------------------------------------------------------------------------------#
# macOS                                                                                         #
#-----------------------------------------------------------------------------------------------#
macx {
    CONFIG += link_pkgconfig
    CONFIG += sdk_no_version_check

    PKGCONFIG +=                                                                                \
    libusb-1.0                                                                                  \

    QMAKE_CXXFLAGS +=                                                                           \
    -Wno-narrowing                                                                              \

    OPENRGB_DEP_SOURCES +=                                                                      \
    dependencies/hueplusplus-1.2.0/src/LinHttpHandler.cpp                                       \

    #-------------------------------------------------------------------------------------------#
    # Determine which hidapi to use based on availability                                       #
    #   Prefer hidapi-hotplug if it exists                                                      #
    #-------------------------------------------------------------------------------------------#
    packagesExist(hidapi-hotplug) {
        PKGCONFIG += hidapi-hotplug
        OPENRGB_HIDAPI_PKG = hidapi-hotplug
        DEFINES   += HID_HOTPLUG_ENABLED=1
        HID_HOTPLUG_ENABLED = "true"

    } else {
        PKGCONFIG += hidapi
        OPENRGB_HIDAPI_PKG = hidapi
    }

    #-------------------------------------------------------------------------------------------#
    # Use mbedtls 3.  A package build can pass MBEDTLS_PREFIX on the qmake command line, which  #
    #   arrives before this file is parsed, so the Homebrew lookup is only the default.         #
    #-------------------------------------------------------------------------------------------#
    isEmpty(MBEDTLS_PREFIX) {
        MBEDTLS_PREFIX = $$system(brew --prefix mbedtls@3)
    }

    INCLUDEPATH +=                                                                              \
    $$MBEDTLS_PREFIX/include                                                                    \

    LIBS +=                                                                                     \
    -lmbedx509                                                                                  \
    -lmbedcrypto                                                                                \
    -lmbedtls                                                                                   \
    -L$$MBEDTLS_PREFIX/lib
}

#-----------------------------------------------------------------------------------------------#
# Apple Silicon (arm64) Homebrew installs at /opt/homebrew                                      #
#-----------------------------------------------------------------------------------------------#
macx:contains(QMAKE_HOST.arch, arm64) {
    INCLUDEPATH +=                                                                              \
    /opt/homebrew/include                                                                       \

    LIBS +=                                                                                     \
    -L/opt/homebrew/lib                                                                         \
}

#-----------------------------------------------------------------------------------------------#
# Intel (x86_64) Homebrew installs at /usr/local/lib                                            #
#-----------------------------------------------------------------------------------------------#
macx:contains(QMAKE_HOST.arch, x86_64) {
    OPENRGB_DEP_INCLUDES +=                                                                     \
    dependencies/macUSPCIO                                                                      \

    OPENRGB_DEP_HEADERS +=                                                                      \
    dependencies/macUSPCIO/macUSPCIOAccess.h                                                    \

    INCLUDEPATH +=                                                                              \
    /usr/local/include                                                                          \
    /usr/local/homebrew/include                                                                 \

    LIBS +=                                                                                     \
    -L/usr/local/lib                                                                            \
    -L/usr/local/homebrew/lib                                                                   \

    DEFINES +=                                                                                  \
    _MACOSX_X86_X64                                                                             \
}
