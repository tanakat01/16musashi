#include "board.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

using Board31 = Board<31>;
static const int SIZE = 31;
using B = Board31;

static const int WIDTH = B::WIDTH();
static const int HEIGHT = B::HEIGHT();

class BoardTest : public ::testing::Test {
public:
  BoardTest() {
  }
};

TEST_F(BoardTest, test_pointset_iteration) {
  {
    uint64_t v = (1ull << 5) + (1ull << 34);
    std::vector<int> points;
    for (auto p : PointSet(v)) {
      points.push_back(p);
    }
    EXPECT_EQ(2, points.size());
    EXPECT_TRUE(std::find(points.begin(), points.end(), 5) != points.end());
    EXPECT_TRUE(std::find(points.begin(), points.end(), 34) != points.end());
  }
}


TEST_F(BoardTest, test_pos) {
  std::vector<int> exists(SIZE);
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++) {
      if (!B::hasPoint(x, y)) continue;
      int bi = B::toPos(x, y);
      // std::cerr << "toPos(" << x << ", " << y << ")=" << bi << std::endl;
      EXPECT_LE(0,  exists[bi]);
      EXPECT_LT(exists[bi], SIZE);
      EXPECT_EQ(0, exists[bi]);
      exists[bi] = 0;
      EXPECT_EQ(x, B::XY(bi).first);
      EXPECT_EQ(y, B::XY(bi).second);
    }
}

TEST_F(BoardTest, makeFromV) {
  {
    B b(0x3'4050'3300ull);
    EXPECT_EQ(7, b.browns_size());
    //    std::cerr << std::hex << "b.v=" << b.v << std::endl;
    // std::cerr << b << std::endl;
  }
}

TEST_F(BoardTest, makeup) {
  std::string bstr = 
    "ooooo.."
    "o...o.."
    "o.X.o.."
    "o...o.."
    "ooooo..o";
  B b(bstr);
  EXPECT_EQ(1, b.turn());
  //  std::cerr << "b.val()=" + std::to_string(b.v) << std::endl;
  EXPECT_EQ(B::toPos(2, 2), b.ppos());
  EXPECT_EQ(bstr, b.to_s());
  for (int y = 0; y < HEIGHT; y++)
    for (int x = 0; x < WIDTH; x++) {
      char c = b.get(x, y);
      if ((x <= 4 &&  (y == 0 || y == 4)) || x == 0 || x == 4)
	EXPECT_EQ('o', c);
      else if (x == 2 && y == 2)
	EXPECT_EQ('X', c);
      else if (B::hasPoint(x, y)) {
	EXPECT_EQ('.', c);
      }
    }
}

TEST_F(BoardTest, neighbors) {
  {
    int msize[5][7] = {{2, 3, 5, 3, 2, 0, 2},
		       {3, 6, 4, 6, 3, 4, 0},
		       {5, 4, 4, 4, 8, 3, 4},
		       {3, 6, 4, 6, 3, 4, 0},
		       {2, 3, 5, 3, 2, 0, 2}};
    int i = 0;
    for (int y = 0; y < B::HEIGHT(); y++)
      for (int x = 0; x < B::WIDTH(); x++) {
	if (!B::hasPoint(x, y)) continue;
	i++;
	PointSet ps = B::neighbors(B::toPos(x, y));
	if (msize[y][x] != ps.size())
	  std::cerr << "x=" << x << ",y=" << y << std::endl;
	EXPECT_EQ(msize[y][x], ps.size());
      }
  }
  {
    PointSet ps = B::neighbors(B::toPos(2, 2));
    vI ps_v = B::neighbors_slow(B::toPos(2, 2));
    EXPECT_EQ(4, ps.size());
    EXPECT_EQ(4, ps_v.size());
  }
  {
    PointSet ps = B::neighbors(B::toPos(4, 2));
    vI ps_v = B::neighbors_slow(B::toPos(4, 2));
    EXPECT_EQ(8, ps.size());
    EXPECT_EQ(8, ps_v.size());
  }
  {
    PointSet ps = B::neighbors(B::toPos(5, 1));
    vI ps_v = B::neighbors_slow(B::toPos(5, 1));
    EXPECT_EQ(4, ps.size());
    EXPECT_EQ(4, ps_v.size());
  }
  {
    PointSet ps = B::neighbors(B::toPos(5, 2));
    vI ps_v = B::neighbors_slow(B::toPos(5, 2));
    EXPECT_EQ(3, ps.size());
    EXPECT_EQ(3, ps_v.size());
  }
  {
    PointSet ps = B::neighbors(B::toPos(6, 0));
    vI ps_v = B::neighbors_slow(B::toPos(6, 0));
    EXPECT_EQ(2, ps.size());
    EXPECT_EQ(2, ps_v.size());
  }
  {
    PointSet ps = B::neighbors(B::toPos(6, 2));
    vI ps_v = B::neighbors_slow(B::toPos(6, 2));
    EXPECT_EQ(4, ps.size());
    EXPECT_EQ(4, ps_v.size());
  }
  {
    PointSet ps = B::neighbors(B::toPos(4, 1));
    vI ps_v = B::neighbors_slow(B::toPos(4, 1));
    // std::cerr << "ps=" << ps <<std::endl; 
    EXPECT_EQ(3, ps.size());
    EXPECT_EQ(3, ps_v.size());
  }
  {
    PointSet ps = B::neighbors(B::toPos(4, 0));
    vI ps_v = B::neighbors_slow(B::toPos(4, 0));
    EXPECT_EQ(2, ps.size());
    EXPECT_EQ(2, ps_v.size());
  }
}

