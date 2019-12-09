#include "board.h"
#include <fstream>
#include <chrono>
#include <thread>

const int pos_size = (1ull << 24);
const int h_table_size = 15 * pos_size;
const int table_size = 15 * pos_size * 2;

/*
  0 - unknown
  1 - (black turn) black cannot move (brown win)
  2 - (brown turn) brown has move to state 1
 2*n - (brown turn) brown has move to state 2*n - 1
 2*n + 1 - (black turn) all black move lead to state 2*m (m <= n) 
 */
constexpr uint64_t bytesize(uint64_t sz) {
  return ((sz + 7) / 8);
}

uint8_t table_brown[bytesize(h_table_size)];
uint8_t table_black[bytesize(h_table_size)];

void set_table(uint8_t *table, int pos) {
  table[pos / 8] |= (1 << (pos % 8));
}

bool test_table(uint8_t *table, int pos) {
  return (table[pos / 8] & (1 << (pos % 8))) != 0;
}

constexpr int black = Board25::black;
constexpr int brown = Board25::brown;

void solve() {
  auto chrono_start = std::chrono::system_clock::now();
  int changed = 0;
  for (int i = 0; i < h_table_size; i++) {
    if (i % 10000000 == 0) {
      std::cerr << "init : i=" << i << std::endl;
    }
    Board25 b = Board25::from_index(i, Board25::black);
    if (b.turn() == black) {
      if (b.final_value() == 1) {
        set_table(table_black, i);
        changed++;
      }
    }
  }
  {
    std::string fname = "black_" + std::to_string(1) + "s.bin";
    std::ofstream f(fname, std::ios::binary);
    f.write((char *)(table_black), sizeof(table_black));
  f.close();
  }
  std::cerr << "changed = " << changed << std::endl;
  for (int step = 2; step < 256; step++) {
    auto chrono_end = std::chrono::system_clock::now();
    std::cerr << "Elapsed time:" << std::chrono::duration_cast<std::chrono::milliseconds>(chrono_end - chrono_start).count() << "[ms]" << std::endl;

    std::cerr << "step=" << step << std::endl;
    changed = 0;
    bool is_black = ((step & 1) == 1);
    if (is_black) {
      for (int i = 0; i < h_table_size; i++) {
        if (i % 10000000 == 0) {
          std::cerr << "step=" << step << ", black : i=" << i << std::endl;
        }
        if (test_table(table_black, i) != 0) continue;
        Board25 b = Board25::from_index(i, Board25::black);
        if (b.to_index() != i) {
          std::cerr << "i=" << i << ",b.to_index()="  << b.to_index() << std::endl;
          throw std::runtime_error("index error");
        }
        // if (b.turn() != black) continue;
        bool has_escape = false;
        for (auto n_ : b.next_states()) {
          Board25 n(n_);
          if (n.v >= table_size) n = n.flip();
          if (!test_table(table_brown, n.to_index())) {
            has_escape = true; break;
          }
        }
        if (!has_escape) {
          set_table(table_black, i);
          changed++;
        }
      }
    } else {
      for (int i = 0; i < h_table_size; i++) {
        if (i % 10000000 == 0) {
          std::cerr << "step=" << step << ", brown : i=" << i << std::endl;
        }
        if (test_table(table_brown, i) != 0) continue;
        Board25 b = Board25::from_index(i, Board25::brown);
        if (b.to_index() != i) {
          std::cerr << "i=" << i << ",b.to_index()="  << b.to_index() << std::endl;
          throw std::runtime_error("index error");
        }
        bool has_capture = false;
        for (auto n_ : b.next_states()) {
          Board25 n(n_);
          if (test_table(table_black, n.to_index())) {
            has_capture = true; break;
          }
        }
        if (has_capture) {
          set_table(table_brown, i);
          changed++;
        }
      }
    }
    std::cerr << "changed = " << changed << std::endl;
    if (changed == 0) break;
    if (is_black) {
      std::string fname = "black_" + std::to_string(step) + "s.bin";
      std::ofstream f(fname, std::ios::binary);
      f.write((char *)(table_black), sizeof(table_black));
      f.close();
    } else {
      std::string fname = "brown_" + std::to_string(step) + "s.bin";
      std::ofstream f(fname, std::ios::binary);
      f.write((char *)(table_brown), sizeof(table_brown));
      f.close();
    }
  }
}

int main() {
  solve();
}
