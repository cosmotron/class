#ifndef TYPE_H
#define TYPE_H

#include "assert.h"

/*
 * Type Structure
 *
 * Used to keep track of a symbol table entry's class, type, and if it's an
 * array, its lower and upper bounds.
 */
typedef enum symtab_type_e {
  ARRAY_TYPE_INT,
  ARRAY_TYPE_REAL,
  INTEGER_TYPE,
  REAL_TYPE,
  BOOLEAN_TYPE,
  VAR_CLASS,
  FUNCTION_CLASS,
  PROCEDURE_CLASS
} symtab_type;

typedef struct type_struct_t {
  symtab_type name;
  int lb;
  int ub;
} type_struct;

#endif
