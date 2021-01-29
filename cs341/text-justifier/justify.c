/*
 * Ryan Lewis - 0147687
 * CS341 - Assignment 1
 *
 * Text Justifier
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8

char * file_reader(char * in_file);
int last_space_finder(char * buffer);

int main(int argc, char ** argv) {
  // Make sure 3 arguments have been passed.
  if (argc != 4) {
    printf("usage: justify [INPUT] [OUTPUT] [WIDTH]\n");
    return 1;
  }

  char * in_file = argv[1];
  char * out_file = argv[2];
  int width = atoi(argv[3]);

  char * file_buffer;

  file_buffer = file_reader(in_file);

  FILE * file_out = fopen(out_file, "w");

  int file_length = strlen(file_buffer);
  char * line_buffer = calloc(width + 1, sizeof(char));

  int char_counter = width;
  char * left_off = & file_buffer[0];
  int last_space = 0;

  // Loop while the number of lines is less than the length of the file.
  while (char_counter < file_length) {
    strncpy(line_buffer, left_off, width);

    int i, j, k, first_space = 0;
    // Get the number of positions back from the end of the line that the last space appears
    int last_space = strlen(line_buffer) - last_space_finder(line_buffer) - 1;
    
    while (line_buffer[first_space] == ' ')
      first_space++;
    
    // Print the first word.
    for (i = first_space; i < width; i++)
      if (line_buffer[i] != ' ') { fprintf(file_out, "%c", line_buffer[i]); } else { break; }
    
    
    // Print the number of spaces to pad out
    for (j = (i + 1); j < (i + 3) + last_space; j++)
      fprintf(file_out, " ");
    
    // Print the rest of the line without the split word at the end
    for (k = (i + 1); k < (width - last_space -1); k++)
      fprintf(file_out, "%c", line_buffer[k]);
    
    fprintf(file_out, "\n");
    left_off = & file_buffer[char_counter] - last_space;
    char_counter += width - last_space;
  }

  // The previous loop will stop short since the last line is less than
  //   the length of a single line.  This will print it out.
  int l;
  for (l = (char_counter - width); l < file_length; l++)
    fprintf(file_out, "%c", file_buffer[l]);
  
  fprintf(file_out, "\n");

  fclose(file_out);
}

/* 
 * file_reader
 *   Return a char array of the contents of a file.
 *   Will also turn new lines into spaces to give a continuous
 *     array.
 *   Will dynamically increase the buffer size.
 */
char * file_reader(char * in_file) {
  FILE * file = fopen(in_file, "r");
  int buffer_size = BUFFER_SIZE;
  char * buffer = (char *) calloc(buffer_size, sizeof(char));

  int c;
  int i = 0;
  while (c != EOF) {
    for ( ; i < buffer_size && ((c = fgetc(file)) != EOF); i++)
      if (c == '\n') { buffer[i] = ' '; } else { buffer[i] = c; }
    
    if (i >= buffer_size && (c != EOF)) {
      buffer_size *= 2;
      buffer = (char *) realloc(buffer, sizeof(char) * buffer_size);
    }
  }

  fclose(file);
  return buffer;
}

/* 
 * last_space_finder
 *   Returns the position of the last space in a char array.
 */
int last_space_finder(char * buffer) {
  int len = strlen(buffer);
  int pos = 0;
  int i;

  for (i = 0; i < len; i++)
    if (buffer[i] == ' ')
      pos = i;

  return pos;
}
