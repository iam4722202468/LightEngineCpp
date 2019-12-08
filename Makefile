IDIR = ./includes
CC = g++

CPPFLAGS += -I$(IDIR) -std=c++11 -g -lsfml-graphics -lsfml-window -lsfml-system -g
DEPS = lighting.h

ODIR = ./build
CPPDIR = ./src

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lighting.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(CPPDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

all: main

main: $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o rm main

