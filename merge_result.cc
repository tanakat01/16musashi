#include "board.h"
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <ios>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

typedef std::vector<char> vC;
typedef std::vector<vC> vvC;

constexpr size_t bytesize(uint64_t sz) {
  return ((sz + 7) / 8);
}

void set_table(std::vector<uint8_t> &table, uint64_t pos) {
  table[pos / 8] |= (1 << (pos % 8));
}

bool test_table(std::vector<uint8_t> const& table, uint64_t pos) {
  return (table[pos / 8] & (1 << (pos % 8))) != 0;
}

class BitReader {
  std::ifstream ifs;
  static constexpr int BUFSIZE() {
    return 0x1000;
  }
  vC buf;
  int i, j;
public:
  BitReader(std::string const& fname) :ifs(fname, std::ios::binary), buf(BUFSIZE()), i(BUFSIZE()), j(0) {
  }
  int read() {
    if (j >= 8) {
      i++;
      j = 0;
    }
    if (i >= 0x1000) {
      ifs.read((char *)&buf[0], BUFSIZE());
      i = 0;
    }
    int r = (buf[i] >> j) & 1;
    j++;
    return r;
  }
};

std::mutex io_lock;

/*
 * capter_type : 0 - brown can capture a black piece at anywhere
 *               1 - brown can capture a black piece at one of four corners
 *               2 - brown can capture a black piece at the corner(0, 0)
 *               3 - brown can capture a black piece at (1, 0)
 *               4 - brown can capture a black piece at (2, 0)
 */
template<int SIZE, int capture_type = 0>
class Merger {
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

//   std::vector<uint8_t> table_brown, table_black;

public:
  Merger() {}

  std::string out_file_name() {
    std::string fname = "count_" + std::to_string(SIZE) + "_" + std::to_string(capture_type) + ".bin";
    return fname;
  }

  std::string file_name(int step) {
    if (step % 2 == 1) {
      std::string fname = "black_" + std::to_string(SIZE) + "_" + std::to_string(capture_type) + "_" + std::to_string(step) + ".bin";
      return fname;
    }
    else {
      std::string fname = "brown_" + std::to_string(SIZE) + "_" + std::to_string(capture_type) + "_" + std::to_string(step) + ".bin";
      return fname;
    }
  }

  bool check_step(int step) {
    std::ifstream ifs(file_name(step));
    return ifs.is_open();
  }
  int step_limit() {
    for (int step = 1; step < 256; step++)
      if (!check_step(step)) return step;
    return 256;
  }
  
  char read_black(std::vector<BitReader> &streams) {
    char r = 0;
    for (size_t i = 0; i < streams.size(); i += 2) {
      int v = streams[i].read();
      if (r == 0 && v > 0) r = i + 1;
    }
    return r;
  }

  char read_brown(std::vector<BitReader> &streams) {
    char r = 0;
    for (size_t i = 1; i < streams.size(); i += 2) {
      int v = streams[i].read();
      if (r == 0 && v > 0) r = i + 1;
    }
    return r;
  }
  
  void merge() {
    // std::string prefix="/mnt/sda1/ktanaka/";
    std::cerr << "start mearging SIZE=" << SIZE << ", capture_type=" << capture_type << std::endl;
    std::ofstream ofs(out_file_name(), std::ios::binary|std::ios::trunc);
    int limit = step_limit();
    std::vector<BitReader> streams;
    for (int step = 1; step < limit; step++) {
      streams.emplace_back(BitReader(file_name(step)));
    }
    for (uint64_t v = 0; v < table_size(); v++ ) {
      if (v % 10000000 == 0) {
	std::lock_guard<std::mutex> l_(io_lock);
	std::cerr << "step=" << v << std::endl;
      }
      Board<SIZE> b(v);
      if (b.turn() == Board<SIZE>::black) {
	ofs << read_black(streams);
      }
      else {
	ofs << read_brown(streams);
      }
    }
  }
};

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
      Merger<25, 0>().merge();
    else if (capture_type == 1)
      Merger<25, 1>().merge();
    else if (capture_type == 2)
      Merger<25, 2>().merge();
    else if (capture_type == 3)
      Merger<25, 3>().merge();
    else if (capture_type == 4)
      Merger<25, 4>().merge();
    else {
      std::cerr << options << std::endl;
      return 0;
    }
  }
  else if (board_size == 31) {
    if (capture_type == 0)
      Merger<31, 0>().merge();
  }
  else if (board_size == 33) {
    if (capture_type == 0)
      Merger<33, 0>().merge();
  }
  else {
    std::cerr << options << std::endl;
    return 0;
  }
}
