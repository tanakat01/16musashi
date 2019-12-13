#include "board.h"
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

constexpr size_t bytesize(uint64_t sz) {
  return ((sz + 7) / 8);
}

void set_table(std::vector<uint8_t> &table, uint64_t pos) {
  table[pos / 8] |= (1 << (pos % 8));
}

bool test_table(std::vector<uint8_t> const& table, uint64_t pos) {
  return (table[pos / 8] & (1 << (pos % 8))) != 0;
}

std::mutex io_lock;

/*
 * capter_type : 0 - brown can capture a black piece at anywhere
 *               1 - brown can capture a black piece at one of four corners
 *               2 - brown can capture a black piece at the corner(0, 0)
 *               3 - brown can capture a black piece at (1, 0)
 *               4 - brown can capture a black piece at (2, 0)
 */
template<int SIZE, int capture_type = 0>
class TableMaker {
  static constexpr bool NO_SYMMETRY() {
    if (capture_type == 2 || capture_type == 3 || capture_type == 4) return true;
    else return false;
  }
  static constexpr size_t pos_size() {
    return (1ull << (SIZE - 1));
  }
  static constexpr size_t h_table_size() {
    if (NO_SYMMETRY())
      return pos_size() * SIZE;
    else
      return pos_size() * Board<SIZE>::HSIZE();
  }
  static constexpr size_t table_size() {
    return h_table_size() * 2;
  }

/*
  0 - unknown
  1 - (black turn) black cannot move (brown win)
  2 - (brown turn) brown has move to state 1
 2*n - (brown turn) brown has move to state 2*n - 1
 2*n + 1 - (black turn) all black move lead to state 2*m (m <= n) 
 */

  std::vector<uint8_t> table_brown, table_black;

public:
  TableMaker()
    :table_brown(bytesize(h_table_size()), 0),
     table_black(bytesize(h_table_size()), 0) {
    std::cerr << "TableMaker : constructor(SIZE=" << SIZE << ",capture_type = " << capture_type << std::endl;
  }
  static constexpr int BSIZE() { return 256; }
  std::atomic<uint64_t> changed;
  static void init_worker(TableMaker* tm, int num_workers, int n) {
    uint64_t l_changed = 0;
    size_t s_index = BSIZE() * n;
    for (size_t j = s_index; j < h_table_size(); j += BSIZE() * num_workers) {
      for (size_t i = j; i < std::min(j + BSIZE(), h_table_size()); i++) {
	if (i % 10000000 == 0) {
	  {
	    std::lock_guard<std::mutex> l_(io_lock);
	    std::cerr << "init : i=" << i << std::endl;
	  }
	}
	Board<SIZE> b = Board<SIZE>::from_index(i, Board<SIZE>::black);
	if (b.turn() == Board<SIZE>::black) {
	  uint64_t bpos = b.ppos();
	  if (capture_type == 2 &&
	      bpos != Board<SIZE>::toPos(0, 0)) continue;
	  if (capture_type == 3 &&
	      bpos != Board<SIZE>::toPos(1, 0)) continue;
	  if (capture_type == 4 &&
	      bpos != Board<SIZE>::toPos(2, 0)) continue;
	  if (capture_type == 1 &&
	      bpos != Board<SIZE>::toPos(0, 0) &&
	      bpos != Board<SIZE>::toPos(4, 0) &&
	      bpos != Board<SIZE>::toPos(0, 4) &&
	      bpos != Board<SIZE>::toPos(4, 4)) continue;
	  if (b.final_value() == 1) {
	    set_table(tm->table_black, i);
	    l_changed++;
	  }
	}
      }
    }
    tm->changed += l_changed;
  }


  static void worker(TableMaker *tm, int step, int num_workers, int n) {
    //   std::cerr << "worker(step=" << step << ",num_workers=" << num_workers << ",n=" << n << std::endl;
    uint64_t l_changed = 0;
    size_t s_index = BSIZE() * n;
    bool is_black = ((step & 1) == 1);
    if (is_black) {
      for (size_t j = s_index; j < h_table_size(); j += BSIZE() * num_workers) {
	for (size_t i = j; i < std::min(j + BSIZE(), h_table_size()); i++) {
	  if (i % 10000000 == 0) {
	    std::lock_guard<std::mutex> l_(io_lock);
	    std::cerr << "step=" << step << ", black : i=" << i << std::endl;
	  }
	  if (test_table(tm->table_black, i) != 0) continue;
	  Board<SIZE> b = Board<SIZE>::from_index(i, Board<SIZE>::black);
	  if (b.to_index() != i) {
	    std::cerr << "i=" << i << ",b.to_index()="  << b.to_index() << std::endl;
	    throw std::runtime_error("index error");
	  }
	  // if (b.turn() != black) continue;
	  bool has_escape = false;
	  for (auto n_ : b.next_states()) {
	    Board<SIZE> n(n_);
	    if (!NO_SYMMETRY())
	      if (n.v >= table_size()) n = n.flip();
	    if (!test_table(tm->table_brown, n.to_index())) {
	      has_escape = true; break;
	    }
	  }
	  if (!has_escape) {
	    set_table(tm->table_black, i);
	    l_changed++;
	  }
	}
      }
    } else {
      for (size_t j = s_index; j < h_table_size(); j += BSIZE() * num_workers) {
	for (size_t i = j; i < std::min(j + BSIZE(), h_table_size()); i++) {
	  if (i % 10000000 == 0) {
	    std::lock_guard<std::mutex> l_(io_lock);
	    std::cerr << "step=" << step << ", brown : i=" << i << std::endl;
	  }
	  if (test_table(tm->table_brown, i) != 0) continue;
	  Board<SIZE> b = Board<SIZE>::from_index(i, Board<SIZE>::brown);
	  if (b.to_index() != i) {
	    std::cerr << "i=" << i << ",b.to_index()="  << b.to_index() << std::endl;
	    throw std::runtime_error("index error");
	  }
	  bool has_capture = false;
	  for (auto n_ : b.next_states()) {
	    Board<SIZE> n(n_);
	    if (test_table(tm->table_black, n.to_index())) {
	      has_capture = true; break;
	    }
	  }
	  if (has_capture) {
	    set_table(tm->table_brown, i);
	    l_changed++;
	  }
	}
      }
    }
    tm->changed += l_changed;
  }

