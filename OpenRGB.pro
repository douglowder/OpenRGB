#-----------------------------------------------------------------------------------------------#
# OpenRGB 0.x QMake Project                                                                     #
#                                                                                               #
#   Adam Honse (CalcProgrammer1)                        5/25/2020                               #
#                                                                                               #
#   The project is split into fragments under build/ so that the headless core can eventually   #
#   be built without the Qt GUI:                                                                #
#                                                                                               #
#       qmake/version.pri       version numbers and build metadata                              #
#       qmake/drivers.pri       device drivers under Controllers/                               #
#       qmake/core.pri          managers, device model, hardware access - no Qt                 #
#       qmake/dependencies.pri  vendored and system dependencies                                #
#       qmake/gui.pri           the Qt application                                              #
#-----------------------------------------------------------------------------------------------#

#-----------------------------------------------------------------------------------------------#
# Qt Configuration                                                                              #
#-----------------------------------------------------------------------------------------------#
QT +=                                                                                           \
    core                                                                                        \
    gui                                                                                         \

#-----------------------------------------------------------------------------------------------#
# Set compiler to use C++17 to make std::filesystem available                                   #
#-----------------------------------------------------------------------------------------------#
CONFIG +=   c++17                                                                               \
            lrelease                                                                            \
            embed_translations                                                                  \
            silent                                                                              \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#-----------------------------------------------------------------------------------------------#
# Application Configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
include(qmake/version.pri)

TARGET      = OpenRGB
TEMPLATE    = app

HID_HOTPLUG_ENABLED = "false"

#-----------------------------------------------------------------------------------------------#
# Source globs                                                                                  #
#                                                                                               #
#   These stay here rather than in the fragments because $$files() resolves relative to the     #
#   file that calls it, so globbing from qmake/ would find nothing.                             #
#-----------------------------------------------------------------------------------------------#
FORMS += $$files("qt/*.ui", true)

GUI_H               = $$files("qt/*.h", true)
GUI_CPP             = $$files("qt/*.cpp", true)

for(iter, $$list($$GUI_H)) {
    GUI_INCLUDES += $$dirname(iter)
}
GUI_INCLUDES        = $$unique(GUI_INCLUDES)

CONTROLLER_H        = $$files("Controllers/*.h", true)
CONTROLLER_CPP      = $$files("Controllers/*.cpp", true)

for(iter, $$list($$CONTROLLER_H)) {
    CONTROLLER_INCLUDES += $$dirname(iter)
}
CONTROLLER_INCLUDES = $$unique(CONTROLLER_INCLUDES)

#-----------------------------------------------------------------------------------------------#
# Remove OS-specific files from the overall controller headers and sources lists                #
# The suffixes _Windows, _Linux, _FreeBSD, and _MacOS are usable to denote that a file only     #
# applies to one or more OSes.  The suffixes may be combined such as <file>_Windows_Linux.cpp.  #
#-----------------------------------------------------------------------------------------------#
CONTROLLER_H_WINDOWS    = $$files("Controllers/*_Windows*.h",   true)
CONTROLLER_CPP_WINDOWS  = $$files("Controllers/*_Windows*.cpp", true)
CONTROLLER_H_LINUX      = $$files("Controllers/*_Linux*.h",     true)
CONTROLLER_CPP_LINUX    = $$files("Controllers/*_Linux*.cpp",   true)
CONTROLLER_H_FREEBSD    = $$files("Controllers/*_FreeBSD*.h",   true)
CONTROLLER_CPP_FREEBSD  = $$files("Controllers/*_FreeBSD*.cpp", true)
CONTROLLER_H_MACOS      = $$files("Controllers/*_MacOS*.h",     true)
CONTROLLER_CPP_MACOS    = $$files("Controllers/*_MacOS*.cpp",   true)

CONTROLLER_H           -= $$CONTROLLER_H_WINDOWS
CONTROLLER_H           -= $$CONTROLLER_H_LINUX
CONTROLLER_H           -= $$CONTROLLER_H_FREEBSD
CONTROLLER_H           -= $$CONTROLLER_H_MACOS

CONTROLLER_CPP         -= $$CONTROLLER_CPP_WINDOWS
CONTROLLER_CPP         -= $$CONTROLLER_CPP_LINUX
CONTROLLER_CPP         -= $$CONTROLLER_CPP_FREEBSD
CONTROLLER_CPP         -= $$CONTROLLER_CPP_MACOS

