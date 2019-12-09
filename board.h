
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <utility>
#include <x86intrin.h>
#include <cassert>

typedef std::vector<int> vI;
typedef std::pair<int, int> II;


static inline uint64_t popcnt(uint64_t n)
{
    return(_popcnt64(n));
}

static inline int bsf(uint64_t mask)
{
  assert(mask);
  uint64_t ret;
  __asm__("bsfq %1,%0" : "=r"(ret) : "r"(mask));
  return static_cast<int>(ret);
}

class PointSetIterator {
  uint64_t v;
public:
  explicit PointSetIterator(uint64_t v_) :v(v_) {}
  int operator*() const noexcept {
    return bsf(v);
  }
  PointSetIterator operator++() noexcept {
    v &= (v - 1);
    return *this;
  }
  PointSetIterator operator++(int) noexcept {
    PointSetIterator old = *this;
    v &= (v - 1);
    return old;
  }
  bool empty() const noexcept {
    return v == 0;
  }
  uint64_t val() const noexcept {
    return v;
  }
  PointSetIterator& operator=(PointSetIterator const& a) {
    v = a.v; 
    return *this;
  }
  friend std::ostream& operator<<(std::ostream& os, PointSetIterator const& a);
};
std::ostream& operator<<(std::ostream& os, PointSetIterator const& a) {
  return os << a.v;
}

static inline bool operator==(PointSetIterator const& x,
                                 PointSetIterator const& y) {
  return x.val() == y.val();
}

static inline bool operator!=(PointSetIterator const& x,
                                 PointSetIterator const& y) {
  return x.val() != y.val();
}

class PointSet {
  uint64_t v;
public:
  typedef PointSetIterator iterator;
  explicit PointSet(uint64_t v_) :v(v_) {}
  explicit PointSet() :PointSet(0) {}
  explicit PointSet(vI const& vs) :v(0) {
    for (auto p : vs) v |= (1ull << p);
  }
  PointSetIterator begin() const {
    return PointSetIterator(v);
  }
  PointSetIterator end() const {
    return PointSetIterator(0);
  }
  bool empty() const {
    return v == 0;
  }
  int size() const {
    return popcnt(v);
  }
  bool test(int pos) const {
    return (v & (1ull << pos)) != 0;
  }
  uint64_t val() const noexcept {
    return v;
  }
};

static inline PointSet operator~(PointSet const& ps) {
  return PointSet(~ps.val());
}

static inline PointSet operator&(PointSet const& ps1, PointSet const& ps2) {
  return PointSet(ps1.val() & ps2.val());
}

static inline PointSet operator|(PointSet const& ps1, PointSet const& ps2) {
  return PointSet(ps1.val() | ps2.val());
}

/*
 additional 8 point
 bits
 14 15 16 17 18    19
  7  8  9 10 11 12 13
  0  1  2  3  4  5  6
 20 21 22 23 24 25 26
 27 28 29 30 31    32
 */

/*
 simple 5 x 5 borrd 
 bits
 10 11 12 13 14
  5  6  7  8  9
  0  1  2  3  4
 15 16 17 18 19
 20 21 22 23 24
 black position (bpos) 
 25-29
 turn (bit bpos) 0 : black(X) to move, 1 : brown(o) to move 
 string representation
 "ooooo"
 "o...o"
 "o.X.o"
 "o...o"
 "oooooo"
 v : 0 - 25 * 2 ** 25
 */
static constexpr II n4[4] = {II(1, 0), II(0, 1), II(-1, 0), II(0, -1)};
static constexpr II n8[8] = {II(1, 0), II(0, 1), II(-1, 0), II(0, -1),
			     II(1, 1), II(-1, 1), II(-1, -1), II(1, -1)};
static constexpr II h4[2] = {II(1, 0), II(0, 1)};
static constexpr II h8[4] = {II(1, 0), II(0, 1), II(1, 1), II(-1, 1)};

class Board25_iterator {
public:
  uint64_t b_v;
  int from;
  PointSetIterator ps0;
  PointSetIterator ps1;
  Board25_iterator(uint64_t b_v_, uint64_t ps0_, uint64_t ps1_) :b_v(b_v_), ps0(ps0_), ps1(ps1_) {} 
  explicit Board25_iterator(uint64_t b_v_) ;
  uint64_t operator*() noexcept;
  Board25_iterator operator++() noexcept;
  Board25_iterator operator++(int) noexcept {
    Board25_iterator old = *this;
    ++(*this);
    return old;
  }
  friend bool operator!=(Board25_iterator const& a, Board25_iterator const& b);
};


