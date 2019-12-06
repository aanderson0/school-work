ifeq ($(OS),Windows_NT)
##
##	Makefile for file locker
##


##
##	Variables
##	---------
##

EXE	= lockRanger.exe

# List of all of the object files in your project
OBJS = \
		lockRanger.obj


# Missing Windows install locations
SYS_CLOC = c:\Program Files (x86)\Windows Kits\10
SYS_HEADER_LOC_A	= $(SYS_CLOC)\Include\10.0.17763.0\ucrt
SYS_HEADER_LOC_B	= $(SYS_CLOC)\Include\10.0.17763.0\um
SYS_HEADER_LOC_C	= $(SYS_CLOC)\Include\10.0.17763.0\shared
SYS_LIB_LOC_A	= $(SYS_CLOC)\Lib\10.0.17763.0\um\x64
SYS_LIB_LOC_B	= $(SYS_CLOC)\Lib\10.0.17763.0\ucrt\x64
SYS_LIB_LOC_C	= c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023\lib\onecore\x64

CC		= cl

# Flags for compiling
CFLAGS	= /I"$(SYS_HEADER_LOC_A)" /I"$(SYS_HEADER_LOC_B)" /I"$(SYS_HEADER_LOC_C)"

# Libs that need to be linked
LDFLAGS	= /LIBPATH:"$(SYS_LIB_LOC_A)" /LIBPATH:"$(SYS_LIB_LOC_B)" /LIBPATH:"$(SYS_LIB_LOC_C)" /MACHINE:x64
LDLIBS =


##
##	Build Targets
##	-------------
##

# Default target - build the program
$(EXE) exe : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) /link /out:$(EXE) $(LDFLAGS) $(LDLIBS)
	icacls $(EXE) /grant Everyone:RX

lockRanger.E :
	$(CC) $(CFLAGS) /E lockRanger.c > lockRanger.E

##
##	Cleanup Targets
##	---------------
##

clean :
	- del /f $(EXE)
	- del /f $(OBJS)
	- del /f *.pdb

else 
ifeq ($(UNAME_S),Darwin)

EXE	= lockRanger
OBJS = lockRanger.o 
CFLAGS = -g

all world : $(EXE)

$(EXE) : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LDLIBS)

clean : 
	- rm -f lockRanger
	- rm -f *.o
	
else

EXE	= lockRanger
OBJS = lockRanger.o 
CFLAGS = -g

all world : $(EXE)

$(EXE) : $(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS) $(LDLIBS)

clean : 
	- rm -f lockRanger
	- rm -f *.o
endif
endif
