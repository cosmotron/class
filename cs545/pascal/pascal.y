%{
#define YYDEBUG 1
#define YYERROR_VERBOSE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "scope.h"
#include "decl_list.h"
#include "arg_list.h"
#include "tree.h"
#include "symtab.h"
#include "type.h"
#include "semantics.h"
#include "gencode.h"

void add_decl_to_symtab(symtab_type class, type_struct *type, location_e location);

/* Symbol table entry */
st_node_t *st_node;

/* Declaration list head node (linked list) */
decl_elem *head = NULL, *data;

/* Function/Procedure argument list head node (linked list) */
arg_elem *arg_head = NULL;

/* Symbol table stack */
scope_elem *symtable_stack;

/* A single symbol table */
symtable *st;

/* Store the name of the function declaration that has been encountered.
   This is stored in it's entry in the symbol table and is checked against
   every time there is an assignment statement in the BEGIN-END block
   in that function to ensure a return assignment was used. */
char *calling_func_name;

/* Number of arguments in a function/procedure declaration */
int num_of_args;

int stack_offset;

%}

%union {
  char *sval;
  int ival;
  float rval;
  char *opval;
  tree_t *tval;
  type_struct *type_s;
}

%token <sval> ID
%token <rval> RNUM
%token <ival> INUM

%token ARRAY BBEGIN DO ELSE END FOR FUNCTION IF INTEGER NOT
%token OF PROCEDURE PROGRAM REAL THEN TO VAR WHILE
%token <opval> ADDOP MULOP RELOP ASSIGNOP

%type <type_s> type
%type <ival> standard_type

%type <tval> compound_statement optional_statements statement_list statement
%type <tval> matched_stmt unmatched_stmt assign_stmt variable procedure_statement
%type <tval> expression_list expression simple_expression
%type <tval> term factor

%%

program
         : PROGRAM ID '(' identifier_list ')' ';'
             {
               /* Check the current decl_elem list for input and output.
                  If encountered, gencode appropriately and clear the list.
                  This list has to be cleared because you do not want
                  "input" and "ouput" in main's symbol table, but rather
                  "read" and "write", which setup_input_output() will handle. */
               st = setup_input_output(st, head);
               decl_destroy(head);
               decl_destroy(data);
               head = NULL;
             }
         declarations
             {
               /* Push main's variable declarations.  This is done to ensure
                  function declarations have access to any global variables. */
               scope_push(symtable_stack, st);
             }
         subprogram_declarations
         compound_statement
             {
               /* Calculate main's stack offset.  It is cheating right now by
                  looking in the symbol table for all declarations and arguments,
                  so the calculation will >= than what's required */
               stack_offset = calculate_stack_offset(st);
               //fprintf(stderr, ">>> STACK OFFSET: %d\n", stack_offset); // debug

               /* Print main's assembly prologue (stack alignment) */
               prologue(st->name, stack_offset);

               /* Generate assembly for all encountered code in main's
                  BEGIN-END block (aka, slay the dragon, haha). */
               compound_stmt_gencode($11);

               /* Print main's stack clean-up code */
               epilogue();
             }
         '.'
         ;

identifier_list
         : ID
             {
               /* Append encountered ID to the decl_elem list */
               head = decl_append(head, $1);
             }
         | identifier_list ',' ID
             {
               head = decl_append(head, $3);
             }
         ;

declarations
         : declarations VAR identifier_list ':' type ';'
             {
               /* Add all elements that has been added to the current decl_list
                  to the symbol table with the appropriate class, type,
                  and location. */
               add_decl_to_symtab(VAR_CLASS, $5, ST_LOCAL);
             }
         | /* empty */
         ;

