#include "sum_integers.h"

#include <vector>

int sum_integers(const std::vector<int> integers)
{
  int sum = 0;
  for (size_t i = 0; i < integers.size(); ++i)
  {
    sum += integers[i];
  }
  return sum;
}