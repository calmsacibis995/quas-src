#ifdef LIBC_SCCS
	.asciz	"@(#)cvtud.s	1.1 (Berkeley/CCI) 7/2/86"
#endif LIBC_SCCS

#include <tahoemath/fp.h>
#include "DEFS.h"

XENTRY(cvtud, R2|R3|R4|R5)
 #
 #	Some initializations:
 #
	clrl	r2		# r2 - exponent.
	movl	4(fp),r0	# fetch operand.
	jeql	retzero		# return zero.
 # 
 #	Compute exponent:
 #
positive:
	ffs	r0,r1
	incl 	r1
	addl2	r1,r2
	shrl	r1,r0,r0
	jneq	positive	# look for more set bits.
 #
 # we have the exponent in r2.
 #
	movl	4(fp),r0	# r0,r1 will hold the resulting f.p. number.
	clrl 	r1
 #
 #	Shift the fraction part to its proper place:
 #
	subl3	r2,$HID_POS,r3
	jlss	shiftr		# if less then zero we have to shift right.
	shll	r3,r0,r0	# else we shift left.
	jmp	shifted
shiftr:
	mnegl	r3,r3
	shrq	r3,r0,r0
shifted:
	andl2	$CLEARHID,r0	# clear the hidden bit.
	shal	$EXPSHIFT,r2,r2	# shift the exponent to its proper place.
	orl2	$EXPSIGN,r2	# set the exponent sign bit(to bias it).
	orl2	r2,r0		# combine exponent & fraction.
sign_ok:
	ret

retzero:
	clrl 	r0
	clrl	r1
	ret

	
