CXX=g++
CFLAGS= -O3 -march=native --std=c++11 -I src -ggdb
SRCDIR=src
BINDIR=bin
OBJDIR=obj
BIN=$(BINDIR)/sudoku
SRC=$(SRCDIR)/sudoku.cpp $(SRCDIR)/metasudoku.cpp
OBJ=$(OBJDIR)/sudoku.o $(OBJDIR)/metasudoku.o

all: sudoku

test: sudoku 
	./test.sh

sudoku: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $^ -o $(BIN)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	rm -f $(BIN) $(OBJ)
