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

constexpr int black = Board25::black;
constexpr int brown = Board25::brown;

int main() {
  std::ofstream f("count_table1.bin", std::ios::binary);
  f.write((char *) count_table, sizeof(count_table));
  f.close();
}
