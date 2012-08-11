#ifndef GENCODE_H
#define GENCODE_H

#include <stdio.h>
#include "tree.h"

/* Number of available registers for expr_gencode() */
#define NUM_REGS 3

/*
 * Code Generation Functions
 *
 * These functions are passed a labeled tree and generate assembly based on it.
 *
 * A register stack is also implmented here.
 *
 * Limitations:
 * - No loop support
 * - No real support
 * - No array (int/real) support
 * - No non-local access
 * - No expr_gencode() panic mode support
 */

void print_fake_main();
void prologue(char *func_name, int stack_offset);
void epilogue();
void save_register_state();
void restore_register_state();
void statement_gencode(tree_t *t);
void compound_stmt_gencode(tree_t *t);
void assign_stmt_gencode(tree_t *t);
void proc_stmt_gencode(tree_t *t);
void function_call_gencode(tree_t *t, int offset);
void if_then_gencode(tree_t *t);
char *determine_jump_condition(char *opval);
void expr_gencode(tree_t *t);

char *operator_name(char *opval);

void init_reg_stack();
int pop_reg_stack();
void push_reg_stack(int x);
int top_reg_stack();
void swap_reg_stack();

#endif
