#include "board.h"
#include "count_reader.h"
#include <fstream>
#include <chrono>
#include <boost/program_options.hpp>
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
  std::cerr << " [count] - how many samples to show (zero for all) " << std::endl;
  exit(0);
}

template<int SIZE, int capture_type>
void show_board(CountReader<SIZE, capture_type> &cr, Board<SIZE> const& b) {
  // const size_t table_size = (1ull << SIZE) * (capture_type > 1 ? SIZE : Board<SIZE>::HSIZE());
  std::cerr << "b.v=" << b.v << "\n" << b << std::endl;
  Board<SIZE> b1 = b;
  if (b.v >= CountReader<SIZE, capture_type>::table_size()) b1 = b.flip(); 
  std::cerr << "b1.v=" << b1.v << "\n" << b1 << std::endl;
  int c = cr.get(b1.v);
  std::cerr << c << std::endl;
  if (c <= 1) return;
  std::cerr << "----" << std::endl;
  auto ns = b.next_states();
  Board<SIZE> best_n = b;
  int loss_count = 0;
  int ns_size = 0;
  for (auto n : ns) {
    std::cerr << "n=" << n << ",cr.get(n)=" << int(cr.get(n)) << std::endl;
    ns_size++;
    Board<SIZE> n1(n);
    std::cerr << "n1=" << n1 << std::endl;
    //    if (n1.v >= table_size) n1 = n1.flip();
    if (cr.get(n1.v) == 0) loss_count++;
    if (cr.get(n1.v) == c - 1) {
      best_n = Board<SIZE>(n);
    }
  }
  std::cerr << "lost / all = " << loss_count << " / " << ns_size << std::endl;
  if (!(best_n == b))
    show_board<SIZE, capture_type>(cr, best_n);
}

template<int SIZE, int capture_type>
void show() {
  const int SSIZE = Board<SIZE>::WIDTH() * Board<SIZE>::HEIGHT() + 1;
  CountReader<SIZE, capture_type> cr;
  std::string s_all;
  std::string s;
  std::cerr << "Ready" << std::endl;
  while (std::cin >> s) {
    s_all += s;
    if (s_all.size() >= SSIZE) {
      Board<SIZE> b(s_all.substr(0, SSIZE));
      show_board<SIZE, capture_type>(cr,b);
      s_all = s_all.substr(SSIZE, s_all.size());
    }
  }
}

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
      show<25, 0>();
    else if (capture_type == 1)
      show<25, 1>();
    else if (capture_type == 2)
      show<25, 2>();
    else if (capture_type == 3)
      show<25, 3>();
    else if (capture_type == 4)
      show<25, 4>();
    else {
      std::cerr << options << std::endl;
      return 0;
    }
  }
  else if (board_size == 33) {
    if (capture_type == 0)
      show<33, 0>();
  }
  else {
    std::cerr << options << std::endl;
    return 0;
  }
    

}
