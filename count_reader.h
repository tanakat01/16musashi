#include <fstream>

template<int SIZE, int capture_type = 0>
class CountReader {
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
 public:
  static constexpr size_t table_size() {
    return h_table_size() * 2;
  }
  static std::string file_name() {
    std::string fname = "count_" + std::to_string(SIZE) + "_" + std::to_string(capture_type) + ".bin";
    return fname;
  }
  std::ifstream ifs;
  CountReader() :ifs(file_name(), std::ios::binary) {
    std::cerr << "CountReader() file_name() = " << file_name() << std::endl;
  }
  char get(size_t offset) {
    if (offset >= table_size()) {
      // std::cerr << "before : offset =" << offset << std::endl;
      Board<SIZE> b(offset);
      b = b.flip();
      offset = b.v;
      // std::cerr << "before : offset =" << offset << std::endl;
    }
    ifs.seekg(offset, std::ios_base::beg);
    char c;
    ifs.read(&c, 1);
    // std::cerr << "offset=" << offset << ",table_size()="  << table_size() << ",c=" << c << std::endl;
    return c;
  }
};
