/*
 * Ryan Lewis
 * CS652 - Homework 1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Image struct.  Keeps track of image dimensions, maximum value in the image,
 * and the matrix of values that make up the image itself.
 */
typedef struct Image {
  int row;
  int col;
  int max_val;
  int **img;
} Image;

/*
 * Allocate a matrix.
 */
int **alloc_matrix(int row, int col) {
  int **ret_matrix = (int **) malloc(sizeof(int *) * row);
  int i;

  for (i = 0; i < row; i++)
    ret_matrix[i] = (int *) malloc(sizeof(int) * col);

  return ret_matrix;
}

/*
 * Free a matrix.
 */
void **dealloc_matrix(int **matrix, int row) {
  int i;

  for (i = 0; i < row; i++)
    free(matrix[i]);
  free(matrix);
}

/*
 * Initialize a new matrix with the values of an existing one.
 */
void init_matrix(Image src, Image *dest) {
  dest->row = src.row;
  dest->col = src.col;
  dest->max_val = src.max_val;
  dest->img = alloc_matrix(src.row, src.col);  
}

/*
 * Read a PGM P5 file.
 */
int read_PGM(char *file_name, Image *data) {
  int i, j;
  FILE * pgm_fp;
  char version[3];
  
  if ((pgm_fp = fopen(file_name, "rb")) == NULL) {
    fprintf(stderr, "Can not open %s\n", file_name);
    return 1;
  }

  // Make sure the image is of the right version.
  fgets(version, sizeof(version), pgm_fp);
  if (strcmp(version, "P5")) {
    fprintf(stderr, "Invalid PGM format. P5 required.\n");
    return 1;
  }

  fgetc(pgm_fp);
  fscanf(pgm_fp, "%d %d", &data->col, &data->row);
  fscanf(pgm_fp, "%d", &data->max_val);
  fgetc(pgm_fp);

  data->img = alloc_matrix(data->row, data->col);
  
  if (data->max_val <= 255) {
    for (i = 0; i < data->row; i++) {
      for (j = 0; j < data->col; j++) {
        data->img[i][j] = fgetc(pgm_fp);
      }
    }
  }
  else {
    fprintf(stderr, "Error: Max value in source PGM is above 255.");
    return 1;
  }

  fclose(pgm_fp);
  
  return 0;
}

/*
 * Write a PGM P5 file.
 */
int write_PGM(char *file_name, Image out) {
  FILE * out_fp;
  int i, j;
  int lo;

  if ((out_fp = fopen(file_name, "wb")) == NULL) {
    fprintf(stderr, "Could not write output file: %s\n", file_name);
    return 1;
  }

  fprintf(out_fp, "P5\n");
  fprintf(out_fp, "%d %d\n", out.col, out.row);
  fprintf(out_fp, "%d\n", out.max_val);

  if (out.max_val <= 255) {
    for (i = 0; i < out.row; i++) {
      for (j = 0; j < out.col; j++) {
        lo = out.img[i][j] & 0x000000FF;
        fputc(lo, out_fp);
      }
    }
  }
  else {
    fprintf(stderr, "Error: Max value in output PGM is above 255.");
    return 1;
  }

  fclose(out_fp);

  return 0;
}

/*
 * Convolve.  Supports 3x3 filter matrices.
 */
void convolve(int filter[3][3], Image src, Image *result) {
  int row, col;

  // Loop through the image matrix
  for (row = 0; row < src.row; row++) {
    for (col = 0; col < src.col; col++) {
      int accum = 0;
      int i, j;

      // Loop through the 3x3 filter matrix
      for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
          int k;
    
          // If the filter needs to grab a value outside the bounds of the image
          // then give it a zero, otherwise give it the actual value.
          if (row + i < 0 || row + i > src.row - 1 ||
              col + j < 0 || col + j > src.col - 1)
            k = 0;
          else {
            k = src.img[row + i][col + j];
          }

          accum += k * filter[1 + i][1 + j];
        }
      }

      result->img[row][col] = accum;
    }
  }
}

/*
 * Calculate the gradient magnitude.
 */
void sobel_grad_mag(Image *G, Image G_y, Image G_x) {
  int i, j;

  for (i = 0; i < G_y.row; i++) {
    for (j = 0; j < G_y.col; j++) {
      int G_y_sq = G_y.img[i][j] * G_y.img[i][j];
      int G_x_sq = G_x.img[i][j] * G_x.img[i][j];
      
      G->img[i][j] = sqrt(G_y_sq + G_x_sq);
    }
  }
}

/*
 * Normalize an image.
 */
void normalize(Image *src) {
  int min = 0, max = 0;
  int i, j;
  int scale;

  // Determine the lower and upper bounds of the current values.
  for (i = 0; i < src->row; i++) {
    for (j = 0; j < src->col; j++) {
      if (src->img[i][j] < min)
        min = src->img[i][j];
      else if (src->img[i][j] > max)
        max = src->img[i][j];
    }
  }

  // Normalize the values to between 0 and 255.
  for (i = 0; i < src->row; i++) {
    for (j = 0; j < src->col; j++) {
      src->img[i][j] = (int) floor(255 * ((float) src->img[i][j] / (float) max));
    }
  }
}

/*
 * Main
 */
int main(int argc, char *argv[]) {
  int i, j;
  int sobel_y[3][3] = {{-1, -2, -1},
                       {0, 0, 0},
                       {1, 2, 1}};
  int sobel_x[3][3] = {{-1, 0, 1},
                       {-2, 0, 2},
                       {-1, 0, 1}};

  Image src;
  Image G_y;
  Image G_x;
  Image out;

  if (argc < 2) {
    fprintf(stderr, "Pass the name of a PGM P5 file.\n");
    return 1;
  }

  // Read source PGM file
  if (read_PGM(argv[1], &src))
    return 1;

  // Initialize all the matrices to be of the same size as the source image.
  init_matrix(src, &G_y);
  init_matrix(src, &G_x);
  init_matrix(src, &out);

  // Convolve the Sobel filters with the source image
  convolve(sobel_y, src, &G_y);
  convolve(sobel_x, src, &G_x);

  // Calculate the gradient magnitude and put it in out.
  sobel_grad_mag(&out, G_y, G_x);

  // Normalize values to 0-255
  normalize(&G_y);
  normalize(&G_x);
  normalize(&out);

  // Write final output PGMs
  if (write_PGM("g_y.pgm", G_y))
    return 1;
  if (write_PGM("g_x.pgm", G_x))
    return 1;
  if (write_PGM("out.pgm", out))
    return 1;

  // Free the matrices
  dealloc_matrix(src.img, src.row);
  dealloc_matrix(G_y.img, 4);
  dealloc_matrix(G_x.img, 4);
  dealloc_matrix(out.img, out.row);

  return 0;
}
