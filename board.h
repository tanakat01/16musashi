
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <utility>
#include <x86intrin.h>
#include <cassert>
#include <cctype>

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
    return v == 0ull;
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

static inline std::ostream& operator<<(std::ostream& os, PointSet const& ps) {
  os << "{";
  for (auto p : ps) {
    os << p << ",";
  }
  return os << "}";
}

/*
SIZE = 33
 additional 8 point
 bits
 14 15 16 17 18    19
  7  8  9 10 11 12 13
  0  1  2  3  4  5  6
 20 21 22 23 24 25 26
 27 28 29 30 31    32
 */

/*
SIZE = 31
 additional 8 point
 bits
 13 14 15 16 17    18
  7  8  9 10 11 12 
  0  1  2  3  4  5  6
 19 20 21 22 23 24
 25 26 27 28 29    30
 */

/*
SIZE = 25
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

template<int SIZE>
class Board_iterator {
public:
  uint64_t b_v;
  int from;
  PointSetIterator ps0;
  PointSetIterator ps1;
  Board_iterator(uint64_t b_v_, uint64_t ps0_, uint64_t ps1_) :b_v(b_v_), ps0(ps0_), ps1(ps1_) {} 
  explicit Board_iterator(uint64_t b_v_) ;
  uint64_t operator*() noexcept;
  Board_iterator operator++() noexcept;
  Board_iterator operator++(int) noexcept {
    Board_iterator old = *this;
    ++(*this);
    return old;
  }
};


template<int SIZE>
class Board {
  static constexpr int yoffsets_25[5] = {10, 5, 0, 15, 20};
  static constexpr int yoffsets_31[5] = {13, 7, 0, 19, 25};
  static constexpr int yoffsets_33[5] = {14, 7, 0, 20, 27};
 public:
  static constexpr int yoffsets(int y) {
    if (SIZE == 25) return yoffsets_25[y];
    else if (SIZE == 31) return yoffsets_31[y];
    else return yoffsets_33[y];
    
  }
  static constexpr int HSIZE() {
    return yoffsets(3);
  }
  static constexpr int HEIGHT() {
    return 5;
  }
  static constexpr int WIDTH() {
    if (SIZE == 25) return 5;
    else return 7;
  }
  //  const static uint64_t brown_size = (1ull << 25);
  enum {
    black = 0,
    brown = 1};
  uint64_t v;
  Board(uint64_t v_) :v(v_){}
  Board(std::string const& s):v(0u) {
    uint64_t r = 0u;
    uint64_t bpos = SIZE;
    for (int y = 0; y < HEIGHT(); y++) {
      int base = yoffsets(y);
      int yj = 0;
      for (int x = 0; x < WIDTH(); x++) {
	//	std::cerr << "x=" << x << ",y=" << y << ",j=" << (y * WIDTH() + x) << ",yj=" << yj << std::endl;
	if (!hasPoint(x, y)) continue;
	char c = s[y * WIDTH() + x];
	if (c == 'o') r |= 1ull << (base + yj);
	else if (c == 'X') {
	  if (bpos != SIZE) throw std::runtime_error("too many black pieces");
	  bpos = base + yj;
	  // std::cerr << "X is found for s =" << ",bpos=" << bpos << std::endl;
	}
	if (!isspace(c)) yj++;
      } 	      
    }           
    uint64_t turn = (s[WIDTH() * HEIGHT()] == 'X' ? black : brown);
    v = (turn << bpos) | r | (bpos << SIZE);
  }
  static Board<SIZE> from_index(uint64_t index, int turn) {
    uint64_t bpos = index >> (SIZE - 1);
    uint64_t v = (index & ((1ull << bpos) - 1));
    v |= (index & ((1ull << (SIZE - 1)) - 1) & ~((1ull << bpos) - 1)) << 1;
    return Board((turn << bpos) | v | (bpos << SIZE));
  }
  uint64_t to_index() const {
    uint64_t r = browns().val();
    uint64_t bpos = ppos();
    r = (r & ((1ull << bpos) - 1)) | ((r & ((1ull << SIZE) - (1ull << bpos))) >> 1);   
    return r | (bpos << (SIZE - 1));
  }
  static constexpr int flip_pos(int pos) {
    if (SIZE == 25) {
      if (pos < 5) return pos;
      else if (pos < 15) return pos + 10;
      else return pos - 10;
    }
    else if (SIZE == 31) {
      if (pos < 7) return pos;
      else if (pos < 19) return pos + 12;
      else return pos - 12;
    }
    else if (SIZE == 33) {
      if (pos < 7) return pos;
      else if (pos < 20) return pos + 13;
      else return pos - 13;
    }
  }
  static constexpr int l2size() {
    if (SIZE == 25) return 5;
    else return 7;
  }
  static constexpr int l01size() {
    return (SIZE - l2size()) / 2;
  }
  Board<SIZE> flip() const {
    uint64_t bpos = ppos();
    uint64_t turn = (v >> bpos) & 1;
    uint64_t v_ = v & ~(1u << bpos);

    uint64_t n = v_ & ((1ull << l2size()) - 1);
    n |= ((v_ >> l2size()) & ((1ull << l01size()) - 1)) << (l01size() + l2size());
    n |= ((v_ >> (l2size() + l01size())) & ((1ull << l01size()) - 1)) << l2size();
    uint64_t bpos1 = flip_pos(bpos);
    n |= (bpos1 << SIZE) | (turn << bpos1);
    return Board(n);
  }     
  void flip_turn() {
    uint64_t bpos = ppos();
    // fprintf(stderr, "flip_turn(v = 0x%x, bpos = %d)", v, bpos);   
    v ^= (1ull << bpos);
    //  fprintf(stderr, " => v = 0x%x\n", v);   
  }
  void set_ppos(uint64_t new_ppos) {
    uint64_t bpos = ppos();
    uint64_t turn = (v >> bpos) & 1; 
    v &= ~(1ull << bpos) & ((1ull << SIZE) - 1);
    v |= (new_ppos << SIZE) | (turn << new_ppos);
  }
  uint64_t turn() const {
    uint64_t bpos = ppos();
    return (v >> bpos) & 1;
  }
  uint64_t ppos() const {
    return (v >> SIZE) & 63;
  }
  static II XY(int bi) {
    int x = SIZE + 1;
    int best_y = 0;
    for (int y = 0; y < HEIGHT(); y++) {
      int x1 = bi - yoffsets(y);
      if (0 <= x1 && x1 < x) {
	best_y = y;
	x = x1;
      }
    }
    if (x >= 5 && (best_y == 0 || best_y ==4)) x = 6;
    return II(x, best_y);
  }
  static bool isOnboard(int x, int y) {
    if (!(0 <= x && x < WIDTH() && 0 <= y && y < HEIGHT())) return false;
    if (SIZE != 25 && ((y == 0 || y == 4) && x == 5)) return false;
    return true;
  }
  static bool hasPoint(int x, int y) {
    if (!isOnboard(x, y)) return false;
    if (SIZE == 31 && ((y == 1 || y == 3) && x == 6)) return false;
    return true;
  }
  static bool hasN8(int x, int y) {
    return ((x + y) & 1) == 0;
  }
  static int toPos(int x, int y) {
    int r = yoffsets(y) + x;
    if (x == 6 && (y == 0 || y == 4)) r--;
    assert(r < SIZE);
    return r;
  }
  II pXY() const {
    return XY(ppos());
  }
  char get(int x, int y) const {
    II pxy = pXY();
    if (II(x, y) == pxy) return 'X';
    if ((v & (1ull << toPos(x, y))) != 0) return 'o';
    if (hasPoint(x, y)) return '.';
    return ' ';
  }
  char get(int pos) const {
    if (pos == ppos()) return 'X';
    if ((v & (1ull << pos)) != 0) return 'o';
    auto [x, y] = II(pos);
    if (hasPoint(x, y)) return '.';
    return ' ';
  }
  char get_slow(int x, int y) const {
    if (!isOnboard(x, y)) return ' ';
    II pxy = pXY();
    if (II(x, y) == pxy) return 'X';
    if ((v & (1ull << toPos(x, y))) != 0) return 'o';
    return '.';
  }
  static vI neighbors_slow(int pos) {
    vI r;
    auto [x, y] = XY(pos);
    // std::cerr << "neighbors_slow(pos=" << pos << ",x=" << x << ",y=" << y << std::endl;
    if (hasN8(x, y)) {
      for (auto [dx, dy] : n8) {
	if (SIZE == 31) {
	  if (dx * dy != 0) {
	    if (x == 2 && y == 2) continue; //center
	    if ((x == 0 || x == 4) && (y == 0 || y == 4)) continue; //corner
	    if ((x == 1 || x == 3) && x == y && dx * dy > 0) continue;
	    if ((x == 1 || x == 3) && x == (4 - y) && dx * dy < 0) continue;
	  }
	  if (x == 6 && y == 2 && dx == -1 && dy == 0) continue;
	}
	if ((x == 4 && y != 2) && dx == 1) continue;
	if (SIZE == 31 && x == 6) dy *= 2;
	int x1 = x + dx, y1 = y + dy;
	if (x == 5 && dx == -1 && y1 != 2) continue;
	if (x == 6 && dx == -1 && (y1 == 0 || y1 == 4)) continue;
	if (hasPoint(x1, y1)) r.push_back(toPos(x1, y1));
      }
    }
    else {
      for (auto [dx, dy] : n4) {
	int x1 = x + dx, y1 = y + dy;
	if ((x == 4 && y != 2) && dx == 1) continue;
	if (SIZE == 31) {
	  if (x == 5 && y == 2 && dx == 1) continue;
	  if (x == 6 && y == 2 && dx == -1 && dy == 0) continue;
	}
	if (hasPoint(x1, y1)) r.push_back(toPos(x1, y1));
      }
    }
    return r;
  }
  static PointSet neighbors(int pos);
  void remove_browns_slow(int pos) {
    auto [x, y] = XY(pos);
    vI ns = neighbors_slow(pos);
    if (hasN8(x, y)) {
      for (auto [dx, dy] : h8) {
	int x1 = x + dx, y1 = y + dy;
	int x2 = x - dx, y2 = y - dy;
	if (get_slow(x1, y1) == 'o' && get_slow(x2, y2) == 'o') {
	  int pos1 = toPos(x1, y1), pos2 = toPos(x2, y2);
	  if (find(ns.begin(), ns.end(), pos1) == ns.end()) continue;
	  if (find(ns.begin(), ns.end(), pos2) == ns.end()) continue;
	  v ^= ((1ull << pos1) | (1ull << pos2));
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
    for (int i = 0; i < SIZE; i++) {
      if (i != ppos() && ((1ull << i) & v) != 0) r.push_back(i); 
    }
    return r;
  }
  PointSet browns() const;
  PointSet pieces() const;

  int browns_size() const;
  std::vector<Board<SIZE> > next_states_slow(bool debug=false) const {
    std::vector<Board<SIZE> > r;
    if (debug) std::cerr << "turn()=" << turn() << std::endl;
    if (turn() == black) {
      int ppos_ = ppos();
      if (debug) std::cerr << "ppos_=" << ppos_ << std::endl;
      for (int pos : movable(ppos_)) {
	if (debug) std::cerr << "pos=" << pos << std::endl;
	Board next = *this;
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
	  Board next = *this;
	  next.v ^= ((1ull << fPos) | (1ull << tPos));
	  // fprintf(stderr, "before flip= 0x%x",  next.v);
	  next.flip_turn();
	  // fprintf(stderr, "after  flip= 0x%x", next.v);
	  r.push_back(next);
	}
      }
    }
    return r;
  }
  Board next_states(bool debug=false) const {
    // std::cerr << "next_states()" << std::endl;
    return *this;
  }
  std::vector<Board> next_states_v(bool debug=false) const;
  Board_iterator<SIZE> begin() const;
  Board_iterator<SIZE> end() const;
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
    for (int y = 0; y < HEIGHT(); y++) {
      for (int x = 0; x < WIDTH(); x++) {
	r += get(x, y);
      }
    }
    r += (turn() != black ? 'o' : 'X');
    return r;
  }
};

template<int SIZE>
class BoardTable {
  PointSet neighbors_[SIZE];
public:
  PointSet c2[SIZE][4];
  BoardTable();
  PointSet neighbors(int pos) const {
    assert(0 <= pos && pos < SIZE);
    return neighbors_[pos];
  }
};

template<int SIZE>
BoardTable<SIZE>::BoardTable() {
  // std::cerr << "making BoardTable SIZE=" << SIZE << std::endl;
  for (int pos = 0; pos < SIZE; pos++) {
    neighbors_[pos] = PointSet(Board<SIZE>::neighbors_slow(pos));
#if 0
    if (pos == 2) {
      std::cerr << "pos=" << pos << ",neighbors=" << neighbors_[pos] << std::endl;
      vI ps = Board<SIZE>::neighbors_slow(pos);
      for (auto p : ps) {
	std::cerr << "," << p;
      }
      std::cerr << std::endl;
    }
#endif
  }
  // std::cerr << "end of neighbors" << std::endl;
  for (int y = 0; y < Board<SIZE>::HEIGHT(); y++)
    for (int x = 0; x < Board<SIZE>::WIDTH(); x++) {
      // std::cerr << "x=" << x <<",y=" << y << std::endl;
      int pos = Board<SIZE>::toPos(x, y);
      std::fill(&c2[pos][0], &c2[pos][4], PointSet());
      int i = 0;
      for (auto [dx, dy] : h8) {
	int x1 = x + dx, y1 = y + dy;
	int x2 = x - dx, y2 = y - dy;
	if (Board<SIZE>::isOnboard(x1, y1) && Board<SIZE>::isOnboard(x2, y2)) {
	  int pos1 = Board<SIZE>::toPos(x1, y1);
	  int pos2 = Board<SIZE>::toPos(x2, y2);
	  if (neighbors_[pos].test(pos1) &&
	      neighbors_[pos].test(pos2)) { 
            c2[pos][i++] = PointSet(vI({pos1, pos2}));
	  }
	}
      }
    }
  //   std::cerr << "boardTable<" << SIZE << ">.neighbors(2)=" << neighbors(2) << std::endl;
}


static BoardTable<25> board25Table;
static BoardTable<31> board31Table;
static BoardTable<33> board33Table;

template<int SIZE>
inline PointSet Board<SIZE>::neighbors(int pos) {
  if (SIZE == 25) return board25Table.neighbors(pos);
  else if (SIZE == 31) return board31Table.neighbors(pos);
  else if (SIZE == 33) return board33Table.neighbors(pos);
}

template<int SIZE>
inline void Board<SIZE>::remove_browns(int pos) {
  for (int i = 0; i < 4; i++) {
    uint64_t c2 = (SIZE == 25 ? board25Table.c2[pos][i].val() :
		   (SIZE == 31 ?  board31Table.c2[pos][i].val() :
		    board33Table.c2[pos][i].val()));
    if (c2 == 0ull) break;
    if ((v & c2) == c2) {
      v &= ~c2;
    }
  }
}

template<int SIZE>
inline PointSet Board<SIZE>::browns() const {
  int bpos = Board<SIZE>::ppos();
  return PointSet((v & ~(1ull << bpos)) & ((1ull << SIZE) - 1));
  
}

template<int SIZE>
inline PointSet Board<SIZE>::pieces() const {
  int bpos = Board<SIZE>::ppos();
  return PointSet((v | (1ull << bpos)) & ((1ull << SIZE) - 1));
}

template<int SIZE>
inline int Board<SIZE>::browns_size() const {
  return popcnt(browns().val());
}

template<int SIZE>
PointSet Board<SIZE>::movable(int pos) const {
  return ~pieces() & (SIZE == 25 ? board25Table.neighbors(pos) :
		      (SIZE == 21 ? board31Table.neighbors(pos) :
		       board33Table.neighbors(pos)));
}

template<int SIZE>
Board_iterator<SIZE>::Board_iterator(uint64_t b_v_) :b_v(b_v_), ps0(0), ps1(0) {
  Board<SIZE> b(b_v);
  if (b.turn() == Board<SIZE>::brown) {
    ps0 = b.browns().begin();
    // std::cerr << "ps0=" << ps0 << std::endl;
    while (ps1.empty() && !ps0.empty()) {
      from = *ps0;
      ++ps0;
      ps1 = Board<SIZE>(b_v).movable(from).begin();
      // std::cerr << "ps0=" << ps0 << ",from=" << from << ",ps1=" << ps1 << std::endl;
    }
  } else {
    from = b.ppos();
    ps1 = b.movable(from).begin();
  }
//  std::cerr << "construct ps0=" << ps0 << ",ps1=" << ps1 << std::endl;
}

template<int SIZE>
bool operator!=(Board_iterator<SIZE> const& a, Board_iterator<SIZE> const& b) {
  return a.ps0 != b.ps0 || a.ps1 != b.ps1;
}

template<int SIZE>
uint64_t Board_iterator<SIZE>::operator*() noexcept {
//  std::cerr << "operator* : ps0=" << ps0 << ",ps1=" << ps1 << std::endl;
  Board<SIZE> b(b_v);
  int to = *ps1;
  Board<SIZE> next = b;
  if (b.turn() == Board<SIZE>::black) {
    next.set_ppos(to);
    next.remove_browns(to);
    next.flip_turn();
  } else {
    next.v ^= ((1ull << from) | (1ull << to));
    next.flip_turn();
  }
  return next.v;
}

template<int SIZE>
Board_iterator<SIZE> Board_iterator<SIZE>::operator++() noexcept {
  ++ps1;
  while (ps1.empty() && !ps0.empty()) {
    from = *ps0;
    ++ps0;
    ps1 = Board<SIZE>(b_v).movable(from).begin();
    // std::cerr << "from=" << from << ",ps1=" << ps1 << std::endl;
  }
  return *this;
}

template<int SIZE>
Board_iterator<SIZE> Board<SIZE>::begin() const {
  // std::cerr << "begin(), v=" << v << std::endl;
  return Board_iterator<SIZE>(v);
}
template<int SIZE>
Board_iterator<SIZE> Board<SIZE>::end() const {
//  std::cerr << "end()" << std::endl;
  return Board_iterator<SIZE>(v, 0, 0);
}

template<int SIZE>
std::vector<Board<SIZE> > Board<SIZE>::next_states_v(bool debug) const {
//  std::cerr << "next_states_v()" << std::endl;
  std::vector<Board<SIZE>> r;
  for (auto b : next_states()) {
    r.push_back(Board<SIZE>(b));
  }
  return r;
}



template<int SIZE>
static constexpr bool operator==(Board<SIZE> const& x, Board<SIZE> const& y) {
  return x.v == y.v;
}

template<int SIZE>
static inline std::ostream& operator<<(std::ostream& os, Board<SIZE> const& v) {
  auto s = v.to_s();
  for (int y = 0; y < Board<SIZE>::HEIGHT(); y++)
    os << s.substr(y * Board<SIZE>::WIDTH(), Board<SIZE>::WIDTH()) << std::endl;
  return os << s.substr(Board<SIZE>::WIDTH() * Board<SIZE>::WIDTH(), 1);
}
