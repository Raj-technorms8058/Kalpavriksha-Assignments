#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MINIMUM_VALUE 2
#define MAXIMUM_VALUE 10
#define MAX_PIXEL_VALUE 255

int getValidInput(int *n) {
  while (1) {
    printf("Enter matrix size (%d-%d): ", MINIMUM_VALUE, MAXIMUM_VALUE);
    int status = scanf("%d", n);
    if (status == EOF) {
      printf("\nEnd of input reached. Exiting program.\n");
      return 0;
    }
    int valid = 1;
    int ch = getchar();
    while (ch != '\n' && ch != EOF) {
      if (ch != ' ' && ch != '\t' && ch != '\r') {
        valid = 0;
      }
      ch = getchar();
    }
    if (status == 1 && valid && *n >= MINIMUM_VALUE && *n <= MAXIMUM_VALUE) {
      return 1;
    }
    printf("Invalid input! Please enter an integer between %d and %d.\n",
           MINIMUM_VALUE, MAXIMUM_VALUE);
  }
}

int allocateMatrix(int ***mat, int **data, int n) {
  *mat = malloc((size_t)n * n * sizeof(int *));
  if (*mat == NULL) {
    return 0;
  }
  *data = malloc((size_t)n * n * sizeof(int));
  if (*data == NULL) {
    free(*mat);
    *mat = NULL;
    return 0;
  }
  for (int i = 0; i < n * n; i++) {
    *(*mat + i) = *data + i;
  }
  return 1;
}

void freeMatrix(int **mat, int *data) {
  free(mat);
  free(data);
}

void generateMatrix(int **mat, int n) {
  for (int i = 0; i < n * n; i++) {
    **(mat + i) = rand() % (MAX_PIXEL_VALUE + 1);
  }
}

void printMatrix(int **mat, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      printf("%4d ", **(mat + i * n + j));
    }
    printf("\n");
  }
}

void swapSlots(int **a, int **b) {
  int *temp = *a;
  *a = *b;
  *b = temp;
}

void rotateMatrix(int **mat, int n) {
  // Transpose the matrix by swapping elements across the diagonal.
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      swapSlots(mat + i * n + j, mat + j * n + i);
    }
  }
  // Reverse each row of the transposed matrix to rotate the matrix 90 degrees
  // clockwise.
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n / 2; j++) {
      swapSlots(mat + i * n + j, mat + i * n + (n - 1 - j));
    }
  }
}

int calculateAverage(int **mat, int n, int i, int j, int *prev, int *curr) {
  int sum = 0, count = 0;
  for (int r = i - 1; r <= i + 1; r++) {
    if (r < 0 || r >= n) {
      continue;
    }
    for (int c = j - 1; c <= j + 1; c++) {
      if (c < 0 || c >= n) {
        continue;
      }
      if (r == i - 1) {
        sum += *(prev + c);
      } else if (r == i) {
        sum += *(curr + c);
      } else {
        sum += **(mat + r * n + c);
      }
      count++;
    }
  }
  return sum / count;
}

int smoothMatrix(int **mat, int n) {
  int *prev = NULL, *curr = NULL, *temp;
  prev = malloc((size_t)n * sizeof(int));
  curr = malloc((size_t)n * sizeof(int));
  if (prev == NULL || curr == NULL) {
    free(prev);
    free(curr);
    return 0;
  }
  // Copy the current row before updating its pixel values.
  // prev stores the original values of the previous row,
  // while curr stores the original values of the current row.
  // This prevents updated pixels from affecting the next averages.
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      *(curr + j) = **(mat + i * n + j);
    }
    for (int j = 0; j < n; j++) {
      **(mat + i * n + j) = calculateAverage(mat, n, i, j, prev, curr);
    }
    // Swap prev and curr so the original values of the current row
    // are available as the previous row,while the old prev buffer
    // can be reused to store the original values of the next row.
    temp = prev;
    prev = curr;
    curr = temp;
  }
  free(prev);
  free(curr);
  return 1;
}

int main(void) {
  int n;
  int **mat = NULL;
  int *data = NULL;
  if (!getValidInput(&n)) {
    printf("Program terminated due to invalid or unavailable input.\n");
    return 1;
  }

  srand(time(NULL));

  if (!allocateMatrix(&mat, &data, n)) {
    printf("Memory allocation failed.\n");
    freeMatrix(mat, data);
    return 1;
  }

  generateMatrix(mat, n);
  printf("\nOriginal Randomly Generated Matrix:\n");
  printMatrix(mat, n);

  rotateMatrix(mat, n);
  printf("\nMatrix after 90 Degree Clockwise Rotation:\n");
  printMatrix(mat, n);

  if (!smoothMatrix(mat, n)) {
    printf("\nMemory allocation failed for smoothing buffers.\n");
    freeMatrix(mat, data);
    return 1;
  }

  printf("\nMatrix after Applying 3x3 Smoothing Filter:\n");
  printMatrix(mat, n);

  freeMatrix(mat, data);
  return 0;
}