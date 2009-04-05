#
#   OPTIONS
#
DEBUG := 1
#DEBUG_PARANOID := 1
export DEBUG
export DEBUG_PARANOID


# OS
OS := LINUX
#OS := FREEBSD
#OS := WIN32
#OS := MACOSX
export OS


# Data path
#NDATA_DEF := \"../ndata\"


ifeq ($(OS),LINUX)
include build/Makefile.posix
endif

ifeq ($(OS),FREEBSD)
include build/Makefile.posix
endif

ifeq ($(OS),MACOSX)
include build/Makefile.posix
endif

ifeq ($(OS),WIN32)
include build/Makefile.win32
endif
