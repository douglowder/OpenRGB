#-----------------------------------------------------------------------------------------------#
# OpenRGB_gui.pro                                                                               #
#                                                                                               #
#   The Qt application.  Contains only user interface code; everything that touches a device    #
#   comes from libopenrgb.                                                                      #
#-----------------------------------------------------------------------------------------------#
TEMPLATE    = app
TARGET      = OpenRGB

QT         += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG     += c++17                                                                             \
              lrelease                                                                          \
              embed_translations                                                                \
              silent                                                                            \

include(qmake/version.pri)
include(qmake/sources.pri)

HID_HOTPLUG_ENABLED = "false"

#-----------------------------------------------------------------------------------------------#
# The fragments below are included for their include paths and their platform configuration.    #
#   Only the GUI sources are compiled here; the rest arrive through libopenrgb.                 #
#-----------------------------------------------------------------------------------------------#
include(qmake/drivers.pri)
include(qmake/core.pri)
include(qmake/dependencies.pri)
include(qmake/gui.pri)

INCLUDEPATH +=                                                                                  \
    $$OPENRGB_DRIVER_INCLUDES                                                                   \
    $$OPENRGB_CORE_INCLUDES                                                                     \
    $$OPENRGB_DEP_INCLUDES                                                                      \
    $$OPENRGB_GUI_INCLUDES                                                                      \

HEADERS +=                                                                                      \
    $$OPENRGB_GUI_HEADERS                                                                       \

SOURCES +=                                                                                      \
    $$OPENRGB_GUI_SOURCES                                                                       \

#-----------------------------------------------------------------------------------------------#
# Link the core                                                                                 #
#-----------------------------------------------------------------------------------------------#
LIBS           += -L$$OPENRGB_ROOT -lopenrgb
unix:QMAKE_RPATHDIR += $$OPENRGB_ROOT

DESTDIR         = $$OPENRGB_ROOT
OBJECTS_DIR     = $$OPENRGB_ROOT/.obj_gui
MOC_DIR         = $$OPENRGB_ROOT/.moc_gui
RCC_DIR         = $$OPENRGB_ROOT/.qrc_gui
UI_DIR          = $$OPENRGB_ROOT/.ui_gui

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

win32:CONFIG(debug, debug|release) {
    win32:DESTDIR = debug
}

win32:CONFIG(release, debug|release) {
    win32:DESTDIR = release
}

#-----------------------------------------------------------------------------------------------#
# Copy dependencies to output directory                                                         #
#-----------------------------------------------------------------------------------------------#
win32:contains(QMAKE_TARGET.arch, x86_64) {
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/libusb-1.0.27/VS2019/MS64/dll/libusb-1.0.dll)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/hidapi-hotplug-win/x64/hidapi-hotplug.dll   )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/PawnIO/PawnIOLib.dll                        )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/PawnIO/modules/SmbusPIIX4.bin               )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/PawnIO/modules/SmbusI801.bin                )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/PawnIO/modules/SmbusIntelSkylakeIMC.bin     )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/PawnIO/modules/SmbusNCT6793.bin             )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/PawnIO/modules/LpcIO.bin                    )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    first.depends = $(first) copydata
    export(first.depends)
    export(copydata.commands)
    QMAKE_EXTRA_TARGETS += first copydata
}

win32:contains(QMAKE_TARGET.arch, x86) {
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/libusb-1.0.27/VS2019/MS32/dll/libusb-1.0.dll)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$OPENRGB_ROOT/dependencies/hidapi-hotplug-win/x86/hidapi-hotplug.dll   )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)

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
    QMAKE_CFLAGS=-fsanitize=address
    QMAKE_CXXFLAGS=-fsanitize=address
    QMAKE_LFLAGS=-fsanitize=address
}

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

DISTFILES += \
    debian/openrgb-udev.postinst \
    debian/openrgb.postinst
