#include "gtest/gtest.h"
#include "../sources/logger/logger.h"
#include <vector>

// TEST(example, sum_zero)
// {
//   int o[6] = {1, -1, 2, -2, 3, -3};
//   std::vector<int> integers(o, o + 6);
//   int result = sum_integers(integers);
//   ASSERT_EQ(result, 0);
// }

// TEST(example, sum_five)
// {
//   int o[6] = {1, 2, 3, 4, 5};
//   const std::vector<int> integers(o, o + 5);
//   int result = sum_integers(integers);
//   ASSERT_EQ(result, 15);
// }
TEST(example, run) {
    ft::Logger SAYONARA(ft::Logger::WARNING);
    
    log(&SAYONARA, ft::Logger::DEBUG, "HELLO %s", "sssS");
    log(&SAYONARA, ft::Logger::INFO, "HELLO %s", "sssS");
    info(&SAYONARA, "MURAVEY");
    error(&SAYONARA, "SUDO");
    warning(&SAYONARA, "MURAVEY %s %p", "iiiiiiii", &SAYONARA);
    critical(&SAYONARA, "III");
    SUCCEED();
}