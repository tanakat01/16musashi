#include "board.h"
#include "count_reader.h"
#include <fstream>
#include <chrono>
#include <vector>
typedef std::vector<int> vI;
typedef std::vector<vI> vvI;

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

const int SIZE = 33;
const int capture_type = 0;
const size_t BLOCK_SIZE = 0x10000;
vvI ret(SIZE, vI(100));
vvI zero_turn(SIZE, vI(2));

int main(int ac, char **ag) {
  CountReader<SIZE, 0> cr;
  for (size_t i = 0; i < CountReader<SIZE, capture_type>::table_size(); i += BLOCK_SIZE) {
    if (i % 0x100000 == 0) {
      std::cerr << "i=" << i << std::endl;
    }
    char buffer[BLOCK_SIZE];
    size_t read_size = std::min(BLOCK_SIZE, CountReader<SIZE, capture_type>::table_size() - i);
    cr.ifs.read(&buffer[0], read_size);
    for (size_t j = 0; j < read_size; j++) {
      Board<SIZE> b(i + j);
      int c = b.browns_size();
#if 0
      if (c == 0 && j == 100) {
	std:: cerr << "i+j) = " << (i + j) << ",b=" << b << ",b.browns_size() = " << b.browns_size() << std::endl;
      }
#endif
      int t = buffer[j]; 
      ret[c][t]++;
      if (t == 0) {
	int turn = b.turn();
	zero_turn[c][turn]++;
      }
    }
  }
  for (int i = 0; i < SIZE; i++) {
    std::cout << i << "," << zero_turn[i][0] << "," << zero_turn[i][1];
    for (size_t j = 1; j < 70; j++)
      std::cout << "," << ret[i][j];
    std::cout << std::endl;
  }
}
