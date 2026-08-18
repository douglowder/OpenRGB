#!/usr/bin/env bash
#-----------------------------------------------------------------------------#
# check-qt-free.sh                                                            #
#                                                                             #
#   Reports any Qt usage outside the GUI directory.  The headless core must   #
#   stay Qt-free so that it can be built as a library and linked by apps      #
#   using other toolkits.                                                     #
#                                                                             #
#   Exits non-zero if a file outside the allow-list uses Qt.                  #
#                                                                             #
#   This file is part of the OpenRGB project                                  #
#   SPDX-License-Identifier: GPL-2.0-or-later                                 #
#-----------------------------------------------------------------------------#
set -uo pipefail

cd "$(dirname "$0")/.." || exit 2

#-----------------------------------------------------------------------------#
# Files that are allowed to use Qt.  Everything here belongs to the GUI        #
# application rather than the core library.                                    #
#-----------------------------------------------------------------------------#
ALLOWED=(
    "PluginManager.h"
    "PluginManager.cpp"
    "OpenRGBPluginInterface.h"
    "startup/startup.cpp"
    "SuspendResume/SuspendResume_Linux_FreeBSD.h"
    "SuspendResume/SuspendResume_Linux_FreeBSD.cpp"
    "SuspendResume/SuspendResume_Windows.h"
    "SuspendResume/SuspendResume_Windows.cpp"
)

#-----------------------------------------------------------------------------#
# QT_TRANSLATE_NOOP is a false positive.  SettingsManager.h defines it as a    #
# pass-through macro so that lupdate can harvest settings strings; it pulls    #
# in no Qt header.                                                            #
#-----------------------------------------------------------------------------#
PATTERN='#include[[:space:]]*<Q|Q_OBJECT|Q_DECLARE_INTERFACE|\bQString\b|\bQWidget\b|\bQMenu\b|\bQImage\b|\bQApplication\b|\bQObject\b|\bqApp\b'

#-----------------------------------------------------------------------------#
# Scan tracked sources only.  An in-source build leaves generated moc_*.cpp    #
# and ui_*.h in the tree, and those legitimately use Qt.                       #
#-----------------------------------------------------------------------------#
mapfile -t HITS < <(
    git ls-files '*.h' '*.cpp' '*.mm' '*.c' \
    | grep -v '^qt/' \
    | grep -v '^dependencies/' \
    | xargs grep -lE "$PATTERN" 2>/dev/null \
    | sort
)

status=0
unexpected=()

for hit in "${HITS[@]}"; do
    allowed=0
    for ok in "${ALLOWED[@]}"; do
        [ "$hit" = "$ok" ] && allowed=1 && break
    done
    if [ "$allowed" -eq 0 ]; then
        unexpected+=("$hit")
        status=1
    fi
done

echo "Qt usage outside qt/ and dependencies/:"
for hit in "${HITS[@]}"; do
    for ok in "${ALLOWED[@]}"; do
        [ "$hit" = "$ok" ] && echo "  allowed   $hit" && continue 2
    done
    echo "  UNEXPECTED $hit"
done

echo
if [ "$status" -eq 0 ]; then
    echo "PASS  ${#HITS[@]} file(s) use Qt, all on the GUI allow-list"
else
    echo "FAIL  ${#unexpected[@]} unexpected file(s) use Qt outside the GUI"
fi

exit $status