TEST_F(BoardTest, test_movable) {
  {
    B b("o......"
	"oo...o."
	"Xoo.o.."
	"o...o.."
	"......o"
	"o");
    {
      PointSet ps = b.movable(B::toPos(2, 2));
      //std::cerr << "ps=" << ps << std::endl;
      //std::cerr << "neighbors=" << b.neighbors(B::toPos(2, 2)) << std::endl;
      EXPECT_EQ(3, ps.size());
    }
    {
      PointSet ps = b.movable(B::toPos(0, 0));
      EXPECT_EQ(1, ps.size());
    }
    {
      PointSet ps = b.movable(B::toPos(1, 1));
      EXPECT_EQ(3, ps.size());
    }
    {
      PointSet ps = b.movable(B::toPos(5, 1));
      EXPECT_EQ(3, ps.size());
    }
    {
      PointSet ps = b.movable(B::toPos(4, 2));
      EXPECT_EQ(6, ps.size());
    }
    {
      PointSet ps = b.movable(B::toPos(4, 3));
      EXPECT_EQ(2, ps.size());
    }
    {
      PointSet ps = b.movable(B::toPos(6, 4));
      EXPECT_EQ(2, ps.size());
    }
  }
  {
    B b("o.... ."
	"oo....."
	"Xoo...."
	"o......"
	"..... ."
	"o");
    {
      PointSet ps = b.movable(B::toPos(0, 1));
      EXPECT_EQ(0, ps.size());
    }
  }
  {
    B b("ooooo ."
	"o...o.."
	"o.X.o.."
	"o...o.."
	"ooooo .o");
    {
      PointSet ps = b.movable(B::toPos(0, 0));
      EXPECT_EQ(0, ps.size());
    }
    {
      PointSet ps = b.movable(B::toPos(2, 3));
      EXPECT_EQ(2, ps.size());
      EXPECT_TRUE(ps.test(B::toPos(1, 3)));
      EXPECT_TRUE(ps.test(B::toPos(3, 3)));
    }
  }
}

TEST_F(BoardTest, test_final_value) {
  {
    B b("ooooo ."
	"o...o.."
	"o.X.o.."
	"o...o.."
	"ooooo .o");
    EXPECT_EQ(0, b.final_value());
  }
  {
    B b("ooooo ."
	"Xo..o.."
	"o...o.."
	"o...o.."
	"ooooo .X");
    EXPECT_EQ(1, b.final_value());
  }
  {
    B b(".o.o. ."
	"ooooo.."
	".oXo..."
	"ooooo.."
	".o.o. .X");
    EXPECT_EQ(1, b.final_value());
  }
}

TEST_F(BoardTest, test_flip) {
  {
    B b("oo.oo ."
	"oXo.oo."
	"o..o..."
	"oo..o.."
	".oooo .o");
    B b1(".oooo ."
	 "oo..o.."
	 "o..o..."
	 "oXo.oo."
	 "oo.oo .o");
    B b2 = b.flip();
    EXPECT_EQ(b1, b2);
    B b3 = b1.flip();
    EXPECT_EQ(b, b3);
  }
  {
    B b("......."
	"......."
	"..X.ooo"
	"......."
	"......o"
	"o");
    B b1("......o"
	 "......."
	 "..X.ooo"
	 "......."
	 "......."
	"o");
    B b2 = b.flip();
    EXPECT_EQ(b1, b2);
    B b3 = b1.flip();
    EXPECT_EQ(b, b3);
  }
}

