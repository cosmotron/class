#include "gencode.h"

/* Globals */
// Register stack.  Push and pop array index of reg_name
int reg_stack[NUM_REGS];

// Stack pointer
int sp_reg_stack;

// The available registers
char *reg_name[] = {"%ebx", "%ecx", "%edx"};

// Counter to keep track of jump labels
int jump_counter = 0;

/* Print the fake main which will call foo, the "rea" main */
void print_fake_main() {
  fprintf(stdout, "\t.text\n");
  fprintf(stdout, ".globl _main\n");
  fprintf(stdout, "_main:\n");
  fprintf(stdout, "\tleal 4(%%esp), %%ecx\n");
  fprintf(stdout, "\tandl $-16, %%esp\n");
  fprintf(stdout, "\tpushl -4(%%ecx)\n");
  fprintf(stdout, "\tpushl %%ebp\n");
  fprintf(stdout, "\tmovl %%esp, %%ebp\n");
  fprintf(stdout, "\tpushl %%ecx\n");
  fprintf(stdout, "\tsubl $4, %%esp\n");
  fprintf(stdout, "\tcall _foo\n");
  fprintf(stdout, "\taddl $4, %%esp\n");
  fprintf(stdout, "\tpopl %%ecx\n");
  fprintf(stdout, "\tpopl %%ebp\n");
  fprintf(stdout, "\tleal -4(%%ecx), %%esp\n");
  fprintf(stdout, "\tret\n");
}

/* Print stack alignment code.  Slice off space from the stack based on
   stack_offset */
void prologue(char *func_name, int stack_offset) {
  if (func_name == NULL)
    func_name = "foo";

  fprintf(stdout, "\t.text\n");
  fprintf(stdout, ".globl _%s\n", func_name);
  fprintf(stdout, "_%s:\n", func_name);
  fprintf(stdout, "\tpushl %%ebp\n");
  fprintf(stdout, "\tmovl %%esp, %%ebp\n");

  if (stack_offset > 0)
    fprintf(stdout, "\tsubl $%d, %%esp\n", stack_offset * 4);
}

/* Stack cleanup code */
void epilogue() {
  fprintf(stdout, "\tleave\n");
  fprintf(stdout, "\tret\n");
}

/* Save the values of the registers.

   Limitation: it doesn't save the values of just those that were used.
 */
void save_register_state() {
  fprintf(stdout, "\tpushl %%ebx\n");
  fprintf(stdout, "\tpushl %%ecx\n");
  fprintf(stdout, "\tpushl %%edx\n");
}

/* Restore all the register values */
void restore_register_state() {
  fprintf(stdout, "\tpopl %%edx\n");
  fprintf(stdout, "\tpopl %%ecx\n");
  fprintf(stdout, "\tpopl %%ebx\n");
}

/* Generate code based on the statement grammar rule */
void statement_gencode(tree_t *t) {
  if (t == NULL) return;

  switch (t->type) {
    case ASSIGN_STMT:
      assign_stmt_gencode(t);
      break;
    case PROC_CALL:
      proc_stmt_gencode(t);
      break;
    case BEGIN_END:
      compound_stmt_gencode(t);
      break;
    case IF_THEN_ELSE_MATCHED:
    case IF_THEN_UNMATCHED:
    case IF_THEN_ELSE_UNMATCHED:
      if_then_gencode(t);
      break;
    case STMT_LIST:
      statement_gencode(t->left);
      statement_gencode(t->right);
      break;
    default:
      fprintf(stderr, "type is %d\n", t->type);
      error("Unsupported statement type in statement_gencode");
  }
}

/* Generate code based on the compound_stmt grammar rule */
void compound_stmt_gencode(tree_t *t) {
  if (t == NULL) return;

  switch (t->type) {
    case STMT_LIST:
      statement_gencode(t->left);
      statement_gencode(t->right);
      break;
    case BEGIN_END:
      compound_stmt_gencode(t->left);
      break;
    case ASSIGN_STMT:
    case PROC_CALL:
    case IF_THEN_ELSE_MATCHED:
    case IF_THEN_UNMATCHED:
    case IF_THEN_ELSE_UNMATCHED:
      statement_gencode(t);
      break;
    default:
      error("Unsupported compound statement type in compound_stmt_gencode");
  }
}

/* Assignment statment gencode: Run expr_gencode on the right side (expression)
   and then move the result (%ebx) into the left side */
