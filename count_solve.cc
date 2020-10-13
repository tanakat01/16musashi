#include "board.h"
#include "count_reader.h"
#include <fstream>
#include <chrono>
#include <vector>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
typedef std::vector<int64_t> vL;
typedef std::vector<vL> vvL;

/*
  0 - unknown
  1 - (black turn) black cannot move (brown win)
  2 - (brown turn) brown has move to state 1
 2*n - (brown turn) brown has move to state 2*n - 1
 2*n + 1 - (black turn) all black move lead to state 2*m (m <= n) 
 */

template<int SIZE, int capture_type=0>
class CountSolve {
public:
  vvL ret;
  vvL zero_turn;
  const size_t BLOCK_SIZE = 0x10000;
  CountSolve() :ret(SIZE, vL(100)), zero_turn(SIZE, vL(2)) {}
  void count() {
    uint64_t all = 0ull;
    CountReader<SIZE, 0> cr;
    for (size_t i = 0; i < CountReader<SIZE, capture_type>::table_size(); i += BLOCK_SIZE) {
      if (i % 0x100000000ull == 0) {
	std::cerr << "i=" << i << std::endl;
      }
      char buffer[BLOCK_SIZE];
      size_t read_size = std::min(BLOCK_SIZE, CountReader<SIZE, capture_type>::table_size() - i);
      cr.ifs.read(&buffer[0], read_size);
      for (size_t j = 0; j < read_size; j++) {
	Board<SIZE> b(i + j);
	bool debug_flag = b.browns_size() == 24;
	Board<SIZE> b1 = b.normalize();
	if (debug_flag) {
	  std:: cerr << "b=" << b << ",b1=" << b1 << std::endl;
	  if (b1.v < i + j) {
	    std::cerr << "prune" << std::endl;
	  }
	}
	if (b1.v < i + j) {
	  // std:: cerr << "i+j) = " << (i + j) << ",b=" << b << ",b1=" << b1 << std::endl;
	  continue;
	}
	int c = b.browns_size();
#if 0
	if (c == 0 && j == 100) {
	  std:: cerr << "i+j) = " << (i + j) << ",b=" << b << ",b.browns_size() = " << b.browns_size() << std::endl;
	}
#endif
	int t = buffer[j]; 
	ret[c][t]++;
	all++;
	if (t == 0) {
	  int turn = b.turn();
	  zero_turn[c][turn]++;
	}
      }
    }
    std::cout << "all=" << all << std::endl;
    for (int i = 0; i < SIZE; i++) {
      std::cout << i << "," << zero_turn[i][0] << "," << zero_turn[i][1];
      for (size_t j = 1; j < 70; j++)
	std::cout << "," << ret[i][j];
      std::cout << std::endl;
    }
  }
};


int main(int ac, char **ag) {
  int board_size;
  int capture_type;
  
  po::options_description options("all_options");
  options.add_options()
    ("board-size,n",
     po::value<int>(&board_size)->default_value(25),
     "board size (25, 31, 33)")
    ("capture-type,t",
     po::value<int>(&capture_type)->default_value(0),
     "capture type : 0 (ALL), 1(any corner), 2(one corner), 3(1,0), 4(2,0)")
    ;
  po::variables_map vm;
  try
    {
      po::store(po::parse_command_line(ac, ag, options), vm);
      po::notify(vm);
    }
  catch (std::exception& e)
    {
      std::cerr << "error in parsing options" << std::endl
		<< e.what() << std::endl;
      std::cerr << options << std::endl;
      return 1;
    }
  if (vm.count("help")) {
    std::cerr << options << std::endl;
    return 0;
  }
  if (board_size == 25) {
    if (capture_type == 0)
      CountSolve<25, 0>().count();
    else if (capture_type == 1)
      CountSolve<25, 1>().count();
    else if (capture_type == 2)
      CountSolve<25, 2>().count();
    else if (capture_type == 3)
      CountSolve<25, 3>().count();
    else if (capture_type == 4)
      CountSolve<25, 4>().count();
    else {
      std::cerr << options << std::endl;
      return 0;
    }
  }
  else if (board_size == 31) {
    if (capture_type == 0)
      CountSolve<31, 0>().count();
  }
  else if (board_size == 33) {
    if (capture_type == 0)
      CountSolve<33, 0>().count();
  }
  else {
    std::cerr << options << std::endl;
    return 0;
  }
}
