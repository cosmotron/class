#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "symtab.h"
#include "semantics.h"
#include "type.h"

/* Symbol table lookup for the passed symbol table */
st_node_t *symtab_lookup(symtable *st, char *name) {
  int hash_index = hashpjw(name);
  st_node_t *n = st->Table[hash_index];

  while (n != NULL) {
    //fprintf(stderr, ">>> symtab name compare: %s | %s\n", name, n->name);
    if (!strcmp(name, n->name))
      return n;
    n = n->next;
  }

  return NULL;
}

/* Insert a new element into the passed symbol table */
st_node_t *symtab_insert(symtable *st, char *name, symtab_type class,
                         type_struct *type, location_e location, int symtab_index) {
  int hash_index = hashpjw(name);
  st_node_t *head = st->Table[hash_index];
  st_node_t *n;

  /* If it exists in the current scope already, redeclaration semantic error */
  if ((n = symtab_lookup(st, name)) != NULL)
    sem_error(VAR_REDECLARED);

  n = (st_node_t*) malloc(sizeof(st_node_t));

  assert(n != NULL);
  n->name = strdup(name);
  n->class = class;
  n->type = type;
  n->location = location;

  /* If entry is for a function, allocate some space for the argument type list
     that pascal.y will assign later. This assignment can not happen yet
     because the argument list hasn't been encountered yet by bison */
  if (class == FUNCTION_CLASS)
    n->arg_type_list = calloc(1, sizeof(arg_elem));
  else
    n->arg_type_list = NULL;

  n->index = symtab_index;
  n->next = NULL;

  /* Insert new entry at the head of the bucket if there isn't anything there yet */
  if (head != NULL)
    n->next = head;
  st->Table[hash_index] = n;

  return n;
}

/* hashpjw */
int hashpjw(char *s) {
  char* p;
  unsigned h = 0, g;

  for (p = s; *p != EOS; p++) {
    h = (h << 4) + (*p);
    if (g = h & 0xf0000000) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }

  return h % PRIME;
}

/* Creates the input/output entries in the symbol table for read/write
   procedures.

   Limitation: read (scanf) is not supported in gencode */
symtable *setup_input_output(symtable *st, decl_elem *head) {
  decl_elem *data = head;
  st_node_t *st_node;
  arg_elem *arg_head = NULL;

  if (head == NULL) return;

  do {
    if (!strcmp(data->name, "input")) {
      type_struct *t = calloc(1, sizeof(type_struct));
      (st->entries)++;
      st_node = symtab_insert(st, "read", PROCEDURE_CLASS, t, ST_LOCAL, st->entries);
    }
    else if (!strcmp(data->name, "output")) {
      type_struct *t = calloc(1, sizeof(type_struct));
      (st->entries)++;
      st_node = symtab_insert(st, "write", PROCEDURE_CLASS, t, ST_LOCAL, st->entries);
      arg_head = arg_append(arg_head, INTEGER_TYPE);
      st_node->arg_type_list = arg_head;
      st_node->num_of_args = 1;

      // print out the label
      fprintf(stdout, ".LC0:\n");
      fprintf(stdout, "\t.ascii \"int: %%d\\12\\0\"\n");
    }

    data = data->next;
  }
  while (data != head);

  return st;
}

/* Hack.  Calculates offset by counting the number of variable class entries in
   the symbol table plus the number of arguments in function/procedure class
   entries. If the procedure is "write", then add three (2 for the format
   argument and 1 for the integer argument to printf)*/
int calculate_stack_offset(symtable *st) {
  st_node_t *n;
  int i, total = 0;

  for (i = 0; i < PRIME; i++) {
    n = st->Table[i];
    if (n != NULL) {
      if (n->class == VAR_CLASS)
        total++;
      else if (n->class == PROCEDURE_CLASS) {
        if (!strcmp(n->name, "write"))
          total += 3;
        else
          total += n->num_of_args;
      }
    }
  }

  return total;
}

/* Print a symbol table. For debugging.  Enable with -DPRINT_SYMBOL_TABLE */
void print_symtab(symtable *st) {
  st_node_t *n;
  int i;

  fprintf(stderr, "*** SYMBOL TABLE : %s ***\n", st->name);
  for (i = 0; i < PRIME; i++) {
    n = st->Table[i];
    if (n != NULL) {
      print_entry(n, i);
    }
  }
}

void print_entry(st_node_t *n, int hash_index) {
  fprintf(stderr, "[(hash) %i\t(name) %s\t(type) %d\t(location) %d\t(index) %d]\n",
          hash_index, n->name, n->type->name, n->location, n->index);

  if (n->class == FUNCTION_CLASS) {
    fprintf(stderr, "### Arg List Types for %s ###\n", n->name);
    if (n->arg_type_list != NULL)
      print_arg_list(n->arg_type_list);
    else
      fprintf(stderr, "problem!!\n");
  }

  if (n->next != NULL)
    print_entry(n->next, hash_index);
}
