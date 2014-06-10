#ifdef _OPENMP
#include <omp.h>
#endif

#include <stdio.h>

#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char **argv) {

  //printf("Running main() from gtest_main.cc\n");
  testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();

  return ret;
}
