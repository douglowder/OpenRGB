#-----------------------------------------------------------------------------------------------#
# openrgbd.pro                                                                                  #
#                                                                                               #
#   The OpenRGB daemon: a server with no user interface and no Qt.                              #
#                                                                                               #
#   CONFIG -= qt is the point of this target.  If any object in libopenrgb reached for a Qt     #
#   symbol, this would fail to link, so the daemon is a standing proof that the core really is  #
#   independent of the GUI.                                                                     #
#-----------------------------------------------------------------------------------------------#
TEMPLATE    = app
TARGET      = openrgbd

CONFIG     += c++17 console
CONFIG     -= qt app_bundle
QT         -= core gui widgets

include(qmake/version.pri)
include(qmake/sources.pri)

HID_HOTPLUG_ENABLED = "false"

#-----------------------------------------------------------------------------------------------#
# Included for include paths and platform configuration.  No core or driver sources are         #
#   compiled here; they arrive through libopenrgb.                                              #
#-----------------------------------------------------------------------------------------------#
include(qmake/drivers.pri)
include(qmake/core.pri)
include(qmake/dependencies.pri)

INCLUDEPATH +=                                                                                  \
    $$OPENRGB_DRIVER_INCLUDES                                                                   \
    $$OPENRGB_CORE_INCLUDES                                                                     \
    $$OPENRGB_DEP_INCLUDES                                                                      \

SOURCES +=                                                                                      \
    startup/main_daemon.cpp                                                                     \

#-----------------------------------------------------------------------------------------------#
# Link the core                                                                                 #
#-----------------------------------------------------------------------------------------------#
LIBS           += -L$$OPENRGB_ROOT -lopenrgb
unix:QMAKE_RPATHDIR += $$OPENRGB_ROOT

DESTDIR         = $$OPENRGB_ROOT
OBJECTS_DIR     = $$OPENRGB_ROOT/.obj_daemon

#-----------------------------------------------------------------------------------------------#
# Windows                                                                                       #
#-----------------------------------------------------------------------------------------------#
win32:QMAKE_CXXFLAGS += /utf-8

win32:DEFINES -=                                                                                \
    UNICODE

win32:DEFINES +=                                                                                \
    _MBCS                                                                                       \
    WIN32                                                                                       \
    _CRT_SECURE_NO_WARNINGS                                                                     \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                                             \
    WIN32_LEAN_AND_MEAN                                                                         \

#-----------------------------------------------------------------------------------------------#
# Install alongside the application                                                             #
#-----------------------------------------------------------------------------------------------#
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    target.path = $$PREFIX/bin/
    INSTALLS   += target

    #-------------------------------------------------------------------------------------------#
    # The build tree rpath above only helps a binary run from the build tree.  An installed     #
    # daemon, which is what a service manager points at, finds libopenrgb here instead.         #
    #-------------------------------------------------------------------------------------------#
    QMAKE_RPATHDIR += $$PREFIX/lib
}

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
