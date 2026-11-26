TARGET = FantasyRPG
OBJS = src/main.o src/game.o

INCDIR =
CFLAGS = -O2 -G0 -Wall -Wextra
CXXFLAGS = $(CFLAGS) -fno-exceptions
ASFLAGS = $(CFLAGS)
LIBS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Fantasy RPG - Starter

include $(PSPSDK)/lib/build.mak