type
        : standard_type
             {
               /* Set a basic type (integer/real) */
               $$ = (type_struct *) calloc(1, sizeof(type_struct));
               $$->name = $1;
             }
         | ARRAY '[' INUM '.' '.' INUM ']' OF standard_type
             {
               /* Set lower and upper bounds for the array type */
               $$ = (type_struct *) calloc(1, sizeof(type_struct));
               if ($9 == INTEGER_TYPE)
                 $$->name = ARRAY_TYPE_INT;
               else if ($9 == REAL_TYPE)
                 $$->name = ARRAY_TYPE_REAL;
               $$->lb = $3;
               $$->ub = $6;
             }
         ;

standard_type
         : INTEGER
             {
               $$ = INTEGER_TYPE;
             }
         | REAL
             {
               $$ = REAL_TYPE;
             }
         ;

subprogram_declarations
         : subprogram_declarations subprogram_declaration ';'
         | /* empty */
         ;

subprogram_declaration
         : subprogram_head declarations compound_statement
             {
               /* Calculate a function's stack offset */
               stack_offset = calculate_stack_offset(st);
               prologue(st->name, stack_offset);

               /* Make sure the function var was assigned if class is function_type
                  The procedure case might not fire because there should be a
                  type mismatch when trying to assign a value to the proc's id */
               if (calling_func_name != NULL) {
                 st_node = search_scope_stack(symtable_stack, calling_func_name);
                 if (st_node->class == FUNCTION_CLASS && st->has_return != 1 && st->name != NULL)
                   sem_error(NO_RETURN_STMT);
                 else if (st_node->class == PROCEDURE_CLASS && st->has_return == 1)
                   sem_error(NON_LOCAL_MODIFICATION);
               }

               /* Slay the function's compound statement dragon */
               compound_stmt_gencode($3);

               /* Hack? Move the last register that gencode used into eax.  eax is then
                  only used to hold function return values. */
               if (st->name != NULL)
                 fprintf(stdout, "\tmovl %%ebx, %%eax\n");

               /* Pop current scope */
               st = scope_pop(symtable_stack);
               /* Pop parent scope so it's available again */
               st = scope_pop(symtable_stack);

               epilogue();
             }
         ;

subprogram_head
         : FUNCTION ID
             {
               type_struct *t = calloc(1, sizeof(type_struct));

               /* st->entries increases to keep track of the variable's
                  index in the symbol table (st).  Used for gencode identifier lookups */
               (st->entries)++;
               symtab_insert(st, $2, FUNCTION_CLASS, t, ST_LOCAL, st->entries);

               // push parent with function name added to it
               scope_push(symtable_stack, st);
               st = calloc(1, sizeof(symtable));
               /* reset to 0 because a new symbol table has its own set of offsets
                  for local declarations */
               st->entries = 0;

               // set encountered func as calling func name
               calling_func_name = $2;

               // reset arg counter and argument list
               num_of_args = 0;
               arg_head = NULL;
             }
           arguments ':' standard_type ';'
             {
               // set symtab name as the function name
               st->name = calling_func_name;

               // create arg_type_list for the parent symbol table
               st_node = search_scope_stack(symtable_stack, $2);
               st_node->arg_type_list = arg_head;
               st_node->type->name = $6;
               st_node->num_of_args = num_of_args;

               // push symtab so it can be searched in the function
               scope_push(symtable_stack, st);
             }
         | PROCEDURE ID
             {
               type_struct *t = calloc(1, sizeof(type_struct));

               (st->entries)++;
               symtab_insert(st, $2, PROCEDURE_CLASS, t, ST_LOCAL, st->entries);

               // push parent with procedure name added to it
               scope_push(symtable_stack, st);
               st = calloc(1, sizeof(symtable));
               st->entries = 0;

               // set func as calling func name
               calling_func_name = $2;

               // reset arg counter and argument list
               num_of_args = 0;
               //arg_destroy(arg_head);
               arg_head = NULL;
             }
           arguments ';'
             {
               // set symtab name as the procedure name
               st->name = calling_func_name;

               // create arg_type_list for the parent symbol table
               st_node = search_scope_stack(symtable_stack, $2);
               st_node->arg_type_list = arg_head;
               st_node->num_of_args = num_of_args;

               // push symtab so it can be searched in the function
               scope_push(symtable_stack, st);
             }
         ;

