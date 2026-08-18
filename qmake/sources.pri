#-----------------------------------------------------------------------------------------------#
# sources.pri                                                                                   #
#                                                                                               #
#   Source globs, shared by every target.                                                       #
#                                                                                               #
#   $$files() resolves relative to the file that calls it, so the globs are rooted at an        #
#   absolute path and converted back to project-relative afterwards.  Keeping the paths         #
#   relative matters: qmake derives object file names from them.                                #
#-----------------------------------------------------------------------------------------------#
OPENRGB_ROOT = $$clean_path($$PWD/..)

defineReplace(openrgbRelative) {
    result =
    for(path, ARGS) {
        result += $$relative_path($$path, $$OPENRGB_ROOT)
    }
    return($$result)
}

FORMS += $$openrgbRelative($$files($$OPENRGB_ROOT/qt/*.ui, true))

GUI_H               = $$openrgbRelative($$files($$OPENRGB_ROOT/qt/*.h, true))
GUI_CPP             = $$openrgbRelative($$files($$OPENRGB_ROOT/qt/*.cpp, true))

for(iter, $$list($$GUI_H)) {
    GUI_INCLUDES += $$dirname(iter)
}
GUI_INCLUDES        = $$unique(GUI_INCLUDES)

CONTROLLER_H        = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*.h, true))
CONTROLLER_CPP      = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*.cpp, true))

for(iter, $$list($$CONTROLLER_H)) {
    CONTROLLER_INCLUDES += $$dirname(iter)
}
CONTROLLER_INCLUDES = $$unique(CONTROLLER_INCLUDES)

#-----------------------------------------------------------------------------------------------#
# Remove OS-specific files from the overall controller headers and sources lists                #
# The suffixes _Windows, _Linux, _FreeBSD, and _MacOS are usable to denote that a file only     #
# applies to one or more OSes.  The suffixes may be combined such as <file>_Windows_Linux.cpp.  #
#-----------------------------------------------------------------------------------------------#
CONTROLLER_H_WINDOWS    = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_Windows*.h,   true))
CONTROLLER_CPP_WINDOWS  = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_Windows*.cpp, true))
CONTROLLER_H_LINUX      = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_Linux*.h,     true))
CONTROLLER_CPP_LINUX    = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_Linux*.cpp,   true))
CONTROLLER_H_FREEBSD    = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_FreeBSD*.h,   true))
CONTROLLER_CPP_FREEBSD  = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_FreeBSD*.cpp, true))
CONTROLLER_H_MACOS      = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_MacOS*.h,     true))
CONTROLLER_CPP_MACOS    = $$openrgbRelative($$files($$OPENRGB_ROOT/Controllers/*_MacOS*.cpp,   true))

CONTROLLER_H           -= $$CONTROLLER_H_WINDOWS
CONTROLLER_H           -= $$CONTROLLER_H_LINUX
CONTROLLER_H           -= $$CONTROLLER_H_FREEBSD
CONTROLLER_H           -= $$CONTROLLER_H_MACOS

CONTROLLER_CPP         -= $$CONTROLLER_CPP_WINDOWS
CONTROLLER_CPP         -= $$CONTROLLER_CPP_LINUX
CONTROLLER_CPP         -= $$CONTROLLER_CPP_FREEBSD
CONTROLLER_CPP         -= $$CONTROLLER_CPP_MACOS
