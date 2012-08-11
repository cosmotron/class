#include <stdio.h>
#include <stdlib.h>
#include "arg_list.h"

/* Free argument list */
void arg_destroy(arg_elem *head) {
  if (head == NULL)
    return;

  fprintf(stderr, "destroying arg_head\n");

  arg_elem *curr, *tmp;

  curr = head->next;
  head->next = NULL;

  while (curr != NULL) {
    tmp = curr->next;
    free(curr);
    curr = tmp;
  }
}

/* Append to the end of an argument list */
arg_elem *arg_append(arg_elem *head, symtab_type type) {
  arg_elem *tmp;

  if (head == NULL) {
    //fprintf(stderr, "head is null\n");
    if ((head = (arg_elem *) malloc(sizeof(arg_elem))) == NULL) {
      printf("could not malloc\n");
      exit(1);
    }

    head->type = type;
    head->next = head;
  }
  else {
    //fprintf(stderr, "entered else, name = %s\n", name);
    tmp = head;

    while (tmp->next != head)
      tmp = tmp->next;

    if ((tmp->next = (arg_elem *)malloc(sizeof(arg_elem))) == NULL) {
      printf("coul not malloc\n");
      exit(1);
    }

    tmp = tmp->next;
    tmp->type = type;
    tmp->next = head;
  }

  //fprintf(stderr, "head->name before return: %s\n", head->name);

  return head;
}

/* Return the number of elements in a given list.
   Used for funciton/procedure semantics. */
int arg_list_size(arg_elem *head) {
  arg_elem *data = head;
  int size = 0;

  if (data != NULL) {
    do {
      size++;

      data = data->next;
    }
    while (data != head);
  }
  else
    fprintf(stderr, "print_arg_list: data is null\n");

  return size;
}

/* Returns zero if there are no differences in two argument lists
   Used for function/procedure semantics */
int arg_list_compare(arg_elem *one, arg_elem *two) {
  arg_elem *data_one = one, *data_two = two;
  int diff = 0;

  if (one == NULL || two == NULL)
    error("arg_list_compare: null comparison");

  do {
    //fprintf(stderr, "compare: one: %d, two: %d\n", data_one->type, data_two->type);
    if (data_one->type != data_two->type)
      diff++;

    data_one = data_one->next;
    data_two = data_two->next;
  }
  while (data_one != one);

  return diff;
}

/* Print an argument list (for debugging) */
void print_arg_list(arg_elem *head) {
  arg_elem *data = head;

  if (data != NULL) {
    do {
      fprintf(stderr, "arg_list, type: %d\n", data->type);

      data = data->next;
    }
    while (data != head);
  }
  else
    fprintf(stderr, "print_arg_list: data is null\n");

}
