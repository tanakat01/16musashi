#include <cstdio>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>

typedef std::vector<int> vI;
typedef std::pair<int, int> II;

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
class Board25 {
  static constexpr int yoffsets[5] = {10, 5, 0, 15, 20};
public:
  const static int black = 0;
  const static int brown = 1;
  uint32_t v;
  Board25(uint32_t v_) :v(v_){}
  Board25(std::string const& s):v(0u) {
    uint32_t r = 0u;
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
  static constexpr int flip_pos(int pos) {
    if (pos < 5) return pos;
    else if (pos < 15) return pos + 10;
    else return pos - 10;
  }
  Board25 flip() const {
    int bpos = (v >> 25) & 31;
    int turn = (v >> bpos) & 1;
    uint32_t v_ = v & ~(1u << bpos);
    uint32_t n = (v_ & 0b11111) + 
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
  static vI neighbors(int pos) {
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
  void remove_browns(int pos) {
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

  vI movable(int from_pos) const {
    vI r;
    for (int pos : neighbors(from_pos)) {
      if (get(pos) == '.') r.push_back(pos);
    }
    return r;
  }
  vI browns() const {
    vI r;
    for (int i = 0; i < 25; i++) {
      if (i != ppos() && ((1 << i) & v) != 0) r.push_back(i); 
    }
    return r;
  }
  std::vector<Board25> next_states(bool debug=false) const {
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
  /**
   * 1 : win by brown
   * 0 : not known
   * -1 : win by black
   */
  int final_value() const {
    if (turn() == black) {
      int ppos_ = ppos();
      if (movable(ppos_).size() == 0) return 1;
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
static constexpr bool operator==(Board25 const& x, Board25 const& y) {
  return x.v == y.v;
}

static inline std::ostream& operator<<(std::ostream& os, Board25 const& v) {
  auto s = v.to_s();
  for (int y = 0; y < 5; y++)
    os << s.substr(y * 5, 5) << std::endl;
  return os << s.substr(25, 1);;
}
