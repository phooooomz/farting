TARGET = FantasyRPG

OBJS = src/main.o src/game.o

INCDIR = src
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

LIBS = -lpspdebug -lpspdisplay -lpspge -lpspctrl

BUILD_PRX = 0
PSP_FW_VERSION = 660

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Fantasy RPG

PSPSDK := $(shell psp-config --pspsdk-path)

ifeq ($(PSPSDK),)
$(error PSP SDK path not found! psp-config failed)
endif

include $(PSPSDK)/lib/build.mak