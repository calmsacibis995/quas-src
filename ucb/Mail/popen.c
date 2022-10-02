/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#ifdef notdef
static char sccsid[] = "@(#)popen.c	5.4 (Berkeley) 2/18/88";
#endif /* notdef */

#include <stdio.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1
static	int	popen_pid[20];

FILE *
popen(cmd,mode)
char	*cmd;
char	*mode;
{
	int p[2];
	register myside, hisside, pid;

	if(pipe(p) < 0)
		return NULL;
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if((pid = vfork()) == 0) {
		/* myside and hisside reverse roles in child */
		close(myside);
		dup2(hisside, tst(0, 1));
		close(hisside);
		execl("/bin/csh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return NULL;
	popen_pid[myside] = pid;
	close(hisside);
	return(fdopen(myside, mode));
}

pclose(ptr)
FILE *ptr;
{
	register f, r;
	int omask;
	union wait status;
	extern int errno;

	f = fileno(ptr);
	fclose(ptr);
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
	while((r = wait(&status)) != popen_pid[f] && r != -1 && errno != EINTR)
		;
	if(r == -1)
		status.w_status = -1;
	sigsetmask(omask);
	return (status.w_status);
}