void assign_stmt_gencode(tree_t *t) {
  tree_t *var = t->left;
  tree_t *expr = t->right;

  expr_gencode(expr);

  if (var->attr.sval->location == ST_LOCAL) {
    //if (var->attr.sval->index == 0)
    //  fprintf(stdout, "\tmovl %%ebx, -(%%ebp)\t// move value into variable\n");
    //else
      fprintf(stdout, "\tmovl %%ebx, -%d(%%ebp)\t// move value into variable (local)\n",
              (var->attr.sval->index * 4));
  }
  else if (var->attr.sval->location == ST_PARAMETER) {
    //if (var->attr.sval->index == 0)
    //  fprintf(stdout, "\tmovl %%ebx, (%%ebp)\t// move value into variable\n");
    //else
      fprintf(stdout, "\tmovl %%ebx, %d(%%ebp)\t// move value into variable (param)\n",
              (var->attr.sval->index * 4));
  }
  else
    error("Bad variable location");
}

/* Procedure gencode: hand off to expr_gencode() */
void proc_stmt_gencode(tree_t *t) {
  expr_gencode(t);
}

/* Function/Procedure call gencode: Sets up the arguments on the stac
   before calling */
void func_proc_call_gencode(tree_t *t, int offset) {
  if (t == NULL) return;

  switch (t->type) {
    /* Descend down the left and right children of there are multiple
       arguments */
    case EXPR_LIST:
      func_proc_call_gencode(t->left, offset++);
      func_proc_call_gencode(t->right, offset++);
      return;

    /* Handle expressions */
    case ADDOP_EXPR:
    case MULOP_EXPR:
      expr_gencode(t);
      break;

    /* Handle plain id/integer lookups */
    case IDENT:
      fprintf(stdout, "\tmovl -%d(%%ebp), %%ebx\t// (f/p call gencode) arg is id\n",
              (t->attr.sval->index * 4));
      break;
    case INUMBER:
      fprintf(stdout, "\tmovl $%d, %%ebx\t// (f/p call gencode) arg is int\n",
              t->attr.ival);
      break;

    default:
      error("unhandled type (func_proc_call_gencode)");
  }

  if (offset == 0)
    fprintf(stdout, "\tmovl %%ebx, (%%esp)\t// (f/p call gencode) set up arg on stack\n");
  else
    fprintf(stdout, "\tmovl %%ebx, %d(%%esp)\t// (f/p call gencode) set up arg on stack\n",
            (offset * 4));
}

/* if-then-else gencode: set up code and apply appropriate labels to the the
   consequent, else, and after everything. There are in-assembly comments for
   each line as well. */
void if_then_gencode(tree_t *t) {
  tree_t *expr = t->left;
  tree_t *then_stmt = t->right;
  tree_t *else_stmt = NULL;
  char *jump_type;
  int then_jump, else_jump, after_jump;

  assert (expr->type == RELOP_EXPR);
  assert (expr->left != NULL && expr->right != NULL);

  // else statment exists if not null
  if (t->children != NULL)
    else_stmt = t->children[1];

  /* if "else" exists, we need three labels, so increment label counter by three
     and give each section its unique label in ascending order.

     if "else" does not exist, only two labels are required. */
  if (else_stmt != NULL) {
    jump_counter += 3; // reserve space for jumps
    then_jump = jump_counter - 2;
    else_jump = jump_counter - 1;
    after_jump = jump_counter;
  }
  else {
    jump_counter += 2; // reserve space for jumps
    then_jump = jump_counter - 1;
    after_jump = jump_counter;
  }

  /* Hack? Handle simple relop expressions such as (1 = x), (x = 1), (1 <> 2).
     expr_gencode() will not move anything into %eax or %edx for comparison
     otherwise.
   */
  if (expr->left->type == INUMBER)
    fprintf(stdout, "\tmovl $%d, %%ebx\t// <<< (if then gencode) handle int\n",
            expr->left->attr.ival);
  else if (expr->left->type == IDENT) {
    if (expr->left->attr.sval->location == ST_LOCAL)
      fprintf(stdout, "\tmovl -%d(%%ebp), %%ebx\t// <<< (if then gencode) handle id\n",
              (expr->left->attr.sval->index * 4));
    else if (expr->left->attr.sval->location == ST_PARAMETER)
      fprintf(stdout, "\tmovl %d(%%ebp), %%ebx\t// <<< (if then gencode) handle id\n",
              (expr->left->attr.sval->index * 4) + 4);
  }
  else
    expr_gencode(expr->left);

  fprintf(stdout, "\tmovl %%ebx, %%edx\t// (if then gencode) move expr left to edx\n");
  fprintf(stdout, "\tpushl %%edx\t// (if then gencode) save edx for cmpl\n");

  if (expr->right->type == INUMBER)
    fprintf(stdout, "\tmovl $%d, %%ebx\t// <<< (if then gencode) handle int\n",
            expr->right->attr.ival);
  else if (expr->right->type == IDENT) {
    if (expr->right->attr.sval->location == ST_LOCAL)
      fprintf(stdout, "\tmovl -%d(%%ebp), %%ebx\t// <<< (if then gencode) handle id\n",
              (expr->right->attr.sval->index * 4));
    else if (expr->right->attr.sval->location == ST_PARAMETER)
      fprintf(stdout, "\tmovl %d(%%ebp), %%ebx\t// <<< (if then gencode) handle id\n",
              (expr->right->attr.sval->index * 4) + 4);
  }
  else
    expr_gencode(expr->right);

  fprintf(stdout, "\tmovl %%ebx, %%eax\t// (if then gencode) move expr right to eax\n");
  fprintf(stdout, "\tpopl %%edx\t// (if then gencode) restore edx for cmpl\n");

  fprintf(stdout, "\tcmpl %%eax, %%edx\t// (if then gencode) comparison\n");
  jump_type = determine_jump_condition(expr->attr.opval);
  fprintf(stdout, "\t%s .L%d\t// (if then gencode) jump to THEN\n", jump_type, then_jump);

  if (else_stmt != NULL)
    fprintf(stdout, "\tjmp .L%d\n", else_jump);
  else
    fprintf(stdout, "\tjmp .L%d\n", after_jump);

  fprintf(stdout, "// begin THEN\n");
  fprintf(stdout, ".L%d:\n", then_jump);
  statement_gencode(then_stmt);
  fprintf(stdout, "\tjmp .L%d\n", after_jump);
  fprintf(stdout, "// end THEN\n");

  if (else_stmt != NULL) {
    fprintf(stdout, "// begin ELSE\n");
    fprintf(stdout, ".L%d:\n", else_jump);
    statement_gencode(else_stmt);
    fprintf(stdout, "// end ELSE\n");
  }

  fprintf(stdout, ".L%d:\n", after_jump);
}

