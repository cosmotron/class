#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "tree.h"
#include "symtab.h"
#include "semantics.h"

/* Insert a node into the tree.  "..." accepts count number of tree nodes */
tree_t *mktree(node_type type, int count, ...) {
  va_list ap;
  tree_t *t = (tree_t *) malloc(sizeof(tree_t));
  int i;

  t->left = t->right = NULL;
  t->type = type;
  t->num_of_children = count;

  if (count > 2)
    t->children = malloc(sizeof(*t->children) * count);

  va_start(ap, count);
  for (i = 0; i < count; i++) {
    if (i == 0)
      t->left = va_arg(ap, tree_t*);
    else if (i == count - 1)
      t->right = va_arg(ap, tree_t*);
    else
      t->children[i] = va_arg(ap, tree_t*);
  }
  va_end(ap);

  return t;
}

/* Label the tree based on each node's rank.  Algorithm from the dragon book */
void label_tree(tree_t *t, int left) {
  int left_label, right_label;

  if (t == NULL) return;

  if (t->left == NULL && t->right == NULL)
    t->label = left;
  else if (t->type == FUNC_CALL || t->type == PROC_CALL) {
    label_tree(t->left, 1);
    t->label = 1;
  }
  else {
    label_tree(t->left, 1);
    label_tree(t->right, 0);

    left_label = t->left->label;
    right_label = t->right->label;

    if (left_label == right_label)
      t->label = left_label + 1;
    else
      t->label = (left_label < right_label ? right_label : left_label);
  }
}

/* Print the tree.  For debugging.  Enable with -DPRINT_SYNTAX_TREE */
void print_tree(tree_t *t, int spaces) {
  int i, j;

  if (t == NULL) return;

  for (i = 0; i < spaces; ++i)
    fprintf(stderr, " ");

  fprintf(stderr, "[%d]", t->label);

  switch (t->type) {
    case BEGIN_END:
      fprintf(stderr, "[BEGIN_END]\n");
      break;
    case STMT_LIST:
      fprintf(stderr, "[STMT_LIST]\n", t->attr.ival);
      break;
    case IF_THEN_ELSE_MATCHED:
      fprintf(stderr, "[IF_THEN_ELSE_MATCHED]\n");
      break;
    case WHILE_DO:
      fprintf(stderr, "[WHILE_DO]\n");
      break;
    case FOR_LOOP:
      fprintf(stderr, "[FOR_LOOP]\n");
      break;
    case IF_THEN_UNMATCHED:
      fprintf(stderr, "[IF_THEN_UNMATCHED]\n");
      break;
    case IF_THEN_ELSE_UNMATCHED:
      fprintf(stderr, "[IF_THEN_ELSE_UNMATCHED]\n");
      break;
    case ASSIGN_STMT:
      fprintf(stderr, "[ASSIGN_STMT]\n");
      break;
    case IDENT:
      if (t->attr.sval == NULL)
        fprintf(stderr, "[IDENT: NULL]\n");
      else
        fprintf(stderr, "[IDENT: %s: %d]\n", t->attr.sval->name, t->attr.sval->type->name);
      break;
    case ID_ARRAY:
      fprintf(stderr, "[ID_ARRAY]\n");
      break;
    case PROC_CALL:
      fprintf(stderr, "[PROC_CALL]\n");
      break;
    case FUNC_CALL:
      fprintf(stderr, "[FUNC_CALL]\n");
      break;
    case EXPR_LIST:
      fprintf(stderr, "[EXPR_LIST]\n");
      break;
    case RELOP_EXPR:
      fprintf(stderr, "[RELOP_EXPR: %s]\n", t->attr.opval);
      break;
    case SIGN_TERM:
      fprintf(stderr, "[SIGN_TERM]\n");
      break;
    case ADDOP_EXPR:
      fprintf(stderr, "[ADDOP_EXPR: %s]\n", t->attr.opval);
      break;
    case MULOP_EXPR:
      fprintf(stderr, "[MULOP_EXPR: %s]\n", t->attr.opval);
      break;
    case RNUMBER:
      fprintf(stderr, "[RNUMBER: %f]\n", t->attr.rval);
      break;
    case INUMBER:
      fprintf(stderr, "[INUMBER: %d]\n", t->attr.ival);
      break;
    case NOT_FACTOR:
      fprintf(stderr, "[NOT_FACTOR]\n");
      break;
    default:
      fprintf(stderr, "unknown tree type (%d)\n", t->type);
      exit(1);
  }

  print_tree(t->left, spaces + 4);
  if (t->num_of_children > 2) {
    for (j = 0; j < t->num_of_children; j++) {
      print_tree(t->children[j], spaces + 4);
    }
  }
  print_tree(t->right, spaces + 4);
}

/* Given a tree, determine its type. */
symtab_type type_check(tree_t *t) {
  symtab_type left_type, right_type;

  assert(t != NULL);
  switch(t->type) {
    /* ID or Func, return type from its symbol table entry */
    case IDENT:
    case FUNC_CALL:
      assert(t->attr.sval != NULL);
      return (t->attr.sval->type)->name;

    /* Array, look up its symbol table entry (left child is ID) */
    case ID_ARRAY:
      assert(t->attr.sval != NULL);
      left_type = type_check(t->left);

      if ((t->attr.sval->type)->name == ARRAY_TYPE_INT &&
          left_type == INTEGER_TYPE)
        return INTEGER_TYPE;
      else if ((t->attr.sval->type)->name == ARRAY_TYPE_REAL &&
               left_type == REAL_TYPE)
        return REAL_TYPE;
      sem_error(TYPE_MISMATCH_ASSN);

    /* Basic types */
    case RNUMBER:
      return REAL_TYPE;
    case INUMBER:
      return INTEGER_TYPE;

    /* Not, make sure left (expression) is boolean */
    case NOT_FACTOR:
      left_type = type_check(t->left);

      if (left_type == BOOLEAN_TYPE)
        return BOOLEAN_TYPE;
      sem_error(TYPE_NON_BOOLEAN);

    /* Relational op, make sure each side is boolean */
    case RELOP_EXPR:
      left_type = type_check(t->left);
      right_type = type_check(t->right);

      if (left_type == right_type)
        return BOOLEAN_TYPE;
      sem_error(TYPE_NON_BOOLEAN);
    /* addop/mulop, make sure types match.  does not allow for type casting */
    case ADDOP_EXPR:
    case MULOP_EXPR:
      left_type = type_check(t->left);
      right_type = type_check(t->right);

      if (left_type == right_type)
        return left_type;
      sem_error(TYPE_MISMATCH_EXPR);
  }
}

