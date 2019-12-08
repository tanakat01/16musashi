#include "board.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

constexpr int Board25::yoffsets[5];

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


TEST_F(BoardTest, test_index) {
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

TEST_F(BoardTest, test_movable) {
  Board25 b("ooooo"
	    "o...o"
	    "o.X.o"
	    "o...o"
	    "oooooo");
  {
    vI ps = b.movable(Board25::toPos(0, 0));
    EXPECT_EQ(1, ps.size());
    EXPECT_EQ(Board25::toPos(1, 1), ps[0]);
  }
  {
    vI ps = b.movable(Board25::toPos(2, 3));
    EXPECT_EQ(2, ps.size());
    EXPECT_TRUE(std::find(ps.begin(), ps.end(), Board25::toPos(1, 3)) != ps.end());
    EXPECT_TRUE(std::find(ps.begin(), ps.end(), Board25::toPos(3, 3)) != ps.end());
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

TEST_F(BoardTest, test_next_states) {
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
    std::vector<Board25> ns = b.next_states();
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
    std::vector<Board25> ns = b.next_states();
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

int main(int ac, char **ag) {
  ::testing::InitGoogleTest(&ac, ag);
  return RUN_ALL_TESTS();
}
  
