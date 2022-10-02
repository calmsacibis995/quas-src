/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)cerror.tahoe.s	5.2 (Berkeley) 1/12/88
 *
 *
 * modified version of cerror
 *
 * The idea is that every time an error occurs in a system call
 * I want a special function "syserr" called.  This function will
 * either print a message and exit or do nothing depending on
 * defaults and use of "onsyserr".
 */

.globl	cerror
.comm	_errno,4

cerror:
	movl	r0,_errno
	callf	$0,_syserr	/* new code */
	mnegl	$1,r0
	ret

.globl	__mycerror		/* clumsy way to get this loaded */

__mycerror:
	.word	0
	ret
