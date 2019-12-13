CC = g++
MAIN = hellomake

SRC_DIR = src
OBJ_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS += -Iinclude
CFLAGS += -std=c++11 -g -lsfml-graphics -lsfml-window -lsfml-system -g
LDFLAGS += -L/includes
LDLIBS += -lm

.PHONY: all clean

all: $(MAIN)

$(MAIN): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
