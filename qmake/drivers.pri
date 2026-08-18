#-----------------------------------------------------------------------------------------------#
# drivers.pri                                                                                   #
#                                                                                               #
#   Device drivers under Controllers/.  Accumulates into OPENRGB_DRIVER_* so that a build       #
#   without a GUI can pick up the same set.                                                     #
#                                                                                               #
#   The CONTROLLER_* globs are evaluated in OpenRGB.pro, because $$files() resolves relative    #
#   to the file that calls it.                                                                  #
#-----------------------------------------------------------------------------------------------#
OPENRGB_DRIVER_INCLUDES += $$CONTROLLER_INCLUDES
OPENRGB_DRIVER_HEADERS  += $$CONTROLLER_H
OPENRGB_DRIVER_SOURCES  += $$CONTROLLER_CPP

#-----------------------------------------------------------------------------------------------#
# Platform-specific drivers                                                                     #
#-----------------------------------------------------------------------------------------------#
win32 {
    OPENRGB_DRIVER_HEADERS += $$CONTROLLER_H_WINDOWS
    OPENRGB_DRIVER_SOURCES += $$CONTROLLER_CPP_WINDOWS
}

contains(QMAKE_PLATFORM, linux) {
    OPENRGB_DRIVER_HEADERS += $$CONTROLLER_H_LINUX
    OPENRGB_DRIVER_SOURCES += $$CONTROLLER_CPP_LINUX
}

contains(QMAKE_PLATFORM, freebsd) {
    OPENRGB_DRIVER_HEADERS += $$CONTROLLER_H_FREEBSD
    OPENRGB_DRIVER_SOURCES += $$CONTROLLER_CPP_FREEBSD

    #-------------------------------------------------------------------------------------------#
    # Drivers that do not build on FreeBSD                                                      #
    #-------------------------------------------------------------------------------------------#
    OPENRGB_DRIVER_HEADERS -=                                                                   \
    Controllers/SeagateController/RGBController_Seagate.h                                       \
    Controllers/SeagateController/SeagateController.h                                           \
    Controllers/ENESMBusController/ENESMBusInterface/ENESMBusInterface_ROGArion.h               \
    $$CONTROLLER_H_WINDOWS                                                                      \

    OPENRGB_DRIVER_SOURCES -=                                                                   \
    Controllers/SeagateController/RGBController_Seagate.cpp                                     \
    Controllers/SeagateController/SeagateController.cpp                                         \
    Controllers/SeagateController/SeagateControllerDetect.cpp                                   \
    Controllers/ENESMBusController/ROGArionDetect.cpp                                           \
    Controllers/ENESMBusController/ENESMBusInterface/ENESMBusInterface_ROGArion.cpp             \
}

macx {
    OPENRGB_DRIVER_HEADERS += $$CONTROLLER_H_MACOS
    OPENRGB_DRIVER_SOURCES += $$CONTROLLER_CPP_MACOS
}
