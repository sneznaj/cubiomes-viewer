CC      = gcc
AR      = ar
ARFLAGS = cr
override LDFLAGS = -lm
override CFLAGS += -Wall -Wextra -fwrapv

UNAME_S := $(shell uname -s)
ifeq ($(OS),Windows_NT)
	override CFLAGS += -D_WIN32
	RM = del
	SO_EXT = dll
	SHARED_FLAG = -shared
else
	override LDFLAGS += -pthread
	#RM = rm
	ifeq ($(UNAME_S),Darwin)
		SO_EXT = dylib
		SHARED_FLAG = -dynamiclib
	else
		SO_EXT = so
		SHARED_FLAG = -shared
	endif
endif

.PHONY : all debug release native libcubiomes clean

all: release

debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: libcubiomes
release: CFLAGS += -O3
release: libcubiomes
native: CFLAGS += -O3 -march=native
native: libcubiomes

ifeq ($(OS),Windows_NT)
else
libcubiomes: CFLAGS += -fPIC
endif

libcubiomes: libcubiomes-bedrock.$(SO_EXT)

libcubiomes-bedrock.$(SO_EXT): layers.o generator.o finders.o util.o mcrandom.o api.o
	$(CC) $(LDFLAGS) $(SHARED_FLAG) -o $@ $^ -lm

api.o: api.c generator.h layers.h
	$(CC) -c $(CFLAGS) $<

finders.o: finders.c finders.h
	$(CC) -c $(CFLAGS) $<

generator.o: generator.c generator.h
	$(CC) -c $(CFLAGS) $<

layers.o: layers.c layers.h
	$(CC) -c $(CFLAGS) $<

util.o: util.c util.h
	$(CC) -c $(CFLAGS) $<

mcrandom.o: mcrandom.c mcrandom.h
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) *.o libcubiomes-bedrock.$(SO_EXT)

