#include "board.h"
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

const size_t pos_size = (1ull << 24);
const size_t h_table_size = 15 * pos_size;
const size_t table_size = 15 * pos_size * 2;

/*
  0 - unknown
  1 - (black turn) black cannot move (brown win)
  2 - (brown turn) brown has move to state 1
 2*n - (brown turn) brown has move to state 2*n - 1
 2*n + 1 - (black turn) all black move lead to state 2*m (m <= n) 
 */
constexpr size_t bytesize(uint64_t sz) {
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

std::mutex io_lock;

const int BSIZE = 256;

std::atomic<uint64_t> changed;

void init_worker(int num_workers, int n) {
  uint64_t l_changed = 0;
  size_t s_index = BSIZE * n;
  for (size_t j = s_index; j < h_table_size; j += BSIZE * num_workers) {
    for (size_t i = j; i < std::min(j + BSIZE, h_table_size); i++) {
      if (i % 10000000 == 0) {
        {
          std::lock_guard<std::mutex> l_(io_lock);
          std::cerr << "init : i=" << i << std::endl;
        }
      }
      Board25 b = Board25::from_index(i, Board25::black);
      if (b.turn() == black) {
        if (b.final_value() == 1) {
          set_table(table_black, i);
          l_changed++;
        }
      }
    }
  }
  changed += l_changed;
}


void worker(int step, int num_workers, int n) {
//   std::cerr << "worker(step=" << step << ",num_workers=" << num_workers << ",n=" << n << std::endl;
  uint64_t l_changed = 0;
  size_t s_index = BSIZE * n;
  bool is_black = ((step & 1) == 1);
  if (is_black) {
    for (size_t j = s_index; j < h_table_size; j += BSIZE * num_workers) {
      for (size_t i = j; i < std::min(j + BSIZE, h_table_size); i++) {
        if (i % 10000000 == 0) {
          std::lock_guard<std::mutex> l_(io_lock);
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
          l_changed++;
        }
      }
    }
  } else {
    for (size_t j = s_index; j < h_table_size; j += BSIZE * num_workers) {
      for (size_t i = j; i < std::min(j + BSIZE, h_table_size); i++) {
        if (i % 10000000 == 0) {
          std::lock_guard<std::mutex> l_(io_lock);
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
          l_changed++;
        }
      }
    }
  }
  changed += l_changed;
}


void solve(int num_workers) {
  auto chrono_start = std::chrono::system_clock::now();
  changed = 0;
#if 0
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
#else
  std::vector<std::thread> threads(num_workers);
  for (int i = 0; i < num_workers; i++) {
    threads[i] = std::thread(init_worker, num_workers, i);
  }
  for (int i = 0; i < num_workers; i++) {
    threads[i].join();
  }
#endif
  std::cerr << "changed = " << changed << std::endl;
  {
    std::string fname = "black_" + std::to_string(1) + "s.bin";
    std::ofstream f(fname, std::ios::binary);
    f.write((char *)(table_black), sizeof(table_black));
  f.close();
  }
  for (int step = 2; step < 256; step++) {
    auto chrono_end = std::chrono::system_clock::now();
    std::cerr << "Elapsed time:" << std::chrono::duration_cast<std::chrono::milliseconds>(chrono_end - chrono_start).count() << "[ms]" << std::endl;

    std::cerr << "step=" << step << std::endl;
    changed = 0;
    std::vector<std::thread> threads(num_workers);
    for (int i = 0; i < num_workers; i++) {
      threads[i] =std::thread(worker, step, num_workers, i);
    }
    for (int i = 0; i < num_workers; i++) {
      threads[i].join();
    }
    std::cerr << "changed = " << changed << std::endl;
    if (changed == 0) break;
    if ((step & 1) == 1) {
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
  solve(64);
}
