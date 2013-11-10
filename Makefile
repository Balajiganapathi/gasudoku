CXX=g++
CFLAGS= -O3 -march=native
SRCDIR=src
BINDIR=bin

sudoku: $(SRCDIR)/sudoku.cpp
	$(CXX) $(CFLAGS) $^ -o $(BINDIR)/$@