arguments
         : '(' parameter_list ')'
         | /* empty */
         ;

parameter_list
         : identifier_list ':' type
             {
               add_decl_to_symtab(VAR_CLASS, $3, ST_PARAMETER);
             }
         | parameter_list ';' identifier_list ':' type
             {
               add_decl_to_symtab(VAR_CLASS, $5, ST_PARAMETER);
             }
         ;

compound_statement
         : BBEGIN optional_statements END
             {
               #ifdef PRINT_SYMBOL_TABLE
               print_symtab(st);
               #endif

               $$ = mktree(BEGIN_END, 1, $2);

               #ifdef PRINT_SYNTAX_TREE
               print_tree($2, 0);
               #endif
             }
         ;

optional_statements
         : statement_list
             { $$ = $1; }
         | /* empty */
             { $$ = NULL; }
         ;

statement_list
         : statement
             { $$ = $1; }
         | statement_list ';' statement
             { $$ = mktree(STMT_LIST, 2, $1, $3); }
         ;

statement
         : matched_stmt
             { $$ = $1; }
         | unmatched_stmt
             { $$ = $1; }
         ;

matched_stmt
         : assign_stmt
             { $$ = $1; }
         | procedure_statement
             { $$ = $1; }
         | compound_statement
             { $$ = $1; }
         | IF expression THEN matched_stmt ELSE matched_stmt
             {
               if_then_semantics($2);
               $$ = mktree(IF_THEN_ELSE_MATCHED, 3, $2, $6, $4);
             }
         | WHILE expression DO matched_stmt
             {
               while_semantics($2);
               $$ = mktree(WHILE_DO, 2, $2, $4);
             }
         | FOR assign_stmt TO expression DO matched_stmt
             { $$ = mktree(FOR_LOOP, 3, $2, $4, $6); }
         ;

unmatched_stmt
         : IF expression THEN statement
             {
               if_then_semantics($2);
               $$ = mktree(IF_THEN_UNMATCHED, 2, $2, $4);
             }
         | IF expression THEN matched_stmt ELSE unmatched_stmt
             {
               if_then_semantics($2);
               $$ = mktree(IF_THEN_ELSE_UNMATCHED, 3, $2, $6, $4);
             }
         ;

assign_stmt
         : variable ASSIGNOP expression
             {
               //fprintf(stderr, "type comparison: %d =?= %d\n", type_check($1), type_check($3));
               if (type_check($1) != type_check($3)) { sem_error(TYPE_MISMATCH_ASSN); }
               label_tree($3, 1);
               $$ = mktree(ASSIGN_STMT, 2, $1, $3);
             }
         ;

variable
         : ID
             {
               /* Look up the entry in the symbol table so it can be attached
                  to the syntax tree */
               st_node = search_scope_stack(symtable_stack, $1);
               if (st_node == NULL) { sem_error(VAR_UNDECLARED); }

               /* set has_return flag for the current symbol table
                  if assignment was to a variable that matches the name
                  of the function that it appears in */
               if (st_node->class == FUNCTION_CLASS)
                 if (!strcmp($1, st->name))
                   st->has_return = 1;
                   //fprintf(stderr, "function has return value (%s | %s)\n", $1, st->name);

               $$ = mktree(IDENT, 0);
               $$->attr.sval = st_node;
             }
         | ID '[' expression ']'
             {
               st_node = search_scope_stack(symtable_stack, $1);
               if (st_node == NULL) { sem_error(VAR_UNDECLARED); }
               array_semantics(st_node, $3);

               $$ = mktree(ID_ARRAY, 1, $3);
               $$->attr.sval = st_node;
             }
         ;

