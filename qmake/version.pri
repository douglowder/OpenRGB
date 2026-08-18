#-----------------------------------------------------------------------------------------------#
# version.pri                                                                                   #
#                                                                                               #
#   Version numbers and build metadata.  Every target needs these, since VERSION_STRING is      #
#   used by core code as well as the GUI.                                                       #
#                                                                                               #
#   NB: scripts/build-package-files.sh parses the VERSION_* message() output that OpenRGB.pro   #
#       prints, so those message() calls must stay in the top level project file.               #
#-----------------------------------------------------------------------------------------------#
MAJOR       = 0
MINOR       = 9
SUFFIX      = git

SHORTHASH   = $$system("git rev-parse --short=7 HEAD")
LASTTAG     = "release_"$$MAJOR"."$$MINOR
COMMAND     = "git rev-list --count "$$LASTTAG"..HEAD"
COMMITS     = $$system($$COMMAND)

VERSION_NUM = $$MAJOR"."$$MINOR"."$$COMMITS
VERSION_STR = $$MAJOR"."$$MINOR

VERSION_DEB = $$VERSION_NUM
VERSION_WIX = $$VERSION_NUM
VERSION_AUR = $$VERSION_NUM
VERSION_RPM = $$VERSION_NUM

equals(SUFFIX, "git") {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX$$COMMITS")"
VERSION_DEB = $$VERSION_DEB"~git"$$SHORTHASH
VERSION_AUR = $$VERSION_AUR".g"$$SHORTHASH
VERSION_RPM = $$VERSION_RPM"^git"$$SHORTHASH
} else {
    !isEmpty(SUFFIX) {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX")"
VERSION_DEB = $$VERSION_DEB"~"$$SUFFIX
VERSION_AUR = $$VERSION_AUR"."$$SUFFIX
VERSION_RPM = $$VERSION_RPM"^"$$SUFFIX
    }
}

#-----------------------------------------------------------------------------------------------#
# Automatically generated build information                                                     #
#-----------------------------------------------------------------------------------------------#
win32:BUILDDATE         = $$system(date /t)
linux:BUILDDATE         = $$system(date -R -d "@${SOURCE_DATE_EPOCH:-$(date +%s)}")
freebsd:BUILDDATE       = $$system(date -j -R -r "${SOURCE_DATE_EPOCH:-$(date +%s)}")
macx:BUILDDATE          = $$system(date -j -R -r "${SOURCE_DATE_EPOCH:-$(date +%s)}")
GIT_COMMIT_ID           = $$system(git log -n 1 --pretty=format:"%H")
GIT_COMMIT_DATE         = $$system(git log -n 1 --pretty=format:"%ci")

unix {
    GIT_BRANCH          = $$system(sh $$PWD/../scripts/git-get-branch.sh)
}
else {
    GIT_BRANCH          = $$system(powershell -ExecutionPolicy Bypass -File $$PWD/../scripts/git-get-branch.ps1)
}

DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$VERSION_STR\\"\"\"                                                  \
    BUILDDATE_STRING=\\"\"\"$$BUILDDATE\\"\"\"                                                  \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\"                                                 \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\"                                             \
    GIT_BRANCH=\\"\"\"$$GIT_BRANCH\\"\"\"
