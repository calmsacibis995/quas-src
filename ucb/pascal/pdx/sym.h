/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)sym.h	5.3 (Berkeley) 1/3/88
 */

/*
 * This header file declares the variables and routines that
 * are defined within the "sym" subdirectory and that can be
 * accessed from outside.
 */

SYM *program;

/*
 * attributes
 */

char *name();			/* symbol string name */
char *classname();		/* class name of a symbol */
int toknum();			/* token number of reserved word */
int tokval();			/* associated token value */
int size();			/* size in bytes of a type */
SYM *rtype();			/* the reduced type, i.e. no type names */
SYM *container();		/* symbol (block) that contains given symbol */
ADDRESS codeloc();		/* address of the beginning of a function */

/*
 * predicates
 */

BOOLEAN isblock();		/* test if a symbol is a block name */
BOOLEAN isbuiltin();		/* test if a symbol is a builtin type */
BOOLEAN isparam();		/* test if a symbol is a parameter */
BOOLEAN isvarparam();		/* test if a symbol is a var parameter */
BOOLEAN isvariable();		/* test if a symbol is a variable */
BOOLEAN isambiguous();		/* test if a symbol name is not unique */
BOOLEAN compatible();		/* test to see if two types are compatible */
BOOLEAN should_print();		/* test if a symbol should be printed */

SYM *readsym();			/* read in a symbol from object file */
SYM *which();			/* find the appropriate symbol of given name */
SYM *findsym();			/* find a symbol for a given block */
SYM *findclass();		/* find symbol with given class */
NODE *dot();			/* construct a tree for the dot operator */
NODE *subscript();		/* construct a tree for subscripting */
SYM *treetype();		/* return the type of a tree, checking also */
long evalindex();		/* evaluate a subscript index */
int unmkstring();		/* free a constant string type */
int chkboolean();		/* check if a tree is boolean-valued */
int printdecl();		/* print out a symbol's declaration */
int printparams();		/* print out values of a fn's parameters */
int printentry();		/* note entrance of a block */
int printexit();		/* note exiting from a block */
int printcall();		/* note call of a function */
int printval();			/* print an eval result */
int printv();			/* print the name and value of a variable */
int printwhich();		/* print the full "path" of an identifier */
int maketypes();		/* initialize basic types */
int make_keyword();		/* create a keyword in a given symbol table */
