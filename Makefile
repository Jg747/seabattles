#########################################################
# Project name
NAME := seabattles
APP_NAME := $(NAME)

# Example: cpp, c ...
EXT := cpp

# Compilers: gcc, g++, clang, clang++
CC := g++

# Example: -lncurses -lyaml ...
LIBRARIES := -lncurses

# Linker flags
LFLAGS := 

# Compiler flags
CFLAGS := -Wall -std=c++20

# Debug flags
DBFLAGS := -fstack-protector-all

ifneq ($(OS), Windows_NT)
# Install path (MacOS / Linux)
DEST := /usr/local/bin
# Other compilation extras, leave empty if nothing extra
OTHER := 
# Executable name & extension (or no extension)
EXECUTABLE := $(APP_NAME)
else
# Install path (Windows)
DEST := C:/Program Files/$(NAME)_pgm
# Other compilation extras, leave empty if nothing extra
OTHER := icons/icon.res
# Executable name & extension (or no extension)
EXECUTABLE := $(APP_NAME).exe
endif

INCLUDE := include
SRC := src
BIN := bin
#########################################################

LFLAGS += $(OTHER)
LFLAGS += $(LIBRARIES)
CFLAGS += -I $(INCLUDE)

DEBUG ?= false

SRC_LIST := $(wildcard $(SRC)/*.$(EXT))
OBJECTS := $(patsubst %.$(EXT),$(BIN)/%.o,$(SRC_LIST))
OBJECTS := $(subst $(SRC)/,,$(OBJECTS))

ifneq ($(OS), Windows_NT)
RM := rm -r
else
RM := del /q
endif
RMDIR := rmdir
CP := cp

default: build

build: $(BIN)/$(EXECUTABLE)
	@echo [Makefile] Done

$(BIN)/$(EXECUTABLE): $(OBJECTS) Makefile
ifeq ($(DEBUG), false)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXECUTABLE) $(LFLAGS)
else
	$(CC) $(CFLAGS) $(DBFLAGS) -g $(OBJECTS) -o $(EXECUTABLE) $(LFLAGS)
endif

$(BIN)/%.o: $(SRC)/%.$(EXT) | $(BIN)
ifeq ($(DEBUG), false)
	$(CC) $(CFLAGS) -MMD -MF $@.d -c $< -o $@
else
	$(CC) $(CFLAGS) -MMD -MF $@.d -g -c $< -o $@
endif

$(BIN):
	@mkdir $(BIN)

-include $(BIN)/*.o.d

run:
ifneq ($(OS), Windows_NT)
	./$(EXECUTABLE)
else
	$(EXECUTABLE)
endif

debug: 
	$(MAKE) clean 
	$(MAKE) build DEBUG=true

setup:
ifeq ($(wildcard $(SRC)/.*),)
	mkdir $(SRC)
endif
ifeq ($(wildcard $(INCLUDE)/.*),)
	mkdir $(INCLUDE)
endif
	@echo [Makefile] Done

clean:
	$(RM) $(BIN)
	@echo [Makefile] Done

install:
ifneq ($(OS), Windows_NT)

ifeq ($(shell id -u), 0)
############# Install ###############
	@make build
	$(CP) ./$(EXECUTABLE) $(DEST)
	@echo [Makefile] Installed
############# Install ###############
else
	@echo [Makefile] Root required!
endif

else
############# Install ###############
	@mkdir "$(DEST)"
	@make build --no-print-directory
	$(CP) $(EXECUTABLE) "$(DEST)"
	@echo [Makefile] Installed
############# Install ###############
endif

remove:
ifneq ($(OS), Windows_NT)

ifeq ($(shell id -u), 0)
ifneq ($(wildcard $(DEST)/$(EXECUTABLE)),)
############# Remove ###############
	$(RM) $(DEST)/$(EXECUTABLE)
	@echo [Makefile] Program removed!
############# Remove ###############
else
	@echo [Makefile] Program not installed
endif
else
	@echo [Makefile] Root required!
endif

else
############# Remove ###############
	$(RM) "$(DEST)" /q >nul
	$(RMDIR) "$(DEST)" /q >nul
	@echo [Makefile] Program removed!
############# Remove ###############
endif