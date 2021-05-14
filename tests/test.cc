#include "sum_integers.h"
#include "gtest/gtest.h"
#include <vector>

TEST(example, sum_zero)
{
  int o[6] = {1, -1, 2, -2, 3, -3};
  std::vector<int> integers(o, o + 6);
  int result = sum_integers(integers);
  ASSERT_EQ(result, 0);
}

TEST(example, sum_five)
{
  int o[6] = {1, 2, 3, 4, 5};
  const std::vector<int> integers(o, o + 5);
  int result = sum_integers(integers);
  ASSERT_EQ(result, 15);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}