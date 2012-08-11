#ifndef TREE_H
#define TREE_H

#include "symtab.h"

typedef enum node_e {
  BEGIN_END,
  STMT_LIST,
  IF_THEN_ELSE_MATCHED,
  WHILE_DO,
  FOR_LOOP,
  IF_THEN_UNMATCHED,
  IF_THEN_ELSE_UNMATCHED,
  ASSIGN_STMT,
  IDENT,
  ID_ARRAY,
  PROC_CALL,
  FUNC_CALL,
  EXPR_LIST,
  RELOP_EXPR,
  SIGN_TERM,
  ADDOP_EXPR,
  MULOP_EXPR,
  RNUMBER,
  INUMBER,
  NOT_FACTOR
} node_type;

/*
 * Syntax Tree
 *
 * Each element of the syntax tree.  Supports 0, 1, 2, ..., n children.
 * Also contains type checking code.
 *
 * type: node type, from the node_type enumeration
 * attr: value information.  changes depending on type
 * num_of_children: the number of children a particular node has
 * label: rank for gencode
 * left, right: pointers to left and right children
 * children: if num_of_children > 2, insert them into this double pointer
 */
typedef struct tree_s {
  node_type type;

  union {
    int ival;        /* attribute value of INUMBER */
    float rval;      /* attribute value of RNUMBER */
    st_node_t *sval; /* attribute value of IDENT */
    char *opval;      /* attribute of RELOP, ADDOP, MULOP */
  } attr;

  int num_of_children;
  int label;

  struct tree_s *left, *right;
  struct tree_s **children;
} tree_t;


tree_t *mktree(node_type type, int count, ...);
void label_tree(tree_t *t, int left);
void print_tree(tree_t *t, int spaces);

symtab_type type_check(tree_t *t);

#endif