  static void write_stream(std::ofstream& os, char* buf, size_t size) {
    size_t BSIZE = 0x1000000;
    for (size_t i = 0ull; i < size; i+= BSIZE) {
      size_t wsize = std::min(size - i, BSIZE);
      os.write(buf + i, wsize);
    }
  }
  void solve(int num_workers) {
    // std::string prefix="/mnt/sda1/ktanaka/";
    std::cerr << "start solving SIZE=" << SIZE << ", num_workers=" << num_workers << std::endl;
    auto chrono_start = std::chrono::system_clock::now();
    changed = 0;
    if (num_workers == 1) {
      init_worker(this, num_workers, 0);
    }
    else {
      std::vector<std::thread> threads(num_workers);
      for (int i = 0; i < num_workers; i++) {
	threads[i] = std::thread(init_worker, this, num_workers, i);
      }
      for (int i = 0; i < num_workers; i++) {
	threads[i].join();
      }
    }
    std::cerr << "changed = " << changed << std::endl;
    {
      std::string fname = "black_" + std::to_string(SIZE) + "_" + std::to_string(capture_type) + "_" + std::to_string(1) + ".bin";
      // fname = prefix + fname;
      std::ofstream f(fname, std::ios::binary);
      // f.write((char *)(&table_black[0]), table_black.size());
      write_stream(f, (char *)(&table_black[0]), table_black.size());
      f.close();
    }
    for (int step = 2; step < 256; step++) {
      auto chrono_end = std::chrono::system_clock::now();
      std::cerr << "Elapsed time:" << std::chrono::duration_cast<std::chrono::milliseconds>(chrono_end - chrono_start).count() << "[ms]" << std::endl;

      std::cerr << "step=" << step << std::endl;
      changed = 0;
      if (num_workers == 1) {
	worker(this, step, num_workers, 0);
      }
      else {
	std::vector<std::thread> threads(num_workers);
	for (int i = 0; i < num_workers; i++) {
	  threads[i] =std::thread(worker, this, step, num_workers, i);
	}
	for (int i = 0; i < num_workers; i++) {
	  threads[i].join();
	}
      }
      std::cerr << "changed = " << changed << std::endl;
      if (changed == 0) break;
      if ((step & 1) == 1) {
	std::string fname = "black_" + std::to_string(SIZE) + "_" + std::to_string(capture_type) + "_" + std::to_string(step) + ".bin";
	// if (step < 5) fname = prefix + fname;
	std::ofstream f(fname, std::ios::binary);
	// f.write((char *)(&table_black[0]), table_black.size());
	write_stream(f, (char *)(&table_black[0]), table_black.size());
	f.close();
      } else {
	std::string fname = "brown_" + std::to_string(SIZE) + "_" + std::to_string(capture_type) + "_" + std::to_string(step) + ".bin";
	//	if (step < 5) fname = prefix + fname;
	std::ofstream f(fname, std::ios::binary);
	// f.write((char *)(&table_brown[0]), table_brown.size());
	write_stream(f, (char *)(&table_brown[0]), table_brown.size());
	f.close();
      }
    }
  }
};

int main(int ac, char **ag) {
  int board_size;
  int capture_type;
  int n_workers;
  
  po::options_description options("all_options");
  options.add_options()
    ("board-size,n",
     po::value<int>(&board_size)->default_value(25),
     "board size (25, 31, 33)")
    ("capture-type,t",
     po::value<int>(&capture_type)->default_value(0),
     "capture type : 0 (ALL), 1(any corner), 2(one corner), 3(1,0), 4(2,0)")
    ("n-workers,w",
     po::value<int>(&n_workers)->default_value(8),
     "number of workers")
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
      TableMaker<25, 0>().solve(n_workers);
    else if (capture_type == 1)
      TableMaker<25, 1>().solve(n_workers);
    else if (capture_type == 2)
      TableMaker<25, 2>().solve(n_workers);
    else if (capture_type == 3)
      TableMaker<25, 3>().solve(n_workers);
    else if (capture_type == 4)
      TableMaker<25, 4>().solve(n_workers);
    else {
      std::cerr << options << std::endl;
      return 0;
    }
  }
  else if (board_size == 33) {
    if (capture_type == 0)
      TableMaker<33, 0>().solve(n_workers);
  }
  else {
    std::cerr << options << std::endl;
    return 0;
  }
}
