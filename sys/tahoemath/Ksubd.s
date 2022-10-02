/*	Ksubd.s	1.3	86/01/05	*/

#include "../tahoe/SYS.h"

/*
 * double 
 * Ksubd(d1,d2)
 * double d1,d2;
 * {
 * 	return(d1+(-d2));
 * }
 */
	.text
ENTRY(Ksubd, 0)
	tstl	4(fp)
	jneq	next
	movl	16(fp),r1
	movl	12(fp),r0
	lnd	r0
	std	r0
	ret
next:
	tstl	12(fp)
	jneq	doit
	movl	8(fp),r1
	movl	4(fp),r0
	ret
doit:
	lnd	12(fp)		# -op
	pushl	20(fp)		# hfs
	pushd			# push op_least op_most
	pushl	8(fp)
	pushl	4(fp)		# acc
	callf	$24,_Kaddd
	ret