class Board25 {
  static constexpr int yoffsets[5] = {10, 5, 0, 15, 20};
public:
  const static uint64_t brown_size = (1ull << 25);
  enum {
    black = 0,
    brown = 1};
  uint64_t v;
  Board25(uint64_t v_) :v(v_){}
  Board25(std::string const& s):v(0u) {
    uint64_t r = 0u;
    int turn = (s[25] == 'X' ? black : brown);
    int bpos = -1;
    for (int y = 0; y < 5; y++) {
      int base = yoffsets[y];
      for (int x = 0; x < 5; x++) {
	char c = s[y * 5 + x];
	if (c == 'o') r |= 1u << (base + x);
	else if (c == 'X') {
	  if (bpos != -1) throw std::runtime_error("too many black pieces");
	  bpos = base + x;
	}
      } 	      
    }           
    v = (turn << bpos) | r | (bpos << 25);
  }
  static Board25 from_index(uint64_t index, int turn) {
    int bpos = index >> 24;
    uint64_t v = (index & ((1ull << bpos) - 1));
    v |= (index & ((1ull << 24) - 1) & ~((1ull << bpos) - 1)) << 1;
    return Board25((turn << bpos) | v | (bpos << 25));
  }
  uint64_t to_index() const {
    uint64_t r = browns().val();
    int bpos = ppos();
    r = (r & ((1ull << bpos) - 1)) | ((r & ((1ull << 25) - (1ull << bpos))) >> 1);   
    return r | (bpos << 24);
  }
  static constexpr int flip_pos(int pos) {
    if (pos < 5) return pos;
    else if (pos < 15) return pos + 10;
    else return pos - 10;
  }
  Board25 flip() const {
    int bpos = (v >> 25) & 31;
    int turn = (v >> bpos) & 1;
    uint64_t v_ = v & ~(1u << bpos);
    uint64_t n = (v_ & 0b11111) + 
      ((v_ & 0b11111'11111'00000) << 10) +
      ((v_ & 0b11111'11111'00000'00000'00000) >> 10);
    int bpos1 = flip_pos(bpos);
    n |= (bpos1 << 25) | (turn << bpos1);
    return Board25(n);
  }     
  void flip_turn() {
    int bpos = (v >> 25) & 31;
    // fprintf(stderr, "flip_turn(v = 0x%x, bpos = %d)", v, bpos);   
    v ^= (1 << bpos);
    //  fprintf(stderr, " => v = 0x%x\n", v);   
  }
  void set_ppos(int ppos) {
    int bpos = (v >> 25) & 31;
    int turn = (v >> bpos) & 1; 
    v &= ~(1 << bpos) & ((1 << 25) - 1);
    v |= (ppos << 25) | (turn << ppos);
  }
  int turn() const {
    int bpos = (v >> 25) & 31;
    return (v >> bpos) & 1;
  }
  int ppos() const {
    return (v >> 25) & 31;
  }
  static II XY(int bi) {
    int x = bi % 5;
    int y = std::find(&yoffsets[0], &yoffsets[5], (bi / 5) * 5) - &yoffsets[0];
    return II(x, y);
  }
  static bool isOnboard(int x, int y) {
    return 0 <= x && x < 5 && 0 <= y && y < 5;
  }
  static bool hasN8(int x, int y) {
    return ((x + y) & 1) == 0;
  }
  static int toPos(int x, int y) {
    return yoffsets[y] + x;
  }
  II pXY() const {
    return XY(ppos());
  }
  char get(int x, int y) const {
    II pxy = pXY();
    if (II(x, y) == pxy) return 'X';
    if ((v & (1 << toPos(x, y))) != 0) return 'o';
    return '.';
  }
  char get(int pos) const {
    if (pos == ppos()) return 'X';
    if ((v & (1 << pos)) != 0) return 'o';
    return '.';
  }
  char get_slow(int x, int y) const {
    if (!isOnboard(x, y)) return ' ';
    II pxy = pXY();
    if (II(x, y) == pxy) return 'X';
    if ((v & (1 << toPos(x, y))) != 0) return 'o';
    return '.';
  }
  static vI neighbors_slow(int pos) {
    vI r;
    auto [x, y] = XY(pos);
    if (hasN8(x, y)) {
      for (auto [dx, dy] : n8) {
	int x1 = x + dx, y1 = y + dy;
	if (isOnboard(x1, y1)) r.push_back(toPos(x1, y1));
      }
    }
    else {
      for (auto [dx, dy] : n4) {
	int x1 = x + dx, y1 = y + dy;
	if (isOnboard(x1, y1)) r.push_back(toPos(x1, y1));
      }
    }
    return r;
  }
  static PointSet neighbors(int pos);
  void remove_browns_slow(int pos) {
    auto [x, y] = XY(pos);
    if (hasN8(x, y)) {
      for (auto [dx, dy] : h8) {
	int x1 = x + dx, y1 = y + dy;
	int x2 = x - dx, y2 = y - dy;
	if (get_slow(x1, y1) == 'o' && get_slow(x2, y2) == 'o') {
	  int pos1 = toPos(x1, y1), pos2 = toPos(x2, y2);
	  v ^= ((1 << pos1) | (1 << pos2));
	}
      }
    } else {
      for (auto [dx, dy] : h4) {
	int x1 = x + dx, y1 = y + dy;
	int x2 = x - dx, y2 = y - dy;
	if (get_slow(x1, y1) == 'o' && get_slow(x2, y2) == 'o') {
	  int pos1 = toPos(x1, y1), pos2 = toPos(x2, y2);
	  v ^= ((1 << pos1) | (1 << pos2));
	}
      }
    }
  }
  void remove_browns(int pos);

