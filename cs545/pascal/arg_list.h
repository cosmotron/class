#ifndef ARG_LIST_H
#define ARG_LIST_H

#include "type.h"

/*
 * Argument Type List
 *
 * A linked list of symbol table types.
 */
typedef struct arg_elem_s {
  symtab_type type;

  struct arg_elem_s *next;
} arg_elem;

void arg_destroy(arg_elem *head);
arg_elem *arg_append(arg_elem *head, symtab_type type);
int arg_list_size(arg_elem *head);
int arg_list_compare(arg_elem *one, arg_elem *two);

void print_arg_list(arg_elem *head);

#endif
