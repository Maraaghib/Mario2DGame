CC = gcc
CFLAGS = -g -std=c99 -Wall -D_XOPEN_SOURCE=500 # Pour éviter les warning concernant la fonction ftruncate
CPPFLAGS =  -I../include
VPATH = ../include
LDFLAGS = 

#SOURCES := $(wildcard src/*.c)
SOURCES = maputil.c
OBJECTS = maputil.o

maputil : $(OBJECTS)
	$(CC) $^ -o $@

maputil.o: maputil.c ../include/map.h ../include/error.h

.PHONY: clean
clean :
	-rm -f $(OBJECTS) maputil

#gcc -std=c99 -Iinclude util/maputil.c -o util/maputil