  vI movable_slow(int from_pos) const {
    vI r;
    for (int pos : neighbors(from_pos)) {
      if (get(pos) == '.') r.push_back(pos);
    }
    return r;
  }
  PointSet movable(int from_pos) const;
  int movable_size(int from_pos) const {
    return movable(from_pos).size();
  }

  vI browns_slow() const {
    vI r;
    for (int i = 0; i < 25; i++) {
      if (i != ppos() && ((1 << i) & v) != 0) r.push_back(i); 
    }
    return r;
  }
  PointSet browns() const;
  PointSet pieces() const;

  int browns_size() const;
  std::vector<Board25> next_states_slow(bool debug=false) const {
    std::vector<Board25> r;
    if (debug) std::cerr << "turn()=" << turn() << std::endl;
    if (turn() == black) {
      int ppos_ = ppos();
      if (debug) std::cerr << "ppos_=" << ppos_ << std::endl;
      for (int pos : movable(ppos_)) {
	if (debug) std::cerr << "pos=" << pos << std::endl;
	Board25 next = *this;
	next.set_ppos(pos);
	next.remove_browns(pos);
	next.flip_turn();
	r.push_back(next);
      }
    }
    else {
      for (int fPos : browns()) {
	if (debug) std::cerr << "fPos=" << fPos << std::endl;
	for (int tPos : movable(fPos)) {
	  if (debug) std::cerr << "tPos=" << tPos << std::endl;
	  Board25 next = *this;
	  next.v ^= ((1 << fPos) | (1 << tPos));
	  // fprintf(stderr, "before flip= 0x%x",  next.v);
	  next.flip_turn();
	  // fprintf(stderr, "after  flip= 0x%x", next.v);
	  r.push_back(next);
	}
      }
    }
    return r;
  }
  Board25 next_states(bool debug=false) const {
    // std::cerr << "next_states()" << std::endl;
    return *this;
  }
  std::vector<Board25> next_states_v(bool debug=false) const;
  Board25_iterator begin() const;
  Board25_iterator end() const;
  /**
   * 1 : win by brown
   * 0 : not known
   * -1 : win by black
   */
  int final_value() const {
    if (turn() == black) {
      int ppos_ = ppos();
      if (movable_size(ppos_) == 0) return 1;
      return 0;
    }
    else {
      //      if (browns().size() <= 6) return -1;
      return 0;
    }
  }
  std::string to_s() const {
    std::string r;
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 5; x++) {
	r += get(x, y);
      }
    }
    r += (turn() != black ? 'o' : 'X');
    return r;
  }
};

class Board25Table {
  PointSet neighbors_[25];
public:
  PointSet c2[25][4];
  Board25Table();
  PointSet neighbors(int pos) const {
    assert(0 <= pos && pos < 25);
    return neighbors_[pos];
  }
};
Board25Table::Board25Table() {
  for (int pos = 0; pos < 25; pos++)
    neighbors_[pos] = PointSet(Board25::neighbors_slow(pos));
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 5; x++) {
      int pos = Board25::toPos(x, y);
      std::fill(&c2[pos][0], &c2[pos][4], PointSet());
      int i = 0;
      if (Board25::hasN8(x, y)) {
        for (auto [dx, dy] : h8) {
          int x1 = x + dx, y1 = y + dy, pos1 = Board25::toPos(x1, y1);
          int x2 = x - dx, y2 = y - dy, pos2 = Board25::toPos(x2, y2);
          if (Board25::isOnboard(x1, y1) && Board25::isOnboard(x2, y2)) { 
            c2[pos][i++] = PointSet(vI({pos1, pos2}));
#if 0
            if (pos == 8) {
              std::cerr << "pos=" << pos << ",x1=" << x1 << ",y1=" << y1 << ",x2=" << x2 << ",y2=" << y2 << ",pos1=" << pos1 << ",pos2 = " << pos2 << ",ps=" << c2[pos][i - 1].val() << std::endl;
            }
#endif
          }
        }
      } else {
        for (auto [dx, dy] : h4) {
          int x1 = x + dx, y1 = y + dy, pos1 = Board25::toPos(x1, y1);
          int x2 = x - dx, y2 = y - dy, pos2 = Board25::toPos(x2, y2);
          if (Board25::isOnboard(x1, y1) && Board25::isOnboard(x2, y2)) 
            c2[pos][i++] = PointSet(vI({pos1, pos2}));
        }
      }
    }
}