#-----------------------------------------------------------------------------------------------#
# Project fragments                                                                             #
#-----------------------------------------------------------------------------------------------#
include(qmake/drivers.pri)
include(qmake/core.pri)
include(qmake/dependencies.pri)
include(qmake/gui.pri)

#-----------------------------------------------------------------------------------------------#
# Assemble the target from the fragments                                                        #
#                                                                                               #
#   A build without a GUI leaves out the OPENRGB_GUI_* variables; everything else is shared.    #
#-----------------------------------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    $$OPENRGB_DRIVER_INCLUDES                                                                   \
    $$OPENRGB_CORE_INCLUDES                                                                     \
    $$OPENRGB_DEP_INCLUDES                                                                      \
    $$OPENRGB_GUI_INCLUDES                                                                      \

HEADERS +=                                                                                      \
    $$OPENRGB_DRIVER_HEADERS                                                                    \
    $$OPENRGB_CORE_HEADERS                                                                      \
    $$OPENRGB_DEP_HEADERS                                                                       \
    $$OPENRGB_GUI_HEADERS                                                                       \

SOURCES +=                                                                                      \
    $$OPENRGB_DRIVER_SOURCES                                                                    \
    $$OPENRGB_CORE_SOURCES                                                                      \
    $$OPENRGB_DEP_SOURCES                                                                       \
    $$OPENRGB_GUI_SOURCES                                                                       \

#-----------------------------------------------------------------------------------------------#
# Windows GitLab CI Configuration                                                               #
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

win32:CONFIG(debug, debug|release) {
    win32:DESTDIR = debug
}

win32:CONFIG(release, debug|release) {
    win32:DESTDIR = release
}

win32:OBJECTS_DIR = _intermediate_$$DESTDIR/.obj
win32:MOC_DIR     = _intermediate_$$DESTDIR/.moc
win32:RCC_DIR     = _intermediate_$$DESTDIR/.qrc
win32:UI_DIR      = _intermediate_$$DESTDIR/.ui

#-----------------------------------------------------------------------------------------------#
# Copy dependencies to output directory                                                         #
#-----------------------------------------------------------------------------------------------#
win32:contains(QMAKE_TARGET.arch, x86_64) {
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/libusb-1.0.27/VS2019/MS64/dll/libusb-1.0.dll)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/hidapi-hotplug-win/x64/hidapi-hotplug.dll   )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/PawnIOLib.dll                        )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusPIIX4.bin               )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusI801.bin                )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusIntelSkylakeIMC.bin     )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusNCT6793.bin             )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/LpcIO.bin                    )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    first.depends = $(first) copydata
    export(first.depends)
    export(copydata.commands)
    QMAKE_EXTRA_TARGETS += first copydata
}

win32:contains(QMAKE_TARGET.arch, x86) {
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/libusb-1.0.27/VS2019/MS32/dll/libusb-1.0.dll)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/hidapi-hotplug-win/x86/hidapi-hotplug.dll   )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)

    first.depends = $(first) copydata
    export(first.depends)
    export(copydata.commands)
    QMAKE_EXTRA_TARGETS += first copydata
}

#-----------------------------------------------------------------------------------------------#
# Linux and FreeBSD binaries are lower case                                                     #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, linux):TARGET   = $$lower($$TARGET)
contains(QMAKE_PLATFORM, freebsd):TARGET = $$lower($$TARGET)

unix:!macx:CONFIG(asan) {
    message("ASan Mode")
    QMAKE_CFLAGS=-fsanitize=address
    QMAKE_CXXFLAGS=-fsanitize=address
    QMAKE_LFLAGS=-fsanitize=address
}

#-----------------------------------------------------------------------------------------------#
# MacOS-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

DISTFILES += \
    debian/openrgb-udev.postinst \
    debian/openrgb.postinst

#-----------------------------------------------------------------------------------------------#
# Print build configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
message("GIT_BRANCH:  "$$GIT_BRANCH)
message("VERSION_NUM: "$$VERSION_NUM)
message("VERSION_STR: "$$VERSION_STR)
message("VERSION_SFX: "$$SUFFIX)
message("VERSION_DEB: "$$VERSION_DEB)
message("VERSION_WIX: "$$VERSION_WIX)
message("VERSION_AUR: "$$VERSION_AUR)
message("VERSION_RPM: "$$VERSION_RPM)
message("QT_VERSION:  "$$QT_VERSION)
equals(HID_HOTPLUG_ENABLED, "true") {
message("HID Hotplug: Enabled")
} else {
message("HID Hotplug: Disabled")
}
