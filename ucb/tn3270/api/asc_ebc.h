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
 *	@(#)asc_ebc.h	3.2 (Berkeley) 3/28/88
 */

/*
 * Definitions of translate tables used for ascii<->ebcdic translation.
 */

#define	INCLUDED_ASCEBC

/*
 * ascii/ebcdic translation information
 */

#define	NASCII	128		/* number of ascii characters */

#define	NEBC	256		/* number of ebcdic characters */

extern unsigned char
	asc_ebc[NASCII], ebc_asc[NEBC];
