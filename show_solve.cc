#include "board.h"
#include <fstream>
#include <chrono>
#include <boost/program_options.hpp>
#include "count_reader.h"
namespace po = boost::program_options;

/*
  0 - unknown
  1 - (black turn) black cannot move (brown win)
  2 - (brown turn) brown has move to state 1
 2*n - (brown turn) brown has move to state 2*n - 1
 2*n + 1 - (black turn) all black move lead to state 2*m (m <= n) 
 */

void usage(char *command) {
  std::cerr << "Usage : " << command << " n count" << std::endl;
  std::cerr << " n - value in the table" << std::endl;
  std::cerr << " stones - number of brown pieces "<< std::endl;
  std::cerr << " [count] - how many samples to show (zero for all) " << std::endl;
  exit(0);
}

template<int SIZE, int capture_type>
void show_solve(int number_of_moves, int number_of_stones, int print_count) {
  const size_t BLOCK_SIZE = 0x10000;
  int count = 0;
  CountReader<SIZE, capture_type> cr;
  size_t offset = 0;
  cr.ifs.seekg(offset, std::ios_base::beg);
  for (size_t i = offset; i < CountReader<SIZE, capture_type>::table_size(); i += BLOCK_SIZE) {
    if (i % 0x100000 == 0) {
      std::cerr << "i=" << i << std::endl;
    }
    char buffer[BLOCK_SIZE];
    size_t read_size = std::min(BLOCK_SIZE, CountReader<SIZE, capture_type>::table_size() - i);
    cr.ifs.read(&buffer[0], read_size);
    for (size_t j = 0; j < read_size; j++) {
      int t = buffer[j]; 
      if (number_of_moves >= 0 && t != number_of_moves) continue;
      Board<SIZE> b(i + j);
      if (t == 0 && b.turn() != Board<SIZE>::brown) continue;
      int c = b.browns_size();
      if (number_of_stones >= 0 && number_of_stones != c) continue;
      std::cout << "---\n" << t << "\nb.v=" << (i+j) << ",b=" << b << std::endl;
      count++;
      if (print_count > 0 && count >= print_count) return; 
    }
  }
}

int main(int ac, char **ag) {
  int board_size;
  int capture_type;
  int number_of_moves;
  int number_of_stones; 
  int print_count;
  
  po::options_description options("all_options");
  options.add_options()
    ("board-size,n",
     po::value<int>(&board_size)->default_value(25),
     "board size (25, 31, 33)")
    ("capture-type,t",
     po::value<int>(&capture_type)->default_value(0),
     "capture type : 0 (ALL), 1(any corner), 2(one corner), 3(1,0), 4(2,0)")
    ("number-of-moves,m",
     po::value<int>(&number_of_moves)->default_value(-1),
     "select states whose move counts matches the number")
    ("number-of-stones,s",
     po::value<int>(&number_of_stones)->default_value(-1),
     "select states which has the specifined number of brown pieces")
    ("print-count,c",
     po::value<int>(&print_count)->default_value(-1),
     "how many states must be printed")
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
      show_solve<25, 0>(number_of_moves, number_of_stones, print_count);
    else if (capture_type == 1)
      show_solve<25, 1>(number_of_moves, number_of_stones, print_count);
    else if (capture_type == 2)
      show_solve<25, 2>(number_of_moves, number_of_stones, print_count);
    else if (capture_type == 3)
      show_solve<25, 3>(number_of_moves, number_of_stones, print_count);
    else if (capture_type == 4)
      show_solve<25, 4>(number_of_moves, number_of_stones, print_count);
    else {
      std::cerr << options << std::endl;
      return 0;
    }
  }
  else if (board_size == 33) {
    if (capture_type == 0)
      show_solve<33, 0>(number_of_moves, number_of_stones, print_count);
  }
  else {
    std::cerr << options << std::endl;
    return 0;
  }
}

