#include "board.h"
#include <fstream>
#include <chrono>
#include <vector>
typedef std::vector<int> vI;
typedef std::vector<vI> vvI;

const int table_size = 15 * 0x2000000;

/*
  0 - unknown
  1 - (black turn) black cannot move (brown win)
  2 - (brown turn) brown has move to state 1
 2*n - (brown turn) brown has move to state 2*n - 1
 2*n + 1 - (black turn) all black move lead to state 2*m (m <= n) 
 */
uint8_t count_table[table_size];

void usage(char *command) {
  std::cerr << "Usage : " << command << " n count" << std::endl;
  std::cerr << " n - value in the table" << std::endl;
  std::cerr << " [count] - how many samples to show (zero for all) " << std::endl;
  exit(0);
}

vvI ret(25, vI(41));

int main(int ac, char **ag) {
  std::ifstream f("count_table.bin", std::ios::binary);
  f.read((char *) count_table, sizeof(count_table));
  f.close();
  for (int i = 0; i < table_size; i++) {
    Board25 b(i);
    int c = b.browns_size();
    ret[c][count_table[i]]++;
  }
  for (int i = 0; i < 25; i++) {
    std::cout << i;
    for (size_t j = 0; j < 41; j++)
      std::cout << "," << ret[i][j];
    std::cout << std::endl;
  }
}
