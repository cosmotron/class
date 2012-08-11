#include <stdio.h>
#include "semantics.h"

/* if_then:
 * - tree is of type boolen
 */
void if_then_semantics(tree_t *t) {
  if (type_check(t) != BOOLEAN_TYPE)
    sem_error(TYPE_NON_BOOLEAN);
}

/* while:
 * - tree is of type boolen
 */
void while_semantics(tree_t *t) {
  if (type_check(t) != BOOLEAN_TYPE)
    sem_error(TYPE_NON_BOOLEAN);
}

/*
 * function:
 * - correct number of arguments
 * - correct argument order and types
 * - return is checked in pascal.y
 */
void function_semantics(st_node_t *st_node, tree_t *t) {
  arg_elem *head = NULL;

  /* Create a list out of the EXPR_LIST tree (t) */
  head = expr_list_eval(head, t);

  if (st_node->num_of_args != arg_list_size(head))
    sem_error(WRONG_NUMBER_OF_ARGS);

  /* Compare arg_lists */
  if (arg_list_compare(st_node->arg_type_list, head) != 0)
    sem_error(WRONG_ARG_TYPE);
}

/*
 * procedure:
 * - correct number of arguments
 * - correct argument order and types
 */
void procedure_semantics(st_node_t *st_node, tree_t *t) {
  arg_elem *head = NULL;

  /* Create a list out of the EXPR_LIST tree (t) */
  head = expr_list_eval(head, t);

  if (st_node->num_of_args != arg_list_size(head))
    sem_error(WRONG_NUMBER_OF_ARGS);

  /* Compare arg_lists */
  if (arg_list_compare(st_node->arg_type_list, head) != 0)
    sem_error(WRONG_ARG_TYPE);
}

/*
 * Turn a tree into a list for easy of comparison
 */
arg_elem *expr_list_eval(arg_elem *head, tree_t *t) {
  if (t == NULL) return head;

  if (t->type == EXPR_LIST) {
    head = expr_list_eval(head, t->left);
    head = expr_list_eval(head, t->right);
  }
  else
    head = arg_append(head, type_check(t));
}

/*
 * array:
 * - integer bounds
 * - index is within the declared bounds
 */
void array_semantics(st_node_t *st_node, tree_t *t) {
  int index, lb, ub;

  if (type_check(t) != INTEGER_TYPE)
    sem_error(ARRAY_NON_INTEGER_BOUNDS);

  /* Make sure index isn't out of bounds */
  if (t->type == INUMBER) {
    index = t->attr.ival;
    lb = st_node->type->lb;
    ub = st_node->type->ub;

    if (index < lb || index > ub)
      sem_error(ARRAY_INDEX_OUT_OF_BOUNDS);
  }
}

/* Print out the appropriate error message based on the id */
void sem_error(sem_error_id id) {
  char *msg;

  switch(id) {
    case VAR_UNDECLARED:
      msg = "Variable is undeclared"; break;
    case VAR_REDECLARED:
      msg = "Variable has been redeclared"; break;
    case WRONG_NUMBER_OF_ARGS:
      msg = "Wrong number of arguments"; break;
    case WRONG_ARG_TYPE:
      msg = "Wrong argument type"; break;
    case NO_RETURN_STMT:
      msg = "Function has no return statement"; break;
    case NON_LOCAL_MODIFICATION:
      msg = "Function tried to modify a non-local variable"; break;
    case BAD_RETURN_TYPE:
      msg = "Function did not return an integer or a real"; break;
    case ARRAY_INDEX_OUT_OF_BOUNDS:
      msg = "Array index is out of bounds"; break;
    case ARRAY_NON_INTEGER_BOUNDS:
      msg = "Array has non-integer bounds"; break;
    case TYPE_MISMATCH_EXPR:
      msg = "Type mismatch in expression"; break;
    case TYPE_MISMATCH_ASSN:
      msg = "Type mismatch in assignment"; break;
    case TYPE_NON_BOOLEAN:
      msg = "Expression is non-boolean"; break;
    default:
      msg = "UNKNOWN ERROR ID";
      fprintf(stderr, "code: %d\n", id);
  }

  fprintf(stderr, "[ERROR @ line: %d]: %s\n", yylineno, msg);
  exit(1);
}
