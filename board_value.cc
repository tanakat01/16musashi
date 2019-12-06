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

void show_board(Board25 const& b) {
  std::cerr << b << std::endl;
  Board25 b1 = b;
  if (b.v >= table_size) b1 = b.flip();
  int c = count_table[b1.v];
  std::cerr << c << std::endl;
  if (c <= 1) return;
  std::cerr << "----" << std::endl;
  auto ns = b.next_states();
  Board25 best_n = b;
  int loss_count = 0;
  for (auto n : ns) {
    Board25 n1 = n;
    if (n.v >= table_size) n1 = n.flip();
    if (count_table[n1.v] == 0) loss_count++;
    if (count_table[n1.v] == c - 1) {
      best_n = n;
    }
  }
  std::cerr << "lost / all = " << loss_count << " / " << ns.size() << std::endl;
  if (!(best_n == b))
    show_board(best_n);
}

int main(int ac, char **ag) {
  std::ifstream f("count_table.bin", std::ios::binary);
  f.read((char *) count_table, sizeof(count_table));
  f.close();
  std::string s_all;
  std::string s;
  std::cerr << "Ready" << std::endl;
  while (std::cin >> s) {
    s_all += s;
    if (s_all.size() >= 26) {
      Board25 b(s_all.substr(0, 26));
      show_board(b);
      s_all = s_all.substr(26, s_all.size());
    }
  }
}
