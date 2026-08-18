#-----------------------------------------------------------------------------------------------#
# OpenRGB 0.x QMake Project                                                                     #
#                                                                                               #
#   Adam Honse (CalcProgrammer1)                        5/25/2020                               #
#                                                                                               #
#   OpenRGB builds as a headless core library plus a Qt application on top of it:               #
#                                                                                               #
#       libopenrgb.pro      the core: drivers, device model, managers, SDK.  No Qt.             #
#       OpenRGB_gui.pro     the Qt application                                                  #
#                                                                                               #
#   The project fragments they share live under qmake/:                                         #
#                                                                                               #
#       qmake/version.pri       version numbers and build metadata                              #
#       qmake/sources.pri       source globs                                                    #
#       qmake/drivers.pri       device drivers under Controllers/                               #
#       qmake/core.pri          managers, device model, hardware access - no Qt                 #
#       qmake/dependencies.pri  vendored and system dependencies                                #
#       qmake/gui.pri           the Qt application                                              #
#                                                                                               #
#   This file stays the entry point so that a bare `qmake` keeps working, which packaging and   #
#   CI rely on.                                                                                 #
#-----------------------------------------------------------------------------------------------#
TEMPLATE = subdirs
CONFIG  += ordered

lib.file    = libopenrgb.pro
gui.file    = OpenRGB_gui.pro
gui.depends = lib

SUBDIRS  =                                                                                      \
    lib                                                                                         \
    gui                                                                                         \

#-----------------------------------------------------------------------------------------------#
# Print build configuration                                                                     #
#                                                                                               #
#   NB: scripts/build-package-files.sh parses the VERSION_* lines below.                        #
#-----------------------------------------------------------------------------------------------#
include(qmake/version.pri)

message("GIT_BRANCH:  "$$GIT_BRANCH)
message("VERSION_NUM: "$$VERSION_NUM)
message("VERSION_STR: "$$VERSION_STR)
message("VERSION_SFX: "$$SUFFIX)
message("VERSION_DEB: "$$VERSION_DEB)
message("VERSION_WIX: "$$VERSION_WIX)
message("VERSION_AUR: "$$VERSION_AUR)
message("VERSION_RPM: "$$VERSION_RPM)
message("QT_VERSION:  "$$QT_VERSION)
