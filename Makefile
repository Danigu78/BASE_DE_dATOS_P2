CC = gcc -g
CFLAGS = -Wall -Wextra -pedantic -std=c99
LDLIBS = -lodbc -lcurses -lpanel -lmenu -lform

# recompile if these headers change
HEADERS = odbc.h bpass.h lmenu.h search.h windows.h loop.h

EXE = menu
OBJ = menu.o bpass.o loop.o search.o windows.o odbc.o

all: $(EXE)

# compile all files ending in *.c
%.o: %.c $(HEADERS)
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c -o $@ $<

# link binary
$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDLIBS)

clean:
	rm -f *.o core $(EXE)

run:
	./$(EXE)

valgrind:
	valgrind --leak-check=full ./$(EXE)
