/*	Kcvtdl.s	1.3	86/01/05	*/

#include "../tahoemath/fp.h"
#include "../tahoemath/Kfp.h"
#include "../tahoe/SYS.h"

 
	.text
	.globl	_Kcvtdl
	.globl	_Kcvtdu
_Kcvtdu:
_Kcvtdl:
	.word	R5|R4|R3|R2

 #
 #Some initializations:
 #
	clrl	r3		# r3 - negative flag.
	movl	4(fp),r0	# fetch operand.
	movl	8(fp),r1	
 #
 #get exponent
 #
	andl3	$EXPMASK,r0,r2	# r2 will hold the exponent.
	jeql	retzero	
	cmpl	$TWO_EXP,r2	# if exponent is less then -2,return zero.
	jgtr	retzero
	andl2	$0!EXPSIGN,r2	# turn off biased exponent sign
	shrl	$EXPSHIFT,r2,r2
 #
 #get fraction
 #
	bbc	$31,r0,positive	# if negative remember it.
	incl	r3
positive:
				# clear the non fraction parts.
	andl2	$(0!(EXPMASK | SIGNBIT)),r0
				# add the hidden bit.
	orl2	$(0!CLEARHID),r0
	subl2	$24,r2		# compute the shift.
	jgtr	shift_left
	mnegl	r2,r2
	shrl	r2,r0,r0	# shift right.
	jmp	shifted
shift_left:
	cmpl	r2,$7
	jgtr	overflow
go_on:	shlq	r2,r0,r0	# shift right.
shifted:
	bbc	$0,r3,done	# check for negative
	mnegl	r0,r0
done:	
	ret

retzero:
	clrl	r0
	ret
overflow:
	ret