/* Return the appropriate jump operator based on the syntax tree opval */
char *determine_jump_condition(char *opval) {
  if (!strcmp(opval, "="))
    return "je";
  else if (!strcmp(opval, "<>"))
    return "jne";
  else if (!strcmp(opval, "<"))
    return "jl";
  else if (!strcmp(opval, "<="))
    return "jle";
  else if (!strcmp(opval, ">"))
    return "jg";
  else if (!strcmp(opval, ">="))
    return "jge";
  else
    error("Unknown boolean comparison (determine jmp condition)");
}

/* Mostly dragon book gencode with some extras for handling func calls */
void expr_gencode(tree_t *t) {
  int left_label, right_label;
  int tmp_register;

  if (t == NULL) return;

  /* If a function call is encountered, save state and set up the stack */
  if (t->type == FUNC_CALL) {
    //fprintf(stderr, "*** FUNCTION CALL GENCODE START ***\n");
    fprintf(stdout, "// >>> func call\n");

    save_register_state();
    func_proc_call_gencode(t->left, 0);

    fprintf(stdout, "\tcall _%s\n", t->attr.sval->name);

    restore_register_state();
    //fprintf(stderr, "*** FUNCTION CALL GENCODE END ***\n");
    return;
  }
  else if (t->type == PROC_CALL) {
    int format_offset = 0;
    fprintf(stdout, "// >>> proc call\n");

    if (!strcmp(t->attr.sval->name, "write") ||
        !strcmp(t->attr.sval->name, "read"))
      format_offset = 1;

    //save_register_state();
    func_proc_call_gencode(t->left, format_offset);

    if (!strcmp(t->attr.sval->name, "write")) {
      fprintf(stdout, "\tmovl $.LC0, (%%esp)\n");
      fprintf(stdout, "\tcall _printf\n");
    }
    else if (!strcmp(t->attr.sval->name, "read")) {
      fprintf(stdout, "\tmovl $.LC0, (%%esp)\n");
      fprintf(stdout, "\tcall _scanf\n");
    }
    else
      fprintf(stdout, "\tcall _%s\n", t->attr.sval->name);

    //restore_register_state();
    return;
  }


  /* case 0 */
  if (t->left == NULL && t->right == NULL) {
    if (t->label == 1) {
      if (t->type == INUMBER)
        fprintf(stdout, "\tmovl $%d, %s\t// <<< expr_gencode case 0 (int)\n",
                t->attr.ival, reg_name[top_reg_stack()]);
      else if (t->type == IDENT) {
        if (t->attr.sval->location == ST_LOCAL)
          fprintf(stdout, "\tmovl -%d(%%ebp), %s\t// <<< expr_gencode case 0 (id)\n",
                  (t->attr.sval->index * 4),
                  reg_name[top_reg_stack()]);
        else if (t->attr.sval->location == ST_PARAMETER)
          fprintf(stdout, "\tmovl %d(%%ebp), %s\t// <<< expr_gencode case 0 (id)\n",
                  (t->attr.sval->index * 4) + 4,
                  reg_name[top_reg_stack()]);
      }
    }
  }
  else {
    left_label = t->left->label;
    right_label = t->right->label;

    /* case 1 */
    if (right_label == 0) {
      expr_gencode(t->left);

      if (t->right->type == INUMBER)
        fprintf(stdout, "\t%s $%d, %s\t// <<< expr_gencode case 1 (int)\n",
                operator_name(t->attr.opval), t->right->attr.ival, reg_name[top_reg_stack()]);
      else if (t->right->type == IDENT) {
        if (t->right->attr.sval->location == ST_LOCAL)
          fprintf(stdout, "\t%s -%d(%%ebp), %s\t// <<< expr_gencode case 1 (id)\n",
                  operator_name(t->attr.opval),
                  (t->right->attr.sval->index * 4),
                  reg_name[top_reg_stack()]);
        else if (t->right->attr.sval->location == ST_PARAMETER)
          fprintf(stdout, "\t%s %d(%%ebp), %s\t// <<< expr_gencode case 1 (id)\n",
                  operator_name(t->attr.opval),
                  (t->right->attr.sval->index * 4) + 4,
                  reg_name[top_reg_stack()]);
      }
    }
    /* case 2 */
    else if (1 <= left_label && left_label < right_label && left_label < NUM_REGS) {
      swap_reg_stack();
      expr_gencode(t->right);
      tmp_register = pop_reg_stack();
      expr_gencode(t->left);

      fprintf(stdout, "\t%s %s, %s\t// <<< expr_gencode case 2\n",
              operator_name(t->attr.opval), reg_name[tmp_register], reg_name[top_reg_stack()]);

      push_reg_stack(tmp_register);
      swap_reg_stack();
    }
    /* case 3 */
    else if (1 <= right_label && right_label <= left_label && right_label < NUM_REGS) {

      expr_gencode(t->left);
      tmp_register = pop_reg_stack();
      expr_gencode(t->right);

      if (t->left->type == FUNC_CALL || t->right->type == FUNC_CALL) {
        fprintf(stdout, "// ### func call as a child, look in eax\n");
        fprintf(stdout, "\t%s %%eax, %s\t// <<< expr_gencode case 3\n",
                operator_name(t->attr.opval), reg_name[tmp_register]);
      }
      else
        fprintf(stdout, "\t%s %s, %s\t// <<< expr_gencode case 3\n",
                operator_name(t->attr.opval), reg_name[top_reg_stack()], reg_name[tmp_register]);

      push_reg_stack(tmp_register);
    }
    /* case 4 */
    else {
      error("Panic mode in gencode: not enough registers (it's NP complete)");
    }
  }
}

