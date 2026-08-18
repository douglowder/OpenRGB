#-----------------------------------------------------------------------------------------------#
# libopenrgb.pro                                                                                #
#                                                                                               #
#   The headless OpenRGB core: device drivers, the device model, the managers, the SDK server   #
#   and client, and hardware access.  Links no Qt, so an application built on any toolkit can   #
#   use it.                                                                                     #
#                                                                                               #
#   Built as a shared library on purpose.  Around 1500 device detectors register themselves     #
#   through file scope static objects that nothing references by name.  A static archive would  #
#   let the linker discard those objects, silently removing most device support; a shared       #
#   library links every translation unit and runs its initialisers on load.                     #
#-----------------------------------------------------------------------------------------------#
TEMPLATE    = lib
TARGET      = openrgb

CONFIG     += c++17 shared
CONFIG     -= qt app_bundle
QT         -= core gui widgets

include(qmake/version.pri)
include(qmake/sources.pri)

VERSION     = $$VERSION_NUM

HID_HOTPLUG_ENABLED = "false"

include(qmake/drivers.pri)
include(qmake/core.pri)
include(qmake/dependencies.pri)

INCLUDEPATH +=                                                                                  \
    $$OPENRGB_DRIVER_INCLUDES                                                                   \
    $$OPENRGB_CORE_INCLUDES                                                                     \
    $$OPENRGB_DEP_INCLUDES                                                                      \

HEADERS +=                                                                                      \
    $$OPENRGB_DRIVER_HEADERS                                                                    \
    $$OPENRGB_CORE_HEADERS                                                                      \
    $$OPENRGB_DEP_HEADERS                                                                       \

SOURCES +=                                                                                      \
    $$OPENRGB_DRIVER_SOURCES                                                                    \
    $$OPENRGB_CORE_SOURCES                                                                      \
    $$OPENRGB_DEP_SOURCES                                                                       \

DESTDIR     = $$OPENRGB_ROOT
OBJECTS_DIR = $$OPENRGB_ROOT/.obj_lib

#-----------------------------------------------------------------------------------------------#
# Windows definitions that the core shares with the application                                 #
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
# Let the application find the library through its rpath rather than an absolute build path     #
#-----------------------------------------------------------------------------------------------#
macx:QMAKE_SONAME_PREFIX = @rpath

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

unix:!macx:CONFIG(asan) {
    QMAKE_CFLAGS=-fsanitize=address
    QMAKE_CXXFLAGS=-fsanitize=address
    QMAKE_LFLAGS=-fsanitize=address
}
