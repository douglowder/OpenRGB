#-----------------------------------------------------------------------------------------------#
# gui.pri                                                                                       #
#                                                                                               #
#   The Qt application: widgets, forms, translations, the plugin loader, suspend/resume, and    #
#   the platform entry points.  Everything Qt-dependent belongs here.                           #
#                                                                                               #
#   The GUI_* globs are evaluated in OpenRGB.pro, because $$files() resolves relative to the    #
#   file that calls it.                                                                         #
#-----------------------------------------------------------------------------------------------#
OPENRGB_GUI_INCLUDES +=                                                                         \
    $$GUI_INCLUDES                                                                              \
    dependencies/ColorWheel                                                                     \
    qt/                                                                                         \
    SuspendResume/                                                                              \

OPENRGB_GUI_HEADERS +=                                                                          \
    $$GUI_H                                                                                     \
    dependencies/ColorWheel/ColorWheel.h                                                        \
    OpenRGBPluginInterface.h                                                                    \
    PluginManager.h                                                                             \
    SuspendResume/SuspendResume.h                                                               \

OPENRGB_GUI_SOURCES +=                                                                          \
    $$GUI_CPP                                                                                   \
    dependencies/ColorWheel/ColorWheel.cpp                                                      \
    PluginManager.cpp                                                                           \
    startup/startup.cpp                                                                         \

RESOURCES +=                                                                                    \
    qt/resources.qrc                                                                            \

#-----------------------------------------------------------------------------------------------#
# Translations                                                                                  #
#   NB: Translation files should not be added dynamically due to the process                    #
#       to add new translations relies on entries here                                          #
#-----------------------------------------------------------------------------------------------#
TRANSLATIONS +=                                                                                 \
    qt/i18n/OpenRGB_be_BY.ts                                                                    \
    qt/i18n/OpenRGB_de_DE.ts                                                                    \
    qt/i18n/OpenRGB_el_GR.ts                                                                    \
    qt/i18n/OpenRGB_en_US.ts                                                                    \
    qt/i18n/OpenRGB_en_AU.ts                                                                    \
    qt/i18n/OpenRGB_en_GB.ts                                                                    \
    qt/i18n/OpenRGB_es_ES.ts                                                                    \
    qt/i18n/OpenRGB_fr_FR.ts                                                                    \
    qt/i18n/OpenRGB_hr_HR.ts                                                                    \
    qt/i18n/OpenRGB_it_IT.ts                                                                    \
    qt/i18n/OpenRGB_ja_JP.ts                                                                    \
    qt/i18n/OpenRGB_ko_KR.ts                                                                    \
    qt/i18n/OpenRGB_ms_MY.ts                                                                    \
    qt/i18n/OpenRGB_nb_NO.ts                                                                    \
    qt/i18n/OpenRGB_pl_PL.ts                                                                    \
    qt/i18n/OpenRGB_pt_BR.ts                                                                    \
    qt/i18n/OpenRGB_ru_RU.ts                                                                    \
    qt/i18n/OpenRGB_tr_TR.ts 									\
    qt/i18n/OpenRGB_uk_UA.ts                                                                    \
    qt/i18n/OpenRGB_zh_CN.ts                                                                    \
    qt/i18n/OpenRGB_zh_TW.ts                                                                    \

#-----------------------------------------------------------------------------------------------#
# lupdate must still see the translatable strings that live in core files.  qmake ignores       #
#   lupdate_only scopes, lupdate honours them.                                                  #
#-----------------------------------------------------------------------------------------------#
lupdate_only {
    SOURCES +=                                                                                  \
    DetectionManager.cpp                                                                        \
    ProfileManager.cpp                                                                          \
    ResourceManager.cpp                                                                         \
}

#-----------------------------------------------------------------------------------------------#
# Windows                                                                                       #
#-----------------------------------------------------------------------------------------------#
win32 {
    OPENRGB_GUI_HEADERS +=                                                                      \
    SuspendResume/SuspendResume_Windows.h                                                       \

    OPENRGB_GUI_SOURCES +=                                                                      \
    SuspendResume/SuspendResume_Windows.cpp                                                     \
    startup/main_Windows.cpp                                                                    \

    RC_ICONS +=                                                                                 \
    qt/OpenRGB.ico                                                                              \
}

