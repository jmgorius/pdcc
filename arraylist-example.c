#include <stdio.h>

#include "arraylist.h"

#define NUM_ITERATIONS 20

int main(void) {
  int *squares = 0;
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    arraylist_push(squares, i * i);
  }
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    printf("%d^2 = %d\n", i, squares[i]);
  }
  arraylist_free(squares);
  return 0;
}
