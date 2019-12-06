#include "board.h"
#include <fstream>
#include <chrono>

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

int main(int ac, char **ag) {
  if (ac < 2) usage(ag[0]);
  int n = atoi(ag[1]), count = (ac > 2 ? atoi(ag[2]) : 0);
  std::ifstream f("count_table.bin", std::ios::binary);
  f.read((char *) count_table, sizeof(count_table));
  f.close();
  for (int i = 0; i < table_size; i++) {
    if (count_table[i] == n) {
      Board25 b(i);
      std::cerr << b << std::endl;
      if (count > 0 && --count ==0) break;
    }
  }
}
