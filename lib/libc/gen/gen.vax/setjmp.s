/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
_sccsid:.asciz	"@(#)setjmp.s	5.7 (Berkeley) 5/25/88"
#endif /* SYSLIBC_SCCS and not lint */

/*
 * C library -- setjmp, longjmp
 *
 *	longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	setjmp(a)
 * by restoring registers from the stack,
 * and a struct sigcontext, see <signal.h>
 */

#include "DEFS.h"

ENTRY(setjmp, R6)
	movl	4(ap),r6		# construct sigcontext
	subl2	$8,sp			# space for current struct sigstack
	pushl	sp			# get current values
	pushl	$0			# no new values
	calls	$3,_sigstack		# pop args plus signal stack value
	movl	(sp)+,(r6)+		# save onsigstack status of caller
	pushl	$0
	calls	$1,_sigblock		# get signal mask
	movl	r0,(r6)+		# save signal mask of caller
	movl	(ap),r0
	moval	4(ap)[r0],(r6)+		# save sp of caller
	movl	12(fp),(r6)+		# save frame pointer of caller
	movl	8(fp),(r6)+		# save argument pointer of caller
	movl	16(fp),(r6)+		# save pc of caller
	movpsl	(r6)			# save psl of caller
	movw	4(fp),(r6)
	clrl	r0
	ret

ENTRY(longjmp, 0)
	movl	8(ap),r0		# return(v)
	movl	4(ap),r1		# fetch buffer
	tstl	12(r1)
	beql	botch
loop:
	cmpl	12(r1),fp		# are we there yet?
	beql	done
	blssu	botch
	moval	20(fp),r2
	blbc	6(fp),1f		# was r0 saved?
	movl	r0,(r2)+
1:
	bbc	$1,6(fp),2f		# was r1 saved?
	movl	r1,(r2)
2:
	movl	$loop,16(fp)
	ret				# pop another frame

done:
	pushl	r1			# pointer to sigcontext
	calls	$1,_sigreturn		# restore previous context
					# we should never return
botch:
	calls	$0,_longjmperror
	halt
