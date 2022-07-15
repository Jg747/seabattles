# Project name
NAME = seabattle

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

RM = rm
MV = mv
CP = cp

ifeq ($(OS), Windows_NT)
# Install path (Windows)
	DEST = C:\\Programs\\$(NAME)
# Other compilation extras, leave empty if nothing extra
	OTHER = 
# Executable name & extension
	EXECUTABLE = $(NAME).exe
#########################################################
	MK = mingw32-make
	DBG = gdb
else
# Install path (MacOS / Linux)
	DEST = /usr/local/bin
# Other compilation extras, leave empty if nothing extra
	OTHER = 
# Executable name & extension (or no extension)
	EXECUTABLE = $(NAME)
#########################################################
	MK = make
ifeq ($(shell uname -s), Darwin)
	DBG = lldb
else
	DBG = gdb
endif
endif

default: build

build:
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
	@$(CC) *.o $(LIBRARIES) $(OTHER) -o $(EXECUTABLE)
	@$(MV) *.o $(BIN)
ifndef suppress
	@echo [Makefile] Done
else
ifneq ($(suppress), true)
	@echo [Makefile] Done
endif
endif

run:
ifeq ($(OS), Windows_NT)
	@$(EXECUTABLE)
else
	@./$(EXECUTABLE)
endif

debug:
	@make build debug=true suppress=true
ifeq ($(OS), Windows_NT)
	@$(DBG) $(EXECUTABLE)
else
	@$(DBG) ./$(EXECUTABLE)
endif

setup:
ifeq ($(wildcard $(SOURCE)/.*),)
	@mkdir $(SOURCE)
endif
ifeq ($(wildcard $(INCLUDE)/.*),)
	@mkdir $(INCLUDE)
endif
	@echo [Makefile] Done

clean:
ifneq ($(wildcard $(BIN)/.*),)
ifeq ($(OS), Windows_NT)
	@$(RM) $(BIN)/* /q
	@$(RMDIR) $(BIN) /q
else
	@$(RM) -r $(BIN)
endif
ifneq ($(wildcard $(EXECUTABLE)),)
	@$(RM) $(EXECUTABLE)
endif
ifneq ($(wildcard .*),)
	@$(RM) *.o
endif
endif
	@echo [Makefile] Done

install:
ifeq ($(shell id -u), 0)
############# Install ###############
	@make build suppress=true
ifeq ($(OS), Windows_NT)
# Windows
ifdef cmd
# Add to cmd
endif
# Installation in programs
else
# MacOS & Linux
	@$(CP) ./$(EXECUTABLE) $(DEST)
endif
	@echo [Makefile] Installed
#####################################
else
	@echo [Makefile] Root/Administrator required!
endif

remove:
ifeq ($(shell id -u), 0)
ifneq ($(wildcard $(DEST)/$(EXECUTABLE)),)
############# Remove ###############
ifeq ($(OS), Windows_NT)
	@$(RM) $(DEST) /q
else
	@$(RM) $(DEST)/$(EXECUTABLE)
endif
	@echo [Makefile] Done
####################################
else
	@echo [Makefile] Program not installed
endif
else
	@echo [Makefile] Root/Administrator required!
endif