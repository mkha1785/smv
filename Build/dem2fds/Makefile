
# dem2fds makefile

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS
INC_DIR =  -I $(SOURCE_DIR)/gd-2.0.15 -I $(SOURCE_DIR)/zlib128 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/dem2fds

WIN_LIBDIR = $(LIB_DIR)/intel_win_64
LIBS_WIN = $(WIN_LIBDIR)\gd.lib $(WIN_LIBDIR)\jpeg.lib $(WIN_LIBDIR)\png.lib $(WIN_LIBDIR)\zlib.lib

#--- OSX

LIBS_OSX = -lgd -ljpeg -lpng -lz

#--- Linux

LIBS_LINUX = $(LIBS_OSX)


FILTERC =
FILTERF =

VPATH = $(SOURCE_DIR)/dem2fds $(SOURCE_DIR)/shared  
bin = .

ifeq ($(shell echo "check_quotes"),"check_quotes")
  GIT_HASH := $(shell ..\..\..\Utilities\Scripts\githash)
  GIT_DATE := $(shell ..\..\..\Utilities\Scripts\gitlog)
else
  GIT_HASH := $(shell git describe --long --dirty)
  GIT_DATE := $(shell git log -1 --format=%cd)
endif
GITINFO=-Dpp_GITHASH=\"$(GIT_HASH)\" -Dpp_GITDATE=\""$(GIT_DATE)\""

ifeq ($(shell echo "check_quotes"),"check_quotes")
  INTEL_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\intel_compversion)
  GNU_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\gnu_compversion)
else
  INTEL_COMPVERSION := $(shell ../../../Utilities/Scripts/intel_compversion.sh)
  GNU_COMPVERSION := $(shell ../../../Utilities/Scripts/gnu_compversion.sh)
endif
INTEL_COMPINFO=-Dpp_COMPVER=\"$(INTEL_COMPVERSION)\"
GNU_COMPINFO=-Dpp_COMPVER=\"$(GNU_COMPVERSION)\"

# Definition of the object variables

obj =    assert.o dmalloc.o file_util.o string_util.o dem_util.o  md5.o sha1.o sha256.o main.o
objwin = $(obj:.o=.obj)

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .c .obj .o 

.c.obj:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<
.c.o:
	$(CC) -c $(CFLAGS) $(FILTERC) $(INC_DIR) $<



# ********  rules for each platform supported *************

# ------------- Windows 64 ----------------

intel_win_64 : CFLAGS    = -Od -D WIN32 -D pp_INTEL /nologo $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : CC        = icl
intel_win_64 : exe       = dem2fds_win_64.exe

intel_win_64 : $(objwin)
	$(CC) -o $(bin)/$(exe) $(objwin) $(LIBS_WIN)

# ------------- gnu Linux 64 ----------------

gnu_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64 : CFLAGS    = -O2 -m64 -Wall -D pp_GCC -D pp_LINUX $(GITINFO) $(GNU_COMPINFO)
gnu_linux_64 : CC        = gcc
gnu_linux_64 : exe       = dem2fds_linux_64

gnu_linux_64: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -L$(LIB_DIR_PLAT) $(LIBS_LINUX)  -lm


# ------------- Linux 64 ----------------
#  64 bit linux

intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : CFLAGS    = -O2 -m64 -D pp_LINUX -D pp_INTEL $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : CC        = icc
intel_linux_64 : exe       = dem2fds_linux_64

intel_linux_64 : $(obj)
	$(CC) -o $(bin)/$(exe) $(obj) -L$(LIB_DIR_PLAT) $(LIBS_LINUX)

	
# ------------- OSX 64 ----------------
#  64 bit OSX

intel_osx_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_osx_64
intel_osx_64 : CFLAGS    = -O2 -m64 -D pp_OSX -D pp_INTEL $(GITINFO) $(INTEL_COMPINFO)
intel_osx_64 : CC        = icc
intel_osx_64 : exe       = dem2fds_osx_64

intel_osx_64 : $(obj)
	$(CC) -o $(bin)/$(exe) -m64 $(obj) -L $(LIB_DIR_PLAT)  $(LIBS_OSX)

# ------------- gnu OSX 64 ----------------

gnu_osx_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_osx_64
gnu_osx_64 : CFLAGS    = -O2 -m64 -Wall -D pp_GCC -D pp_OSX $(GITINFO) $(GNU_COMPINFO)
gnu_osx_64 : CC        = gcc
gnu_osx_64 : exe       = dem2fds_osx_64

gnu_osx_64: $(obj)
	$(CC) -m64 -o $(bin)/$(exe) $(obj) -L$(LIB_DIR_PLAT) $(LIBS_OSX)
