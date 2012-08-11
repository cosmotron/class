CC = gcc
CFLAGS = -g
#CFLAGS = -g -DPRINT_SYMBOL_TABLE -DPRINT_SYNTAX_TREE
LEX = flex
LFLAGS = -l
YACC = bison
YFLAGS = -dv
LIBS = -ll -ly
HEADERS = scope.h tree.h symtab.h decl_list.h arg_list.h semantics.h gencode.h type.h
SOURCES = scope.c tree.c symtab.c decl_list.c arg_list.c semantics.c gencode.c
OBJECTS = pascal.tab.o lex.yy.o scope.o tree.o symtab.o decl_list.o arg_list.o semantics.o gencode.o
BINARY = pascal

all: pascal.tab.c lex.yy.c $(SOURCES) $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BINARY) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

pascal.tab.c: pascal.y
	$(YACC) $(YFLAGS) pascal.y

lex.yy.c: pascal.lex
	$(LEX) $(LFLAGS) pascal.lex

tar:
	tar -cvf pascal.tar Makefile pascal.lex pascal.y test_files $(HEADERS) $(SOURCES)

clean:
	rm -f pascal.tab.* lex.yy.* *.o pascal.output $(BINARY)