char *operator_name(char *opval) {
  if (opval == NULL) return "BADBAD";

  if (!strcmp("+", opval))
    return "addl";
  else if (!strcmp("-", opval))
    return "subl";
  else if (!strcmp("*", opval))
    return "imull";
  else if (!strcmp("/", opval))
    return "idivl";
  else {
    fprintf(stderr, "opval: %s\n", opval);
    error("Bad operator name. (operator_name)");
  }
}

void init_reg_stack() {
  int i;
  for (i = 0; i < NUM_REGS; i++)
    reg_stack[i] = i;
  sp_reg_stack = 0;
}

int pop_reg_stack() {
  assert(sp_reg_stack < NUM_REGS);
  return reg_stack[sp_reg_stack++];
}

void push_reg_stack(int x) {
  assert(sp_reg_stack > 0);
  reg_stack[--sp_reg_stack] = x;
}

int top_reg_stack() {
  return reg_stack[sp_reg_stack];
}

void swap_reg_stack() {
  assert(sp_reg_stack < NUM_REGS - 1);
  reg_stack[sp_reg_stack] ^= reg_stack[sp_reg_stack + 1];
  reg_stack[sp_reg_stack + 1] ^= reg_stack[sp_reg_stack];
  reg_stack[sp_reg_stack] ^= reg_stack[sp_reg_stack + 1];
}
