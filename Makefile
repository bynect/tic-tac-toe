CC=gcc
CFLAGS=-O2 -g3 -Wall -Werror -DDEBUG
CLIBS=-lSDL2 -lSDL2_image

SRC=main.c
OBJ=main.o
EXE=tic-tac-toe.bin

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CLIBS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ) $(EXE)
