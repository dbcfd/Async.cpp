#include <iostream>
#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  std::cout << "Running main() from TestRunner.cpp\n";

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}