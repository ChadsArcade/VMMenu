#
# Basic MAKEFILE
#
# For use with GNU Make, calls DJGPP

CC = gcc
LINK = gcc
ASM = NASM
INC = 	`sdl-config --cflags` -I../zvg
LIBS= `sdl-config --libs` -lSDL -lGL -lSDL_gfx -lm -lX11

NAME = vmmenu

OBJS = ./$(NAME).o \
       ../zvg/vchars.o \
       ../zvg/gamelist.o \
       ../zvg/zvgFrame.o \
       ../zvg/zvgPort.o \
       ../zvg/timer.o \
       ../zvg/zvgEnc.o \
       ../zvg/zvgError.o \
       ../zvg/zvgBan.o \
       ../zvg/iniparser.o \
       ../zvg/dictionary.o \
       ../zvg/LinuxVMM.o

all: ./$(NAME)

# Clean up intermediate files

clean:
	-rm ./$(NAME)
	-rm *.o
	-rm ../zvg/*.o

# Flags for linker
#
# /x = No map
# /m = Map with publics
# /v = Include debug symbols in link

./$(NAME) : $(OBJS)
#	$(LINK) -s -o ./$(NAME) $(OBJS) $(LIBS)
	$(LINK) -o ./$(NAME) $(OBJS) $(LIBS)
	
# Flags for C++ compiler

%.o : %.c
#	$(CC) -W -Wall -O2 -c -I$(INC) -o $@ $<
	$(CC) -W -Wall -O2 -c -s -g -ggdb3 $(INC) -o $@ $<

# Flags for assembler
#
# -f = Set the file format type
# -X = Set the error type (-Xvc or -Xgnu)

%.o : %.asm
	$(ASM) -f coff -I$(INC) $<
