#ifndef SYMTAB_H
#define SYMTAB_H
#include "decl_list.h"
#include "arg_list.h"
#include "type.h"

#define EOS '\0'
#define PRIME 211

typedef enum location_enum {
  ST_LOCAL,
  ST_PARAMETER
} location_e;

/*
 * Symbol Table Entry
 *
 * A single element in a symbol table.
 * Is linkable for hash collisions.
 *
 * name: identifier name
 * class: Variable, Function, Procedure
 * type: Integer, Real, Boolean, Int Array, Real Array
 * location: Local, Parameter
 * arg_type_list: list of the argument types in the declared order
 * num_of_args: number of arguments
 * index: memory offset for gencode
 */
typedef struct st_node_s {
  char *name;
  symtab_type class;
  type_struct *type;
  location_e location;
  arg_elem *arg_type_list;
  int num_of_args;
  int index;

  struct st_node_s *next;
} st_node_t;

/*
 * Symbol Table
 *
 * name: the name of the function that Table is describing
 * has_return: true if the function has a return assignment
 * entries: number of entries in the table.  Used to determine st_node_t->index
 * Table: hash table for symbol table entries
 */
typedef struct symtab_s {
  char *name;
  int has_return;
  int entries;
  st_node_t *Table[PRIME];
} symtable;

st_node_t *symtab_lookup(symtable *st, char *name);
st_node_t *symtab_insert(symtable *st, char *name, symtab_type class,
                         type_struct *type, location_e location, int symtab_index);
int hashpjw(char *s);

symtable *setup_input_output(symtable *st, decl_elem *head);

int calculate_stack_offset(symtable *st);

void print_symtab(symtable *st);
void print_entry(st_node_t *n, int hash_index);

#endif
