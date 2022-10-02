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
static char sccsid[] = "@(#)dr_5.c	5.3 (Berkeley) 6/18/88";
#endif /* not lint */

#include "externs.h"

subtract(from, totalfrom, crewfrom, fromcap, pcfrom)
struct ship *from, *fromcap;
int pcfrom;
register int  totalfrom, crewfrom[3];
{
	register int n;

	if (fromcap == from && totalfrom) {		/* if not captured */
		for (n = 0; n < 3; n++) {
			if (totalfrom > crewfrom[n]) {
				totalfrom -= crewfrom[n];
				crewfrom[n] = 0;
			} else {
				crewfrom[n] -= totalfrom;
				totalfrom = 0;
			}
		}
		Write(W_CREW, from, 0, crewfrom[0], crewfrom[1], crewfrom[2], 0);
	} else if (totalfrom) {
		pcfrom -= totalfrom;
		pcfrom = pcfrom < 0 ? 0 : pcfrom;
		Write(W_PCREW, from, 0, pcfrom, 0, 0, 0);
	}
}

mensent(from, to, crew, captured, pc, isdefense)
struct ship *from, *to, **captured;
int crew[3], *pc;
char isdefense;
{					/* returns # of crew squares sent */
	int men = 0;
	register int n;
	int c1, c2, c3;
	register struct BP *bp;

	*pc = from->file->pcrew;
	*captured = from->file->captured;
	crew[0] = from->specs->crew1;
	crew[1] = from->specs->crew2;
	crew[2] = from->specs->crew3;
	bp = isdefense ? from->file->DBP : from->file->OBP;
	for (n=0; n < NBP; n++, bp++) {
		if (bp->turnsent && bp->toship == to)
			men += bp->mensent;
	}
	if (men) {
		c1 = men/100 ? crew[0] : 0;
		c2 = (men%100)/10 ? crew[1] : 0;
		c3 = men/10 ? crew[2] : 0;
		c3 = *captured == 0 ? crew[2] : *pc;
	} else
		c1 = c2 = c3 = 0;
	return(c1 + c2 + c3);
}