TEST_F(BoardTest, test_next_states) {
  {
    B b("o.....o"
	"...o.X."
	"......."
	"o....o."
	"..... o"
	"X");
    B b1("o......"
	"...o..."
	"......X"
	"o....o."
	"..... ."
	"o");
    B b2("o.... o"
	"......."
	"....X.."
	"o......"
	"..... o"
	"o");
    B b3("o.... o"
	"...o..."
	".....X."
	"o....o."
	"..... o"
	"o");
    std::vector<B> ns = b.next_states_v();
    EXPECT_EQ(3, ns.size());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b1) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b2) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b3) != ns.end());
    return;
  }
  {
    B b("o.... o"
	"oo....."
	"Xoo...."
	"o......"
	"..... ."
	"o");
    B b1("o.... o"
	 "oo....."
	 "Xo....."
	 "o.o...."
	 "..... ."
	 "X");
    B b2("o.... o"
	 "oo....."
	 "Xoo...."
	 "......."
	 ".o... ."
	 "X");
    B b3("o.... ."
	 "oo...o."
	 "Xoo...."
	 "o......"
	 "..... ."
	 "X");
    B b4("o.... ."
	 "oo....."
	 "Xoo...o"
	 "o......"
	 "..... ."
	 "X");
    std::vector<B> ns = b.next_states_v();
    EXPECT_EQ(12, ns.size());
#if 0
    for (auto b : ns) {
      std::cerr << "b.v=" << b.v << "," << B(b) << std::endl;
    }
#endif
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b1) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b2) == ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b3) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b4) != ns.end());
    return;
  }
  {
    B b("ooooo ."
	"o...o.."
	"o.X.o.."
	"o...o.."
	"ooooo .o");
    B b1("oooo. ."
	 "o..oo.."
	 "o.X.o.."
	 "o...o.."
	 "ooooo .X");
    B b2("ooooo ."
	 "o......"
	 "o.Xoo.."
	 "o...o.."
	 "ooooo .X");
    std::vector<B> ns = b.next_states_v();
    EXPECT_EQ(24, ns.size());
#if 0
    for (auto b : ns) {
      std::cerr << b << std::endl;
    }
#endif
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b1) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b2) == ns.end());
  }
  {
    B b("ooooo ."
	"..o.o.."
	".oX.o.."
	"o...o.."
	"ooooo .X");
    B b1("ooooo ."
	 "..o.o.."
	 ".o.Xo.."
	 "o...o.."
	 "ooooo .o");
    B b2("oo.oo ."
	 "...X..."
	 ".o....."
	 "o...o.."
	 "ooooo .o");
    B b3("ooooo ."
	 "..o.o.."
	 ".o....."
	 "o..Xo.."
	 "oo.oo .o");
    std::vector<B> ns = b.next_states_v();
#if 0
    for (auto n : ns) {
      std::cerr << n << std::endl;
    }
#endif    
    EXPECT_EQ(6, ns.size());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b1) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b2) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b3) != ns.end());
  }
}

TEST_F(BoardTest, test_browns_size) {
  {
    B b("ooooo ."
	"..o.o.."
	".oX.o.."
	"o...o.."
	"ooooo .X");
    EXPECT_EQ(16, b.browns_size());
  }
}

TEST_F(BoardTest, test_index) {
  {
    B b("ooooo ."
	"..o.o.."
	".oX.o.."
	"o...o.."
	"ooooo .X");
    uint64_t i = b.to_index();
    B b1 = B::from_index(i, B::black);
    EXPECT_EQ(b1, b);
  }
  {
    uint64_t i0 = (3ull << (SIZE - 1)) | 0x80000ull;
    B b = B::from_index(i0, B::brown);
    uint64_t i1 = b.to_index();
    EXPECT_EQ(i0, i1);
    EXPECT_EQ(B::brown, b.turn());
  }
}


int main(int ac, char **ag) {
  ::testing::InitGoogleTest(&ac, ag);
  return RUN_ALL_TESTS();
}
  
