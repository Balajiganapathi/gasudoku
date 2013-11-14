CXX=g++
CFLAGS= -O3 -march=native --std=c++11
SRCDIR=src
BINDIR=bin
BIN=$(BINDIR)/sudoku
BIN11=$(BINDIR)/sudoku11

all: test sudoku

test: sudoku 
	./test.sh

sudoku: $(BIN)

$(BIN): $(SRCDIR)/sudoku.cpp $(SRCDIR)/common.h
	$(CXX) $(CFLAGS) $< -o $(BIN)

.PHONY: clean

clean:
	rm -f $(BIN) $(BIN11)
