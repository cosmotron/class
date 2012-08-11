#ifndef DECL_LIST_H
#define DECL_LIST_H

/*
 * Declaration List
 *
 * A linked list of declared variable names
 *
 * Code here is functionally equivalent to arg_elem.h, this just keeps
 * track of a name rather than a type
 */
typedef struct decl_elem_s {
  char *name;

  struct decl_elem_s *next;
} decl_elem;

//void decl_init(decl_elem *d);
void decl_destroy(decl_elem *head);
decl_elem *decl_append(decl_elem *head, char *name);

#endif
