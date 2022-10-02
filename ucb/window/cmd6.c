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
static char sccsid[] = "@(#)cmd6.c	3.15 (Berkeley) 6/29/88";
#endif /* not lint */

#include "defs.h"
#include "string.h"
#include "char.h"

/*
 * Debugging commands.
 */

c_debug()
{
	register struct ww *w;

	if (!terse)
		wwputs("[m(smap) n(ns) o(os) s(string) v(nvis) w(win)]? ", cmdwin);
	wwcurtowin(cmdwin);
	while (wwpeekc() < 0)
		wwiomux();
	if (!terse)
		wwputc('\n', cmdwin);
	switch (wwgetc()) {
	case 'm':
		wwdumpsmap();
		break;
	case 'n':
		wwdumpns();
		break;
	case 'o':
		wwdumpos();
		break;
	case 's':
		debug_str();
		break;
	case 'v':
		if ((w = getwin()) != 0)
			wwdumpnvis(w);
		break;
	case 'w':
		if ((w = getwin()) != 0)
			wwdumpwin(w);
		break;
	default:
		wwbell();
	}
}

#ifdef STR_DEBUG
debug_str()
{
	register struct ww *w;
	struct string *s;

	if ((w = openiwin(wwnrow - 3, "Allocated Strings")) == 0) {
		error("Can't open string window: %s.", wwerror());
		return;
	}
	for (s = str_head.s_forw; s != &str_head; s = s->s_forw) {
		if (more(w, 0) == 2)
			goto out;
		wwprintf(w, "(0x%x)\t\"%s\"\n", s->s_data, s->s_data);
	}
	waitnl(w);
out:
	closeiwin(w);
}
#else
debug_str()
{
	error("No string debugging.");
}
#endif
