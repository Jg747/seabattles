# Project name
NAME = seabattles

# Example: cpp, c ...
EXT = cpp

# Compilers: gcc, g++, clang, clang++
CC = g++

# Example: -lncurses -lyaml ...
LIBRARIES = -lncurses
#########################################################

CFLAGS = -Wall
INCLUDE = include
SOURCE = src
BIN = bin
MK = make

ifneq ($(OS), Windows_NT)

RM = rm
MV = mv
CP = cp

# Install path (MacOS / Linux)
DEST = /usr/local/bin
# Other compilation extras, leave empty if nothing extra
OTHER = 
# Executable name & extension (or no extension)
EXECUTABLE = $(NAME)

else

#RM = del
#RMDIR = rmdir
#MV = move
#CP = copy
RM = rm
MV = mv
CP = cp

# Install path (Windows)
DEST = C:/Program Files/$(NAME)_pgm
# Other compilation extras, leave empty if nothing extra
OTHER = icons/icon.res
# Executable name & extension (or no extension)
EXECUTABLE = $(NAME).exe

endif

ifeq ($(shell uname -s), Darwin)
	DBG = lldb
else
	DBG = gdb
#	OTHER += -static
endif

ifneq ($(shell uname -s), Darwin)
ifneq ($(OS), Windows_NT)
	LIBRARIES += -ltinfo
endif
endif

default: build

build:
ifneq ($(OS), Windows_NT)

ifeq ($(wildcard $(BIN)/.*),)
	@mkdir $(BIN)
endif
ifndef debug
	@$(CC) $(CFLAGS) -I $(INCLUDE) -c src/*.$(EXT)
else
ifeq ($(debug), true)
	@$(CC) $(CFLAGS) -g -I $(INCLUDE) -c src/*.$(EXT)
else
	@$(CC) $(CFLAGS) -I $(INCLUDE) -c src/*.$(EXT)
endif
endif
	@$(CC) *.o $(OTHER) -o $(EXECUTABLE) $(LIBRARIES)
	@$(MV) *.o $(BIN)
ifndef suppress
	@echo [Makefile] Done
else
ifneq ($(suppress), true)
	@echo [Makefile] Done
endif
endif

else

ifeq ($(wildcard $(BIN)/.*),)
	@mkdir $(BIN)
endif
ifndef debug
	@$(CC) $(CFLAGS) -I $(INCLUDE) -c src/*.$(EXT)
else
ifeq ($(debug), true)
	@$(CC) $(CFLAGS) -g -I $(INCLUDE) -c src/*.$(EXT)
else
	@$(CC) $(CFLAGS) -I $(INCLUDE) -c src/*.$(EXT)
endif
endif
	@$(CC) *.o $(OTHER) -o $(EXECUTABLE) $(LIBRARIES)
#	for /r %x in (*.o) do $(MV) "%x" "$(BIN)"
	@mv *.o $(BIN)
ifndef suppress
	@echo [Makefile] Done
else
ifneq ($(suppress), true)
	@echo [Makefile] Done
endif
endif

endif

run:
ifneq ($(OS), Windows_NT)
	@./$(EXECUTABLE)
else
	@$(EXECUTABLE)
endif

debug:
	@make build debug=true suppress=true
	@$(DBG) ./$(EXECUTABLE)

setup:
ifneq ($(OS), Windows_NT)

ifeq ($(wildcard $(SOURCE)/.*),)
	@mkdir $(SOURCE)
endif
ifeq ($(wildcard $(INCLUDE)/.*),)
	@mkdir $(INCLUDE)
endif
	@echo [Makefile] Done

else

ifeq ($(wildcard $(SOURCE)/.*),)
	@mkdir $(SOURCE)
endif
ifeq ($(wildcard $(INCLUDE)/.*),)
	@mkdir $(INCLUDE)
endif
	@echo [Makefile] Done

endif

clean:
ifneq ($(OS), Windows_NT)

ifneq ($(wildcard $(BIN)/.*),)
	@$(RM) -r $(BIN)
ifneq ($(wildcard $(EXECUTABLE)),)
	@$(RM) $(EXECUTABLE)
endif
ifneq ($(wildcard .*),)
	@$(RM) *.o
endif
endif
	@echo [Makefile] Done

else

ifneq ($(wildcard $(BIN)/.*),)
	@$(RM) $(BIN)
#	@$(RMDIR) $(BIN) /q >nul
ifneq ($(wildcard $(EXECUTABLE)),)
#	@$(RM) $(EXECUTABLE) /q >nul
	@$(RM) $(EXECUTABLE)
endif
ifneq ($(wildcard .*),)
#	@for /r %%x in (*.o) do $(RM) "%%x" /q >nul
	@rm *.o
endif
endif
	@echo [Makefile] Done

endif

install:
ifneq ($(OS), Windows_NT)

ifeq ($(shell id -u), 0)
############# Install ###############
	@make build suppress=true
	@$(CP) ./$(EXECUTABLE) $(DEST)
	@echo [Makefile] Installed
#####################################
else
	@echo [Makefile] Root required!
endif

else

#	@mkdir "$(DEST)" >nul
	@mkdir "$(DEST)"
	@$(MK) build suppress=true --no-print-directory
#	@$(CP) $(EXECUTABLE) "$(DEST)" >nul
	@$(CP) $(EXECUTABLE) "$(DEST)"
	@echo [Makefile] Installed

endif

remove:
ifneq ($(OS), Windows_NT)

ifeq ($(shell id -u), 0)
ifneq ($(wildcard $(DEST)/$(EXECUTABLE)),)
############# Remove ###############
	@$(RM) $(DEST)/$(EXECUTABLE)
	@echo [Makefile] Done
####################################
else
	@echo [Makefile] Program not installed
endif
else
	@echo [Makefile] Root required!
endif

else

	@$(RM) "$(DEST)" /q >nul
	@$(RMDIR) "$(DEST)" /q >nul
	@$(RM) /f "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Sea Battles.lnk" /q >nul
	@echo [Makefile] Program removed!

endif