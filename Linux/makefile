##################################
#                                #
#   VMMenu MAKEFILE for Linux    #
#                                #
#     For use with GNU Make      #
#                                #
#  Chad Gray, sometime in 2011   #
#                                #
##################################

CC = gcc
LINK = gcc
ASM = NASM
INC = `sdl-config --cflags` -I./VMMSrc -I./Linux -I./Linux/zvg -I./iniparser
LIBS= `sdl-config --libs` -lSDL -lGL -lSDL_gfx -lm -lX11

NAME = vmmenu

OBJS = ./VMMSrc/$(NAME).o \
       ./Linux/zvg/zvgFrame.o \
       ./Linux/zvg/zvgPort.o \
       ./Linux/zvg/timer.o \
       ./Linux/zvg/zvgEnc.o \
       ./Linux/zvg/zvgError.o \
       ./Linux/zvg/zvgBan.o \
       ./iniparser/iniparser.o \
       ./iniparser/dictionary.o \
       ./Linux/LinuxVMM.o \
       ./VMMSrc/vchars.o \
       ./VMMSrc/gamelist.o \
       ./VMMSrc/editlist.o

all: ./$(NAME)

# Clean up intermediate files

clean:
	-rm ./$(NAME)
	-rm ./VMMSrc/*.o
	-rm ./Linux/*.o
	-rm ./Linux/zvg/*.o
	-rm ./iniparser/*.o

# install
install:
	cp ./$(NAME) /usr/local/sbin
	chown root /usr/local/sbin/$(NAME)
	chmod u+s /usr/local/sbin/$(NAME)

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

