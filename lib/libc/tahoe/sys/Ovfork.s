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

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)Ovfork.s	5.3 (Berkeley) 6/27/88"
#endif /* SYSLIBC_SCCS and not lint */

/*
 * C library -- vfork
 * pid = vfork();
 *
 * r1 == 0 in parent process, r1 == 1 in child process.
 * r0 == pid of child in parent, r0 == pid of parent in child.
 *
 * trickery here, due to keith sklower, uses ret to clear the stack,
 * and then returns with a jump indirect, since only one person can return
 * with a ret off this stack... we do the ret before we vfork!
 * 
 */

	.set	vfork,66
.globl	_vfork

_vfork:
	.word	0x0000
	movl	-8(fp),r2
	movab	here,-8(fp)
	ret
here:
	kcall	$vfork
	bcc	vforkok
	jmp	verror
vforkok:
	tstl	r1		# child process ?
	bneq	child	# yes
	jbr 	parent
.globl	_errno
verror:
	movl	r0,_errno
	mnegl	$1,r0
	jmp	(r2)
child:
	clrl	r0
parent:
	jmp	(r2)