Board25Table board25Table;

inline PointSet Board25::neighbors(int pos) {
  return board25Table.neighbors(pos);
}

inline void Board25::remove_browns(int pos) {
//  std::cerr << "remove_browns(pos=" << pos << ")" << std::endl;
  for (int i = 0; i < 4; i++) {
    uint64_t c2 = board25Table.c2[pos][i].val();
//    std::cerr << "remove_browns(pos=" << pos << ",c2=" << c2 << ")" << std::endl;
    if (c2 == 0ull) break;
//    std::cerr << "remove_browns(pos=" << pos << ",v & c2=" << (v & c2) << ")" << std::endl;
    if ((v & c2) == c2) {
//      std::cerr << "before remove " << v << std::endl;
      v &= ~c2;
//      std::cerr << "after remove " << v << std::endl;
    }
  }
}

inline PointSet Board25::browns() const {
  int bpos = (v >> 25) & 31;
  return PointSet((v & ~(1ull << bpos)) & ((1ull << 25) - 1));
  
}

inline PointSet Board25::pieces() const {
  int bpos = (v >> 25) & 31;
  return PointSet((v | (1ull << bpos)) & ((1ull << 25) - 1));
}

inline int Board25::browns_size() const {
  return popcnt(browns().val());
}

PointSet Board25::movable(int pos) const {
  return board25Table.neighbors(pos) & ~pieces();
}

Board25_iterator::Board25_iterator(uint64_t b_v_) :b_v(b_v_), ps0(0), ps1(0) {
  Board25 b(b_v);
  if (b.turn() == Board25::brown) {
    ps0 = b.browns().begin();
    while (ps1.empty() && !ps0.empty()) {
      from = *ps0;
      ++ps0;
      ps1 = Board25(b_v).movable(from).begin();
    }
  } else {
    from = b.ppos();
    ps1 = b.movable(from).begin();
  }
//  std::cerr << "construct ps0=" << ps0 << ",ps1=" << ps1 << std::endl;
}

bool operator!=(Board25_iterator const& a, Board25_iterator const& b) {
  return a.ps0 != b.ps0 || a.ps1 != b.ps1;
}

uint64_t Board25_iterator::operator*() noexcept {
//  std::cerr << "operator* : ps0=" << ps0 << ",ps1=" << ps1 << std::endl;
  Board25 b(b_v);
  int to = *ps1;
  Board25 next = b;
  if (b.turn() == Board25::black) {
    next.set_ppos(to);
    next.remove_browns(to);
    next.flip_turn();
  } else {
    next.v ^= ((1 << from) | (1 << to));
    next.flip_turn();
  }
  return next.v;
}

Board25_iterator Board25_iterator::operator++() noexcept {
  ++ps1;
  while (ps1.empty() && !ps0.empty()) {
    from = *ps0;
    ++ps0;
    ps1 = Board25(b_v).movable(from).begin();
  }
  return *this;
}

Board25_iterator Board25::begin() const {
//  std::cerr << "begin()" << std::endl;
  return Board25_iterator(v);
}
Board25_iterator Board25::end() const {
//  std::cerr << "end()" << std::endl;
  return Board25_iterator(v, 0, 0);
}


std::vector<Board25> Board25::next_states_v(bool debug) const {
//  std::cerr << "next_states_v()" << std::endl;
  std::vector<Board25> r;
  for (auto b : next_states()) {
    r.push_back(Board25(b));
  }
  return r;
}



static constexpr bool operator==(Board25 const& x, Board25 const& y) {
  return x.v == y.v;
}

static inline std::ostream& operator<<(std::ostream& os, Board25 const& v) {
  auto s = v.to_s();
  for (int y = 0; y < 5; y++)
    os << s.substr(y * 5, 5) << std::endl;
  return os << s.substr(25, 1);
}
