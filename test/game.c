// Edited by uint256_t

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 20

int count_nbr(int grid[SIZE][SIZE], int i, int j, int size);

int main(void) {
  int neighbour_count[SIZE][SIZE];
  int grid[SIZE][SIZE] = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  int i;
  int j;
  int steps;

  for (steps = 0; steps < 200; ++steps) {
    printf("\e[0;0H");
    for (i = 0; i < SIZE; ++i) {
      printf("\n");
      for (j = 0; j < SIZE; ++j) {
        if (grid[i][j] == 1)
          printf("\e[42m  \e[m");
        else
          printf("\e[47m  \e[m");
        neighbour_count[i][j] = count_nbr(grid, i, j, SIZE);
      }
    }

    for (i = 0; i < SIZE; ++i) {
      for (j = 0; j < SIZE; ++j) {
        if (grid[i][j] >= 1) {
          if (neighbour_count[i][j] <= 1 || neighbour_count[i][j] >= 4)
            grid[i][j] = 0;
        } else if (neighbour_count[i][j] == 3)
          grid[i][j] = 1;
      }
    }

    usleep(10000);
  }

  return 0;
}

int count_nbr(int grid[SIZE][SIZE], int i, int j, int size) {
  int n_count = 0;
  if (i - 1 >= 0 && j - 1 >= 0) {
    if (grid[i - 1][j - 1] >= 1)
      n_count++;
  }

  if (i - 1 >= 0) {
    if (grid[i - 1][j] >= 1)
      n_count++;
  }

  if (i - 1 >= 0 && j + 1 < size) {
    if (grid[i - 1][j + 1] >= 1)
      n_count++;
  }

  if (j - 1 >= 0) {
    if (grid[i][j - 1] >= 1)
      n_count++;
  }

  if (j + 1 < size) {
    if (grid[i][j + 1] >= 1)
      n_count++;
  }

  if (i + 1 < size && j - 1 >= 0) {
    if (grid[i + 1][j - 1] >= 1)
      n_count++;
  }

  if (i + 1 < size) {
    if (grid[i + 1][j] >= 1)
      n_count++;
  }

  if (i + 1 < size && j + 1 < size) {
    if (grid[i + 1][j + 1] >= 1)
      n_count++;
  }

  return n_count;
}

