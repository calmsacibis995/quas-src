/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)lisp.c	5.1 (Berkeley) 3/16/87";
#endif not lint

#include <ctags.h>
#include <strings.h>

extern char	*lbp;			/* pointer shared with fortran */

/*
 * lisp tag functions
 * just look for (def or (DEF
 */
l_entries()
{
	register int	special;
	register char	*cp,
			savedc;
	char	tok[MAXTOKEN];

	for (;;) {
		lineftell = ftell(inf);
		if (!fgets(lbuf,sizeof(lbuf),inf))
			return;
		++lineno;
		lbp = lbuf;
		if (!cicmp("(def"))
			continue;
		special = NO;
		switch(*lbp | ' ') {
		case 'm':
			if (cicmp("method"))
				special = YES;
			break;
		case 'w':
			if (cicmp("wrapper") || cicmp("whopper"))
				special = YES;
		}
		for (;!isspace(*lbp);++lbp);
		for (;isspace(*lbp);++lbp);
		for (cp = lbp;*cp && *cp != '\n';++cp);
		*cp = EOS;
		if (special) {
			if (!(cp = index(lbp,')')))
				continue;
			for (;cp >= lbp && *cp != ':';--cp);
			if (cp < lbp)
				continue;
			lbp = cp;
			for (;*cp && *cp != ')' && *cp != ' ';++cp);
		}
		else
			for (cp = lbp + 1;
			    *cp && *cp != '(' && *cp != ' ';++cp);
		savedc = *cp;
		*cp = EOS;
		(void)strcpy(tok,lbp);
		*cp = savedc;
		getline();
		pfnote(tok,lineno);
	}
	/*NOTREACHED*/
}
