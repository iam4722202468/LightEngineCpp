IDIR = ./includes
CC = g++

CPPFLAGS += -I$(IDIR) -std=c++11 -g -lsfml-graphics -lsfml-window -lsfml-system -g -fopenmp
DEPS = lighting.h client.h consts.h map.h server.h loader.h

ODIR = ./build
CPPDIR = ./src

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = consts.o main.o lighting.o client.o map.o server.o loader.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CPPDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

all: main

main: $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o rm main

