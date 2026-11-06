#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MIN_MATRIX_SIZE 2
#define MAX_MATRIX_SIZE 10
#define MAX_PIXEL_VALUE 255

int getValidInput(int *n) {
  char input[100], *end;
  long value;
  int ch;

  while (1) {
    printf("Enter matrix size (%d-%d): ", MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);

    if (fgets(input, sizeof(input), stdin) == NULL) {
      printf("\nEnd of input reached. Exiting program.\n");
      return 0;
    }

    if (strchr(input, '\n') == NULL && !feof(stdin)) {
      while ((ch = getchar()) != '\n' && ch != EOF) {
      }
      printf("Invalid input! Please enter an integer between %d and %d.\n",
             MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);
      continue;
    }

    value = strtol(input, &end, 10);

    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') {
      end++;
    }

    if (end != input && *end == '\0' &&
        value >= MIN_MATRIX_SIZE && value <= MAX_MATRIX_SIZE) {
      *n = (int)value;
      return 1;
    }

    printf("Invalid input! Please enter an integer between %d and %d.\n",
           MIN_MATRIX_SIZE, MAX_MATRIX_SIZE);
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

  // Pointer slots are used because the assignment requires pointer swapping for in-place matrix rotation.
  for (int i = 0; i < n * n; i++) {
    *(*mat + i) = *data + i;
  }
  return 1;
}

void freeMatrix(int **mat, int *data) {
  free(mat);
  free(data);
}

int *cell(int **mat, int n, int i, int j) {
  return *(mat + i * n + j);
}

void generateMatrix(int **mat, int n) {
  for (int i = 0; i < n * n; i++) {
    **(mat + i) = rand() % (MAX_PIXEL_VALUE + 1);
  }
}

void printMatrix(int **mat, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      printf("%4d ", *cell(mat, n, i, j));
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

int calculateAverage(int *copy, int n, int i, int j) {
  int sum = 0, count = 0;
  for (int r = i - 1; r <= i + 1; r++) {
    if (r < 0 || r >= n) {
      continue;
    }
    for (int c = j - 1; c <= j + 1; c++) {
      if (c < 0 || c >= n) {
        continue;
      }
      sum += *(copy + r * n + c);
      count++;
    }
  }
  // Integer division in sum / count truncates the fractional part.
  return sum / count;
}

int smoothMatrix(int **mat, int n) {
  int *copy = malloc((size_t)n * n * sizeof(int));
  if (copy == NULL) {
    return 0;
  }

  // Copy the original matrix before updating its pixel values.
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      *(copy + i * n + j) = *cell(mat, n, i, j);
    }
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      *cell(mat, n, i, j) = calculateAverage(copy, n, i, j);
    }
  }

  free(copy);
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