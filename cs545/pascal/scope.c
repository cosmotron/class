#include <stdio.h>
#include <stdlib.h>
#include "scope.h"
#include "symtab.h"

/* Initialize the stack with max_size as an upper bound on the number of
   scopes that can be pushed at any given time. */
scope_elem *scope_init(int max_size) {
  scope_elem *stack = (scope_elem *) malloc(sizeof(scope_elem));

  symtable **content;
  int i;

  content = (symtable **) malloc(sizeof(symtable*) * max_size);
  for (i = 0; i < max_size; i++)
    content[i] = (symtable *) malloc(sizeof(symtable));


  stack->content = content;
  stack->max_size = max_size;
  stack->top = -1;

  return stack;
}

void scope_push(scope_elem *stack, symtable *st) {
  stack->content[++stack->top] = st;
}

symtable *scope_pop(scope_elem *stack) {
  return stack->content[stack->top--];
}

/* Search the stack top-down */
st_node_t *search_scope_stack(scope_elem *stack, char * name) {
  symtable *current;
  st_node_t *n;
  int i;

  for (i = stack->top; i >= 0; --i) {
     current = stack->content[i];
     n = symtab_lookup(current, name);

     if (n != NULL) return n;
  }

  return NULL;
}
