CXX = g++
CXXFLAGS = -Wall -DNDEBUG -O2 -g -std=c++17
TESTLIBS = -lgtest -lgtest_main -lpthread

all : solve test_board show_solve

test_board.o : test_board.cc board.h

test_board : test_board.o
	$(CXX) -o test_board test_board.o $(TESTLIBS)

solve.o : solve.cc board.h

solve : solve.o
	$(CXX) -o $@ $< $(TESTLIBS)

show_solve.o : show_solve.cc board.h

show_solve : show_solve.o
	$(CXX) -o $@ $< $(TESTLIBS)
