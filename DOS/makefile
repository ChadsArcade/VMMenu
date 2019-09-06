##################################
#											#
#   VMMenu MAKEFILE for Linux		#
#											#
#     For use with GNU Make		#
#											#
#  Chad Gray, sometime in 2009	#
#											#
##################################

CC = GCC
LINK = GCC
ASM = NASM
INC = -I.\DOS\zvg -I.\DOS -I.\iniparser -I.\VMMSrc
#LIBS	= -lalleg

NAME = vmmenu

OBJS =  .\VMMSrc\$(NAME).o \
	.\DOS\zvg\zvgFrame.o \
	.\DOS\zvg\zvgPort.o \
	.\DOS\zvg\timer.o \
	.\DOS\zvg\zvgEnc.o \
	.\DOS\zvg\zvgError.o \
	.\DOS\zvg\zvgBan.o \
	.\iniparser\iniparser.o \
	.\iniparser\dictionary.o \
	.\DOS\DOSvmm.o \
	.\VMMSrc\vchars.o \
	.\VMMSrc\gamelist.o

all: .\$(NAME).exe

# Clean up intermediate files

clean:
	-del .\$(NAME).exe
	-del .\VMMSrc\*.o
	-del .\DOS\*.o
	-del .\DOS\zvg\*.o
	-del .\iniparser\*.o

# Flags for linker
#
# /x = No map
# /m = Map with publics
# /v = Include debug symbols in link

.\$(NAME).exe : $(OBJS)
#	$(LINK) -s -o .\$(NAME) $(OBJS) $(LIBS)
	$(LINK) -o .\$(NAME) $(OBJS)
	
# Flags for C++ compiler

%.o : %.c
	$(CC) -W -Wall -O2 -c -I$(INC) -s -o $@ $<

# Flags for assembler
#
# -f = Set the file format type
# -X = Set the error type (-Xvc or -Xgnu)

%.o : %.asm
	$(ASM) -f coff -I$(INC) $<

