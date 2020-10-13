#include "board.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

using Board25 = Board<25>;

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
  std::vector<int> exists(25);
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 5; x++) {
      int bi = Board25::toPos(x, y);
      EXPECT_LE(0,  exists[bi]);
      EXPECT_LT(exists[bi], 25);
      EXPECT_EQ(0, exists[bi]);
      exists[bi] = 0;
      EXPECT_EQ(x, Board25::XY(bi).first);
      EXPECT_EQ(y, Board25::XY(bi).second);
    }
}

TEST_F(BoardTest, makeup) {
  std::string bstr = 
    "ooooo"
    "o...o"
    "o.X.o"
    "o...o"
    "oooooo";
  Board25 b(bstr);
  EXPECT_EQ(1, b.turn());
  EXPECT_EQ(Board25::toPos(2, 2), b.ppos());
  EXPECT_EQ(bstr, b.to_s());
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 5; x++) {
      char c = b.get(x, y);
      if (y == 0 || y == 4 || x == 0 || x == 4)
	EXPECT_EQ('o', c);
      else if (x == 2 && y == 2)
	EXPECT_EQ('X', c);
      else
	EXPECT_EQ('.', c);
    }
}

TEST_F(BoardTest, neighbors) {
  {
    vI ps = Board25::neighbors_slow(Board25::toPos(2, 2));
    EXPECT_EQ(8, ps.size());
  }
  {
    PointSet ps = Board25::neighbors(Board25::toPos(2, 2));
    EXPECT_EQ(8, ps.size());
#if 0
    std::cerr << "pos=" << Board25::toPos(2, 2) << ", ps=" << ps << std::endl;
    for (int pos = 0; pos < 25; ++pos)
      std::cerr << "board25Table.neighbors[" << pos << "]=" << board25Table.neighbors(pos) << std::endl;
#endif
  }
}

TEST_F(BoardTest, test_movable) {
  {
    Board25 b("o...."
              "oo..."
              "Xoo.."
              "o...."
              "....."
              "o");
    {
      PointSet ps = b.movable(Board25::toPos(2, 2));
      EXPECT_EQ(6, ps.size());
    }
    {
      PointSet ps = b.movable(Board25::toPos(0, 0));
      EXPECT_EQ(1, ps.size());
    }
    {
      PointSet ps = b.movable(Board25::toPos(1, 1));
      EXPECT_EQ(3, ps.size());
    }
  }
  {
    Board25 b("o...."
              "oo..."
              "Xoo.."
              "o...."
              "....."
              "o");
    {
      PointSet ps = b.movable(Board25::toPos(0, 1));
      EXPECT_EQ(0, ps.size());
    }
  }
  {
    Board25 b("ooooo"
              "o...o"
              "o.X.o"
              "o...o"
              "oooooo");
    {
      PointSet ps = b.movable(Board25::toPos(0, 0));
      EXPECT_EQ(1, ps.size());
      EXPECT_TRUE(ps.test(Board25::toPos(1, 1)));
    }
    {
      PointSet ps = b.movable(Board25::toPos(2, 3));
      EXPECT_EQ(2, ps.size());
      EXPECT_TRUE(ps.test(Board25::toPos(1, 3)));
      EXPECT_TRUE(ps.test(Board25::toPos(3, 3)));
    }
  }
}

TEST_F(BoardTest, test_final_value) {
  {
    Board25 b("ooooo"
	      "o...o"
	      "o.X.o"
	      "o...o"
	      "oooooo");
    EXPECT_EQ(0, b.final_value());
  }
  {
    Board25 b("ooooo"
	      "Xo..o"
	      "o...o"
	      "o...o"
	      "oooooX");
    EXPECT_EQ(1, b.final_value());
  }
  {
    Board25 b(".o.o."
	      "ooooo"
	      ".oXo."
	      "ooooo"
	      ".o.o.X");
    EXPECT_EQ(1, b.final_value());
  }
}

TEST_F(BoardTest, test_flip) {
  {
    Board25 b("oo.oo"
	      "oXo.o"
	      "o..o."
	      "oo..o"
	      ".ooooo");
    Board25 b1(".oooo"
	       "oo..o"
	       "o..o."
	       "oXo.o"
	       "oo.ooo");
    Board25 b2 = b.flip();
    EXPECT_EQ(b1, b2);
    Board25 b3 = b1.flip();
    EXPECT_EQ(b, b3);
  }
}

