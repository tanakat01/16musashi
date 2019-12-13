CXX = g++
CXXFLAGS = -Wall -march=native -DNDEBUG -O2 -g -std=c++17
# CXXFLAGS = -Wall -march=native -O0 -g -std=c++17
TESTLIBS = -lgtest -lgtest_main -lpthread -lboost_program_options

all : solve test_board show_solve count_solve board_value merge_result

test_board.o : test_board.cc board.h

test_board : test_board.o
	$(CXX) -o test_board test_board.o $(TESTLIBS)

test_board33.o : test_board33.cc board.h

test_board33 : test_board33.o
	$(CXX) -o test_board33 test_board33.o $(TESTLIBS)

test_board31.o : test_board31.cc board.h

test_board31 : test_board31.o
	$(CXX) -o test_board31 test_board31.o $(TESTLIBS)

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

merge_result.o : merge_result.cc board.h

merge_result : merge_result.o
	$(CXX) -o $@ $< $(TESTLIBS)


compare_result.o : compare_result.cc board.h

compare_result : compare_result.o
	$(CXX) -o $@ $< $(TESTLIBS)