#-----------------------------------------------------------------------------------------------#
# Linux                                                                                         #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, linux) {
    QT += dbus

    OPENRGB_GUI_HEADERS +=                                                                      \
    SuspendResume/SuspendResume_Linux_FreeBSD.h                                                 \

    OPENRGB_GUI_SOURCES +=                                                                      \
    SuspendResume/SuspendResume_Linux_FreeBSD.cpp                                               \
    startup/main_FreeBSD_Linux_MacOS.cpp                                                        \

    #-------------------------------------------------------------------------------------------#
    # Set up install paths                                                                      #
    # These install paths are used for AppImage and .deb packaging                              #
    #-------------------------------------------------------------------------------------------#
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    !defined(OPENRGB_SYSTEM_PLUGIN_DIRECTORY, var):OPENRGB_SYSTEM_PLUGIN_DIRECTORY =            \
        "$$PREFIX/lib/openrgb/plugins"                                                          \

    DEFINES +=                                                                                  \
        OPENRGB_SYSTEM_PLUGIN_DIRECTORY=\\"\"\"$$OPENRGB_SYSTEM_PLUGIN_DIRECTORY\\"\"\"         \

    #-------------------------------------------------------------------------------------------#
    # Custom target for dynamically created udev_rules                                          #
    #   Ordinarily you would add the 'udev_rules' target to both QMAKE_EXTRA_TARGETS to add a   #
    #   rule in the Makefile and PRE_TARGETDEPS to ensure it is a dependency of the TARGET      #
    #                                                                                           #
    #   ie. QMAKE_EXTRA_TARGETS += udev_rules                                                   #
    #       PRE_TARGETDEPS      += udev_rules                                                   #
    #-------------------------------------------------------------------------------------------#
    CONFIG(release, debug|release) {
        udev_rules.CONFIG       = no_check_exist
        udev_rules.target       = 60-openrgb.rules
        udev_rules.path         = $$PREFIX/lib/udev/rules.d/

        exists($$udev_rules.target) {
            message($$udev_rules.target " - UDEV rules file exists. Removing from build")
            udev_rules.files    = $$udev_rules.target
        } else {
            message($$udev_rules.target " - UDEV rules file missing. Adding script to build")
            #-----------------------------------------------------------------------------------#
            # This is a compiler config flag to save the preproccessed .ii & .s                 #
            #   files so as to automatically process the UDEV rules and the Supported Devices   #
            #-----------------------------------------------------------------------------------#
            QMAKE_CXXFLAGS+=-save-temps
            QMAKE_CXXFLAGS-=-pipe
            udev_rules.extra    = $$PWD/scripts/build-udev-rules.sh $$PWD $$GIT_COMMIT_ID
            udev_rules.files    = $$OUT_PWD/60-openrgb.rules
        }
    }

    #-------------------------------------------------------------------------------------------#
    # Add static files to installation                                                          #
    #-------------------------------------------------------------------------------------------#
    target.path=$$PREFIX/bin/
    desktop.path=$$PREFIX/share/applications/
    desktop.files+=qt/org.openrgb.OpenRGB.desktop
    icon.path=$$PREFIX/share/icons/hicolor/128x128/apps/
    icon.files+=qt/org.openrgb.OpenRGB.png
    metainfo.path=$$PREFIX/share/metainfo/
    metainfo.files+=qt/org.openrgb.OpenRGB.metainfo.xml
    systemd_service.path=$$PREFIX/lib/systemd/system/
    systemd_service.files+=qt/openrgb.service
    tmpfiles.path=$$PREFIX/lib/tmpfiles.d/
    tmpfiles.files+=qt/openrgb.conf
    INSTALLS += target desktop icon metainfo udev_rules systemd_service tmpfiles
}

#-----------------------------------------------------------------------------------------------#
# FreeBSD                                                                                       #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_PLATFORM, freebsd) {
    QT += dbus

    OPENRGB_GUI_HEADERS +=                                                                      \
    SuspendResume/SuspendResume_Linux_FreeBSD.h                                                 \

    OPENRGB_GUI_SOURCES +=                                                                      \
    SuspendResume/SuspendResume_Linux_FreeBSD.cpp                                               \
    startup/main_FreeBSD_Linux_MacOS.cpp                                                        \

    #-------------------------------------------------------------------------------------------#
    # Set up install paths                                                                      #
    # These install paths are used for AppImage and .deb packaging                              #
    #-------------------------------------------------------------------------------------------#
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    target.path=$$PREFIX/bin/
    desktop.path=$$PREFIX/share/applications/
    desktop.files+=qt/org.openrgb.OpenRGB.desktop
    icon.path=$$PREFIX/share/icons/hicolor/128x128/apps/
    icon.files+=qt/org.openrgb.OpenRGB.png
    metainfo.path=$$PREFIX/share/metainfo/
    metainfo.files+=qt/org.openrgb.OpenRGB.metainfo.xml
    rules.path=$$PREFIX/lib/udev/rules.d/
    rules.files+=60-openrgb.rules
    INSTALLS += target desktop icon metainfo rules
}

#-----------------------------------------------------------------------------------------------#
# macOS                                                                                         #
#-----------------------------------------------------------------------------------------------#
macx {
    OPENRGB_GUI_HEADERS +=                                                                      \
    qt/macutils.h                                                                               \
    SuspendResume/SuspendResume_MacOS.h                                                         \

    OPENRGB_GUI_SOURCES +=                                                                      \
    qt/macutils.mm                                                                              \
    SuspendResume/SuspendResume_MacOS.cpp                                                       \
    startup/main_FreeBSD_Linux_MacOS.cpp                                                        \

    ICON = qt/OpenRGB.icns

    info_plist.input = mac/Info.plist.in
    info_plist.output = $$OUT_PWD/Info.plist
    QMAKE_SUBSTITUTES += info_plist
    QMAKE_INFO_PLIST = $$OUT_PWD/Info.plist
}
