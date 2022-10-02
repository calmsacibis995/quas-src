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
 *	@(#)vaxbsubs.s	1.2 (Berkeley) 3/28/88
 */

/* This is taken from bcmp.s from 4.2.
 * The output of bunequal is the offset of the byte which didn't match;
 * if all the bytes match, then we return n.
 *
 * BUGNOTE:  This has no chance of working for lengths greater than 64K.
 *		(so, if you use this somewhere else, you may need to
 *		fix it...)
 */

/* bunequal(s1, s2, n) */

#include "defs.h"

ENTRY(bunequal)
	movl	4(ap),r1
	movl	8(ap),r3
	movl	12(ap),r4
1:
	movzwl	$65535,r0
	cmpl	r4,r0
	jleq	2f
	subl2	r0,r4
	cmpc3	r0,(r1),(r3)
	jeql	1b
	addl2	r4,r0
	/* changes... */
	subl3	r0,12(ap),r0
	/* end of changes for bunequal... */
	ret
2:
	cmpc3	r4,(r1),(r3)
	/* changes... */
	subl3	r0,12(ap),r0
	/* end of changes for bunequal... */
	ret




/* brand new code, using the above as base... */
/* bskip(s1, n, b) : finds the first occurrence of any byte != 'b' in the 'n'
 * bytes beginning at 's1'.
 *
 * BUGNOTE:  This has no chance of working for lengths greater than 64K.
 *		(so, if you use this somewhere else, you may need to
 *		fix it...)
 */

ENTRY(bskip)
	movl	4(ap),r1
	movl	8(ap),r3
	movl	12(ap),r4
1:
	movzwl	$65535,r0
	cmpl	r3,r0
	jleq	2f
	subl2	r0,r3
	skpc	r4,r0,(r1)
	jeql	1b
	addl2	r3,r0
	subl3	r0,8(ap),r0
	ret
2:
	skpc	r4,r3,(r1)
	subl3	r0,8(ap),r0
	ret
