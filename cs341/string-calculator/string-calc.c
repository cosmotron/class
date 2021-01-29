/*
 * Ryan Lewis - 0147687
 * CS341 - Assignment 2
 *
 * String Calculator
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8
#define CATOP 257
#define REPOP 258
#define IDENTIFIER 259


void match(int token);
void error(char * message);
char * expr();
char * term();
char * repeat();


int current_token;
char * current_attribute;

int main() {
  char * value;

  while (1) {
    printf(">> ");
    current_token = get_token();
    value = expr();
    fprintf(stderr, "\n= %s\n", value);
  }
}

char * expr() {
  char * value = term();

  while(1) {
    if (current_token == CATOP) {
      match(CATOP);
      
      int value_len = strlen(value);
      char * term_value = term();
      int term_value_len = strlen(term_value);
      char * temp = (char *) malloc(sizeof(char) * (value_len + term_value_len));
      
      temp = strcat(value, term_value);
      value = temp;
    }
    else
      return value;
  }
}

char * term() {
  char * value = repeat();

  int i;
  while (1) {
    if (current_token == REPOP) {
      match(REPOP);

      int value_len = strlen(value);
      int num_to_repeat = atoi(repeat());
      char * temp = (char *) malloc(sizeof(char) * (value_len * num_to_repeat));

      strcpy(temp, value);

      int i = 1;
      for ( ;i < num_to_repeat; i++)
        temp = strcat(temp, value);
      
      value = temp;
    }
    else
      return value;
  }
}

char * repeat() {
  char * value;
  if (current_token == '(') {
    match('(');
    value = expr();
    match(')');
    return value;
  }
  else if (current_token == IDENTIFIER) {
    match(IDENTIFIER);
    value = current_attribute;
    return value;
  }
  else
    error("ERROR IN REPEAT");
}

void match(int token) {
  if (current_token == token)
    current_token = get_token();
  else
    error("Unexpected token");
}

void error(char * message) {
  fprintf(stderr, "ERROR: %s\n", message);
  exit(1);
}

int get_token() {
  int c;
  int buffer_size;
  char * buffer;

 begin:
  switch (c = getchar()) {
  case '(':
  case ')':
    return c;

  case '.':
    return CATOP;

  case '^':
    return REPOP;

  default:
    if (isdigit(c)) {
      buffer_size = BUFFER_SIZE;
      buffer = (char *) calloc(buffer_size, sizeof(char));
      ungetc(c, stdin);

      int i = 0;
      while (isdigit(c)) {
        for ( ; i < buffer_size && isdigit(c = getchar()); i++)
          buffer[i] = c;

        if (i >= buffer_size && isdigit(c)) {
          buffer_size *= 2;
          buffer = (char *) realloc(buffer, sizeof(char) * buffer_size);
        }
      }

      ungetc(c, stdin);
      current_attribute = buffer;
      return IDENTIFIER;
    }
    if (c == ' ' || c == '\t') {
      while (c == ' ' || c == '\t') { c = getchar(); }
      ungetc(c, stdin);
      goto begin;
    }
    else if (c == '\n')
      return c;
    else
      return c;
  }
}
