%{

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)parse.y	5.4 (Berkeley) 6/18/88";
#endif /* not lint */

#include "htable.h"
%}

%union {
	int	number;
	struct	addr *addrlist;
	struct	name *namelist;
}
%start Table

%token			END
%token <number>		NUMBER KEYWORD
%token <namelist>	NAME

%type <namelist>	Names Cputype Opsys Protos Proto
%type <addrlist>	Addresses Address
%%
Table	:	Entry
	|	Table Entry
	;

Entry	:	KEYWORD ':' Addresses ':' Names ':' END
	= {
		do_entry($1, $3, $5, NONAME, NONAME, NONAME);
	}
	|	KEYWORD ':' Addresses ':' Names ':' Cputype ':' END
	= {
		do_entry($1, $3, $5, $7, NONAME, NONAME);
	}
	|	KEYWORD ':' Addresses ':' Names ':' Cputype ':' Opsys ':' END
	= {
		do_entry($1, $3, $5, $7, $9, NONAME);
	}
	|	KEYWORD ':' Addresses ':' Names ':' Cputype ':' Opsys ':' ':' END
	= {
		do_entry($1, $3, $5, $7, $9, NONAME);
	}
	|	KEYWORD ':' Addresses ':' Names ':' Cputype ':' Opsys ':' Protos ':' END
	= {
		do_entry($1, $3, $5, $7, $9, $11);
	}
	|	error END
	|	END		/* blank line */
	;

Addresses:	Address
	= {
		$$ = $1;
	}
	|	Address ',' Addresses
	= {
		$1->addr_link = $3;
		$$ = $1;
	}
	;

Address	:	NUMBER '.' NUMBER '.' NUMBER '.' NUMBER
	= {
		char *a;

		$$ = (struct addr *)malloc(sizeof (struct addr));
		a = (char *)&($$->addr_val);
		a[0] = $1; a[1] = $3; a[2] = $5; a[3] = $7;
		$$->addr_link = NOADDR;
	}
	;

Names	:	NAME
	= {
		$$ = $1;
	}
	|	NAME ',' Names
	= {
		$1->name_link = $3;
		$$ = $1;
	}
	;

Cputype :	/* empty */
	= {
		$$ = NONAME;
	}
	|	NAME
	= {
		$$ = $1;
	}
	;

Opsys	:	/* empty */
	= {
		$$ = NONAME;
	}
	|	NAME
	= {
		$$ = $1;
	}
	;

Protos	:	Proto
	= {
		$$ = $1;
	}
	|	Proto ',' Protos
	= {
		$1->name_link = $3;
		$$ = $1;
	}
	;

Proto	:	NAME
	= {
		$$ = $1;
	}
	;
%%

#include <stdio.h>

extern int yylineno;

yyerror(msg)
	char *msg;
{
	fprintf(stderr, "\"%s\", line %d: %s\n", infile, yylineno, msg);
}
