#########################################################
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
# Executable name & extension (or no extension)
EXECUTABLE := test
else
# Executable name & extension (or no extension)
EXECUTABLE := test.exe
endif

INCLUDE := ../include
SRC := ../src
BIN := bin
#########################################################

LFLAGS += $(OTHER)
LFLAGS += $(LIBRARIES)
CFLAGS += -I $(INCLUDE)

DEBUG ?= true

SRC_LIST := $(filter-out $(SRC)/main.$(EXT),$(wildcard $(SRC)/*.$(EXT)))
OBJECTS := $(patsubst %.$(EXT),$(BIN)/%.o,$(SRC_LIST))
OBJECTS := $(subst $(SRC)/,,$(OBJECTS))

ifneq ($(OS), Windows_NT)
RM := rm -r
else
RM := del
endif
RMDIR := rmdir
CP := cp

default: build

build: $(BIN)/$(EXECUTABLE)
	@echo [Makefile] Done

$(BIN)/$(EXECUTABLE): $(OBJECTS) Makefile
ifeq ($(DEBUG), false)
	$(CC) $(CFLAGS) -MMD -MF $(BIN)/main.o.d -c main.cpp -o $(BIN)/main.o
	$(CC) $(CFLAGS) $(OBJECTS) $(BIN)/main.o -o $(EXECUTABLE) $(LFLAGS)
else
	$(CC) $(CFLAGS) -MMD -MF $(BIN)/main.o.d -g -c main.cpp -o $(BIN)/main.o
	$(CC) $(CFLAGS) $(DBFLAGS) -g $(OBJECTS) $(BIN)/main.o -o $(EXECUTABLE) $(LFLAGS)
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

clean:
	$(RM) $(BIN)
	@echo [Makefile] Done