procedure_statement
         : ID
             {
               st_node = search_scope_stack(symtable_stack, $1);
               if (st_node == NULL) { sem_error(VAR_UNDECLARED); }

               $$ = mktree(IDENT, 0);
               $$->attr.sval = st_node;
             }
         | ID '(' expression_list ')'
             {
               st_node = search_scope_stack(symtable_stack, $1);
               if (st_node == NULL) { sem_error(VAR_UNDECLARED); }
               procedure_semantics(st_node, $3);

               $$ = mktree(PROC_CALL, 1, $3);
               $$->attr.sval = st_node;
             }
         ;

expression_list
         : expression
             {
               $$ = $1;
             }
         | expression_list ',' expression
             {
               $$ = mktree(EXPR_LIST, 2, $1, $3);
             }
         ;

expression
         : simple_expression
             {
               label_tree($1, 1);
               $$ = $1;
             }
         | simple_expression RELOP simple_expression
             {
               $$ = mktree(RELOP_EXPR, 2, $1, $3);
               $$->attr.opval = $2;
             }
         ;

simple_expression
         : term
             { $$ = $1; }
         | sign term
             { $$ = mktree(SIGN_TERM, 1, $2); }
         | simple_expression ADDOP term
             {
               $$ = mktree(ADDOP_EXPR, 2, $1, $3);
               $$->attr.opval = $2;
             }
         ;

term
         : factor
             { $$ = $1; }
         | term MULOP factor
             {
               $$ = mktree(MULOP_EXPR, 2, $1, $3);
               $$->attr.opval = $2;
             }
         ;

factor
         : ID
             {
               st_node = search_scope_stack(symtable_stack, $1);
               if (st_node == NULL) { sem_error(VAR_UNDECLARED); }

               $$ = mktree(IDENT, 0);
               $$->attr.sval = st_node;
             }
         | ID '(' expression_list ')'
             {
               st_node = search_scope_stack(symtable_stack, $1);
               if (st_node == NULL) { sem_error(VAR_UNDECLARED); }
               function_semantics(st_node, $3);

               $$ = mktree(FUNC_CALL, 1, $3);
               $$->attr.sval = st_node;
             }
         | ID '[' expression ']'
             {
               st_node = search_scope_stack(symtable_stack, $1);
               if (st_node == NULL) { sem_error(VAR_UNDECLARED); }
               array_semantics(st_node, $3);

               $$ = mktree(ID_ARRAY, 1, $3);
               $$->attr.sval = st_node;
             }
         | RNUM
             {
               $$ = mktree(RNUMBER, 0);
               $$->attr.rval = $1;
             }
         | INUM
             {
               $$ = mktree(INUMBER, 0);
               $$->attr.ival = $1;
             }
         | '(' expression ')'
             {
               $$ = $2;
             }
         | NOT factor
             {
               $$ = mktree(NOT_FACTOR, 1, $2);
             }
         ;

sign
         : '+' | '-'
         ;

%%
 /* Loop through the current decl_elem list (list of declarations) and each one
    to the current current symbol table.  Clear the list when done is it's ready
    for the any future declaration lists. */
void add_decl_to_symtab(symtab_type class, type_struct *type, location_e location) {
  data = head;

  if (data != NULL) {
    do {
      if (location == ST_PARAMETER) {
        num_of_args++;
        arg_head = arg_append(arg_head, type->name);
      }

      //fprintf(stderr, "calling func name: %s\n", calling_func_name);
      (st->entries)++;
      symtab_insert(st, data->name, class, type, location, st->entries);
      //fprintf(stderr, "param_list, name: %s, type: %d\n\n", data->name, type->name);

      data = data->next;
    }
    while (data != head);

    decl_destroy(head);
    decl_destroy(data);
    head = NULL;
  }
  else
    fprintf(stderr, "data is null\n");

}

main() {
  /* Print the fake main which calls foo, the "real" main */
  print_fake_main();

  /* Limitation: The maximum number of scopes that can be pushed onto the stack is 10 */
  symtable_stack = scope_init(10);
  st = calloc(1, sizeof(symtable));
  st->entries = 0;

  yyparse();
  printf("\n");
}
