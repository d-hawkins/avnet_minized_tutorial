# -----------------------------------------------------------------------------
# Makefile
#
# 4/11/2021 D. W. Hawkins (dwh@caltech.edu)
#
# FTDI D2XX library applications.
#
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Targets
# -----------------------------------------------------------------------------
#
default: apps

# -----------------------------------------------------------------------------
# Build parameters
# -----------------------------------------------------------------------------
#
# Cygwin or Linux?
CYGWIN_BUILD = 0
ifeq ("$(shell uname -o)","Cygwin")
CYGWIN_BUILD = 1
endif

# FTDI source path (windows string format is ok)
ifeq ($(CYGWIN_BUILD),1)
FTDI_SOURCE = "C:\software\ftdi\ftd2xx"
else
FTDI_SOURCE = .
endif

# Convert paths to unix format
ifeq ($(CYGWIN_BUILD),1)
FTDI_HDR = $(shell cygpath -u $(FTDI_SOURCE))
# Select the 64-bit or 32-bit DLL
ifeq ("$(shell uname -m)","x86_64")
FTDI_LIB = $(shell cygpath -u $(FTDI_SOURCE))/amd64
else
FTDI_LIB = $(shell cygpath -u $(FTDI_SOURCE))/i386
endif
else
FTDI_HDR = $(FTDI_SOURCE)
endif

# Compiler and linker
CC = g++
LD = g++

# Compiler options
CPPFLAGS += -Wall -I$(FTDI_HDR) -D_WIN32

# Linker options
ifeq ($(CYGWIN_BUILD),1)
LDFLAGS += -L$(FTDI_LIB)
endif

# Linker libraries
ifeq ($(CYGWIN_BUILD),1)
# Cygwin
ifeq ("$(shell uname -m)","x86_64")
LDLIBS += -lftd2xx64 -lreadline -lpthread -lm
else
LDLIBS += -lftd2xx -lreadline -lpthread -lm
endif
else
# Linux
LDLIBS += -lftd2xx -lreadline -lpthread -lm
endif
ifeq ($(CYGWIN_BUILD),0)
LDLIBS += -lncurses -Wl,-rpath /usr/local/lib
endif

# -----------------------------------------------------------------
# Applications
# -----------------------------------------------------------------

APPS = \
	ftdi_list_devices \
	minized_ftdi_serial

apps: $(APPS)

clean:
	@rm -rf $(APPS) *.o
