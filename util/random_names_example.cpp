#include <cstdio>

#include "randomnames.h"

// compile with g++ util/random_names_example.cpp -I sources/Application/Utils -o test
int main(int argc, char *argv[]) {
  printf("Test names \n");
  int max = argc == 2 ? atoi(argv[1]) : 0;

  if (max == 0) {
    printf("provide arg with number of names to generate\n");
    exit(1);
  }

  RandomNames names {};

  for(int i =0; i < max; i++) {
    auto name = names.getName();
    printf("name: %s\n", name.c_str());
  }
}