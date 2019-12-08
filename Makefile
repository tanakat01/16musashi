CXX = g++
CXXFLAGS = -Wall -march=native -DNDEBUG -O2 -g -std=c++17
TESTLIBS = -lgtest -lgtest_main -lpthread

all : solve test_board show_solve count_solve board_value

test_board.o : test_board.cc board.h

test_board : test_board.o
	$(CXX) -o test_board test_board.o $(TESTLIBS)

solve.o : solve.cc board.h

solve : solve.o
	$(CXX) -o $@ $< $(TESTLIBS)

show_solve.o : show_solve.cc board.h

show_solve : show_solve.o
	$(CXX) -o $@ $< $(TESTLIBS)

count_solve.o : count_solve.cc board.h

count_solve : count_solve.o
	$(CXX) -o $@ $< $(TESTLIBS)

board_value.o : board_value.cc board.h

board_value : board_value.o
	$(CXX) -o $@ $< $(TESTLIBS)
