/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *	@(#)astosc.c	3.2 (Berkeley) 3/28/88
 */

#include <ctype.h>

#include "../general/general.h"

#include "../ctlr/function.h"

#include "astosc.h"

struct astosc astosc[256] = {
#include "astosc.out"
};

/* compare two strings, ignoring case */

static
ustrcmp(string1, string2)
register char *string1;
register char *string2;
{
    register int c1, c2;

    while ((c1 = (unsigned char) *string1++) != 0) {
	if (isupper(c1)) {
	    c1 = tolower(c1);
	}
	if (isupper(c2 = (unsigned char) *string2++)) {
	    c2 = tolower(c2);
	}
	if (c1 < c2) {
	    return(-1);
	} else if (c1 > c2) {
	    return(1);
	}
    }
    if (*string2) {
	return(-1);
    } else {
	return(0);
    }
}


/*
 * This routine takes a string and returns an integer.  It may return
 * -1 if there is no other integer which corresponds to the
 * string.  -1 implies an error.
 */

int
ascii_to_index(string)
register char *string;
{
    register struct astosc *this;

    for (this = astosc; this <= &astosc[highestof(astosc)]; this++) {
	if ((this->name != 0) && (ustrcmp(this->name, string) == 0)) {
	    return this-astosc;
	}
    }
    return -1;
}
