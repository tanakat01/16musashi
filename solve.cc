#include "board.h"
#include <fstream>
#include <chrono>

const int pos_size = 0x1000000;
const int h_table_size = 15 * pos_size;

/*
  0 - unknown
  1 - (black turn) black cannot move (brown win)
  2 - (brown turn) brown has move to state 1
 2*n - (brown turn) brown has move to state 2*n - 1
 2*n + 1 - (black turn) all black move lead to state 2*m (m <= n) 
 */
uint8_t table_brown[((h_table_size + 7) / 8) * 8];
uint8_t table_black[((h_table_size + 7) / 8) * 8];

void set_table(uint8_t *table, pos) {
  table[pos / 8] |= (1 << (pos % 8));
}

void test_table(uint8_t *table, pos) {
  return (table[pos / 8] & (1 << (pos % 8))) != 0;
}

constexpr int black = Board25::black;
constexpr int brown = Board25::brown;

int main() {
  auto chrono_start = std::chrono::system_clock::now();
  int changed = 0;
  for (int i = 0; i < h_table_size; i++) {
    if (i % 10000000 == 0) {
      std::cerr << "init : i=" << i << std::endl;
    }
    Board25 b(i);
    if (b.turn() == black) {
      if (b.final_value() == 1) {
	count_table[i] = 1;
        changed++;
      }
    }
  }
  std::cerr << "changed = " << changed << std::endl;
  for (int step = 2; step < 256; step++) {
    auto chrono_end = std::chrono::system_clock::now();
    std::cerr << "Elapsed time:" << std::chrono::duration_cast<std::chrono::milliseconds>(chrono_end - chrono_start).count() << "[ms]" << std::endl;

    std::cerr << "step=" << step << std::endl;
    changed = 0;
    bool is_black = ((step & 1) == 1);
    if (is_black) {
      for (int i = 0; i < table_size; i++) {
	if (i % 10000000 == 0) {
	  std::cerr << "step=" << step << ", black : i=" << i << std::endl;
	}
	if (count_table[i] != 0) continue;
	Board25 b(i);
	if (b.turn() != black) continue;
	bool has_escape = false;
	for (auto n_ : b.next_states()) {
          Board25 n(n_);
	  if (n.v >= table_size) n = n.flip();
	  if (count_table[n.v] == 0) {has_escape = true; break;}
	}
	if (!has_escape){
	  count_table[i] = step;
	  changed++;
	}
      }
    }
    else {
      for (int i = 0; i < table_size; i++) {
	if (i % 10000000 == 0) {
	  std::cerr << "step=" << step << ", brown : i=" << i << std::endl;
	}
	if (count_table[i] != 0) continue;
	Board25 b(i);
	if (b.turn() != brown) continue;
	bool has_capture = false;
	for (auto n_ : b.next_states()) {
          Board25 n(n_);
	  if (count_table[n.v] != 0) {has_capture = true; break;}
	}
	if (has_capture){
	  count_table[i] = step;
	  changed++;
	}
      }
    }
    std::cerr << "changed = " << changed << std::endl;
    if (changed == 0) break;
  }
  std::ofstream f("count_table1.bin", std::ios::binary);
  f.write((char *) count_table, sizeof(count_table));
  f.close();
}
