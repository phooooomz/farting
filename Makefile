# ECLIPSE: Little Wonders of Aetheria
# Build with the PSP SDK (pspdev). Output: EBOOT.PBP

TARGET = eclipse
SRCS = $(wildcard src/*.c) $(wildcard src/core/*.c)
OBJS = $(SRCS:.c=.o)

INCDIR = src
CFLAGS = -O2 -G0 -Wall -Wextra -Wno-unused-parameter -std=gnu99 -Isrc -Isrc/core
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

PSP_EBOOT_TITLE = ECLIPSE: Little Wonders of Aetheria
PSP_FW_VERSION = 371

LIBS = -lpspgu -lpspgum -lpspdisplay -lpspctrl -lpspkernel -lm

EXTRA_TARGETS = EBOOT.PBP

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
