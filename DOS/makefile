#
# Basic MAKEFILE
#
# For use with GNU Make, calls DJGPP

CC = GCC
LINK = GCC
ASM = NASM
INC = ..\zvg
#LIBS	= -lalleg

NAME = vmmenu
OBJS = $(NAME).o \
		$(INC)\vchars.o \
		$(INC)\gamelist.o \
		$(INC)\zvgFrame.o \
		$(INC)\zvgPort.o \
		$(INC)\timer.o \
		$(INC)\zvgEnc.o \
		$(INC)\zvgError.o \
		$(INC)\zvgBan.o \
		$(INC)\iniparser.o \
		$(INC)\dictionary.o \
		$(INC)\DOSvmm.o

all: .\$(NAME).exe

# Clean up intermediate files

clean:
	-del .\$(NAME).exe
	-del *.o
	-del $(INC)\*.o

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
