#ifndef SCOPE_H
#define SCOPE_H

#include "symtab.h"

/*
 * Scope: The Symbol Table Stack
 *
 * Push and pop take a symbol table and then modify the content member
 * appropriately.
 *
 * The search function will search through all scopes from top-down to see if
 * the given name can be found.  Since it's a stack, the same identifier can be
 * used in multiple scopes and the top-most will be returned first.
 */
typedef struct scope_elem_s {
  symtable **content;
  int max_size;
  int top;
} scope_elem;

scope_elem *scope_init(int max_size);
void scope_push(scope_elem *stack, symtable *st);
symtable *scope_pop(scope_elem *stack);
st_node_t *search_scope_stack(scope_elem *stack, char * name);

#endif