TEST_F(BoardTest, test_rotate90) {
  {
    Board25 b("oo.oo"
	      "oXo.o"
	      "o..o."
	      "oo..o"
	      ".ooooo");
    Board25 b1("oo.oo"
	       "o.o.o"
	       ".o..o"
	       "oX.oo"
	       "oooo.o");
    Board25 b2 = b.rotate90();
    EXPECT_EQ(b1, b2);
  }
  {
    Board25 b("oX.oo"
	      "o.o.o"
	      "o..o."
	      "oo..o"
	      ".ooooo");
    Board25 b1("oo.oo"
	       "o.o.o"
	       ".o..o"
	       "X..oo"
	       "oooo.o");
    Board25 b2 = b.rotate90();
    EXPECT_EQ(b1, b2);
  }
}

TEST_F(BoardTest, test_normalize) {
  {
    Board25 b("oo.oo"
	      "oXo.o"
	      "o..o."
	      "oo..o"
	      ".ooooo");
    Board25 b1("oooo."
	       "oX.oo"
	       ".o..o"
	       "o.o.o"
	       "oo.ooo");
    Board25 b2 = b.normalize();
    EXPECT_EQ(b1, b2);
  }
  {
    Board25 b("ooooo"
	      "ooooo"
	      "Xoooo"
	      "ooooo"
	      "oooooo");
    Board25 b1("ooooo"
	      "ooooo"
	      "Xoooo"
	      "ooooo"
	      "oooooo");
    Board25 b2 = b.normalize();
    EXPECT_EQ(b1, b2);
  }
}

TEST_F(BoardTest, test_next_states) {
  {
    Board25 b("o...."
              "oo..."
              "Xoo.."
              "o...."
              "....."
              "o");
    Board25 b1("o...."
               "oo..."
               "Xo..."
               "o.o.."
               "....."
               "X");
    Board25 b2("o...."
               "oo..."
               "Xoo.."
               "....."
               ".o..."
               "X");
    std::vector<Board25> ns = b.next_states_v();
    EXPECT_EQ(13, ns.size());
#if 0
    for (auto b : ns) {
      std::cerr << "b.v=" << b.v << "," << Board25(b) << std::endl;
    }
#endif
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b1) != ns.end());
    EXPECT_TRUE(std::find(ns.begin(), ns.end(), b2) == ns.end());
    return;
  }
  {
    Board25 b("ooooo"
	      "o...o"
	      "o.X.o"
	      "o...o"
	      "oooooo");
    Board25 b1("oooo."
	       "o..oo"
	       "o.X.o"
	       "o...o"
	       "oooooX");
    Board25 b2("ooooo"
	       "o...."
	       "o.Xoo"
	       "o...o"
	       "oooooX");
    std::vector<Board25> ns = b.next_states_v();
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
    std::string bstr = 
      "ooooo"
      "..o.o"
      ".oX.o"
      "o...o"
      "oooooX";
    Board25 b(bstr);
    Board25 b1("ooooo"
	       "..o.o"
	       ".o.Xo"
	       "o...o"
	       "oooooo");
    Board25 b2("oo.oo"
	       "...X."
	       ".o..."
	       "o...o"
	       "oooooo");
    Board25 b3("ooooo"
	       "..o.o"
	       ".o..."
	       "o..Xo"
	       "oo.ooo");
    std::vector<Board25> ns = b.next_states_v();
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
    Board25 b("ooooo"
      "..o.o"
      ".oX.o"
      "o...o"
      "oooooX");
    EXPECT_EQ(16, b.browns_size());
  }
}

TEST_F(BoardTest, test_index) {
  {
    Board25 b("ooooo"
      "..o.o"
      ".oX.o"
      "o...o"
      "oooooX");
    uint64_t i = b.to_index();
    Board25 b1 = Board25::from_index(i, Board25::black);
    EXPECT_EQ(b1, b);
  }
  {
    uint64_t i0 = (3 << 24) | 0x80000;
    Board25 b = Board25::from_index(i0, Board25::brown);
    uint64_t i1 = b.to_index();
    EXPECT_EQ(i0, i1);
    EXPECT_EQ(Board25::brown, b.turn());
  }
}


int main(int ac, char **ag) {
  ::testing::InitGoogleTest(&ac, ag);
  return RUN_ALL_TESTS();
}
  
