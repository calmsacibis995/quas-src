/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#ifdef notdef
static char sccsid[] = "@(#)vars.c	5.4 (Berkeley) 2/18/88";
#endif /* notdef */

#include "rcv.h"

/*
 * Mail -- a mail program
 *
 * Variable handling stuff.
 */

/*
 * Assign a value to a variable.
 */

assign(name, value)
	char name[], value[];
{
	register struct var *vp;
	register int h;

	h = hash(name);
	vp = lookup(name);
	if (vp == NOVAR) {
		vp = (struct var *) calloc(sizeof *vp, 1);
		vp->v_name = vcopy(name);
		vp->v_link = variables[h];
		variables[h] = vp;
	}
	else
		vfree(vp->v_value);
	vp->v_value = vcopy(value);
}

/*
 * Free up a variable string.  We do not bother to allocate
 * strings whose value is "" since they are expected to be frequent.
 * Thus, we cannot free same!
 */

vfree(cp)
	char *cp;
{
	if (*cp)
		free(cp);
}

/*
 * Copy a variable value into permanent (ie, not collected after each
 * command) space.  Do not bother to alloc space for ""
 */

char *
vcopy(str)
	char str[];
{
	char *new;
	unsigned len;

	if (*str == '\0')
		return "";
	len = strlen(str) + 1;
	if ((new = malloc(len)) == NULL)
		panic("Out of memory");
	bcopy(str, new, (int) len);
	return new;
}

/*
 * Get the value of a variable and return it.
 * Look in the environment if its not available locally.
 */

char *
value(name)
	char name[];
{
	register struct var *vp;

	if ((vp = lookup(name)) == NOVAR)
		return(getenv(name));
	return(vp->v_value);
}

/*
 * Locate a variable and return its variable
 * node.
 */

struct var *
lookup(name)
	register char name[];
{
	register struct var *vp;

	for (vp = variables[hash(name)]; vp != NOVAR; vp = vp->v_link)
		if (*vp->v_name == *name && equal(vp->v_name, name))
			return(vp);
	return(NOVAR);
}

/*
 * Locate a group name and return it.
 */

struct grouphead *
findgroup(name)
	register char name[];
{
	register struct grouphead *gh;

	for (gh = groups[hash(name)]; gh != NOGRP; gh = gh->g_link)
		if (*gh->g_name == *name && equal(gh->g_name, name))
			return(gh);
	return(NOGRP);
}

/*
 * Print a group out on stdout
 */

printgroup(name)
	char name[];
{
	register struct grouphead *gh;
	register struct group *gp;

	if ((gh = findgroup(name)) == NOGRP) {
		printf("\"%s\": not a group\n", name);
		return;
	}
	printf("%s\t", gh->g_name);
	for (gp = gh->g_list; gp != NOGE; gp = gp->ge_link)
		printf(" %s", gp->ge_name);
	putchar('\n');
}

/*
 * Hash the passed string and return an index into
 * the variable or group hash table.
 */

hash(name)
	register char *name;
{
	register h = 0;

	while (*name) {
		h <<= 2;
		h += *name++;
	}
	if (h < 0 && (h = -h) < 0)
		h = 0;
	return (h % HSHSIZE);
}
