/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_synch.c	7.6 (Berkeley) 12/11/87
 */

#include "../machine/pte.h"
#include "../machine/psl.h"
#include "../machine/mtpr.h"

#include "param.h"
#include "systm.h"
#include "dir.h"
#include "user.h"
#include "proc.h"
#include "vm.h"
#include "kernel.h"
#include "buf.h"

/*
 * Force switch among equal priority processes every 100ms.
 */
roundrobin()
{

	runrun++;
	aston();
	timeout(roundrobin, (caddr_t)0, hz / 10);
}

/*
 * constants for digital decay and forget
 *	90% of (p_cpu) usage in 5*loadav time
 *	95% of (p_pctcpu) usage in 60 seconds (load insensitive)
 *          Note that, as ps(1) mentions, this can let percentages
 *          total over 100% (I've seen 137.9% for 3 processes).
 *
 * Note that hardclock updates p_cpu and p_cpticks independently.
 *
 * We wish to decay away 90% of p_cpu in (5 * loadavg) seconds.
 * That is, the system wants to compute a value of decay such
 * that the following for loop:
 * 	for (i = 0; i < (5 * loadavg); i++)
 * 		p_cpu *= decay;
 * will compute
 * 	p_cpu *= 0.1;
 * for all values of loadavg:
 *
 * Mathematically this loop can be expressed by saying:
 * 	decay ** (5 * loadavg) ~= .1
 *
 * The system computes decay as:
 * 	decay = (2 * loadavg) / (2 * loadavg + 1)
 *
 * We wish to prove that the system's computation of decay
 * will always fulfill the equation:
 * 	decay ** (5 * loadavg) ~= .1
 *
 * If we compute b as:
 * 	b = 2 * loadavg
 * then
 * 	decay = b / (b + 1)
 *
 * We now need to prove two things:
 *	1) Given factor ** (5 * loadavg) ~= .1, prove factor == b/(b+1)
 *	2) Given b/(b+1) ** power ~= .1, prove power == (5 * loadavg)
 *	
 * Facts:
 *         For x close to zero, exp(x) =~ 1 + x, since
 *              exp(x) = 0! + x**1/1! + x**2/2! + ... .
 *              therefore exp(-1/b) =~ 1 - (1/b) = (b-1)/b.
 *         For x close to zero, ln(1+x) =~ x, since
 *              ln(1+x) = x - x**2/2 + x**3/3 - ...     -1 < x < 1
 *              therefore ln(b/(b+1)) = ln(1 - 1/(b+1)) =~ -1/(b+1).
 *         ln(.1) =~ -2.30
 *
 * Proof of (1):
 *    Solve (factor)**(power) =~ .1 given power (5*loadav):
 *	solving for factor,
 *      ln(factor) =~ (-2.30/5*loadav), or
 *      factor =~ exp(-1/((5/2.30)*loadav) =~ exp(-1/(2*loadav)) =
 *          exp(-1/b) =~ (b-1)/b =~ b/(b+1).                    QED
 *
 * Proof of (2):
 *    Solve (factor)**(power) =~ .1 given factor == (b/(b+1)):
 *	solving for power,
 *      power*ln(b/(b+1)) =~ -2.30, or
 *      power =~ 2.3 * (b + 1) = 4.6*loadav + 2.3 =~ 5*loadav.  QED
 *
 * Actual power values for the implemented algorithm are as follows:
 *      loadav: 1       2       3       4
 *      power:  5.68    10.32   14.94   19.55
 */
#define	filter(loadav) ((2 * (loadav)) / (2 * (loadav) + 1))

double	ccpu = 0.95122942450071400909;		/* exp(-1/20) */

/*
 * Recompute process priorities, once a second
 */
schedcpu()
{
	register double ccpu1 = (1.0 - ccpu) / (double)hz;
	register struct proc *p;
	register int s, a;
	float scale = filter(avenrun[0]);

	wakeup((caddr_t)&lbolt);
	for (p = allproc; p != NULL; p = p->p_nxt) {
		if (p->p_time != 127)
			p->p_time++;
		if (p->p_stat==SSLEEP || p->p_stat==SSTOP)
			if (p->p_slptime != 127)
				p->p_slptime++;
		/*
		 * If the process has slept the entire second,
		 * stop recalculating its priority until it wakes up.
		 */
		if (p->p_slptime > 1) {
			p->p_pctcpu *= ccpu;
			continue;
		}
		/*
		 * p_pctcpu is only for ps.
		 */
		p->p_pctcpu = ccpu * p->p_pctcpu + ccpu1 * p->p_cpticks;
		p->p_cpticks = 0;
		a = (int) (scale * (p->p_cpu & 0377)) + p->p_nice;
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		p->p_cpu = a;
		(void) setpri(p);
		s = splhigh();	/* prevent state changes */
		if (p->p_pri >= PUSER) {
#define	PPQ	(128 / NQS)
			if ((p != u.u_procp || noproc) &&
			    p->p_stat == SRUN &&
			    (p->p_flag & SLOAD) &&
			    (p->p_pri / PPQ) != (p->p_usrpri / PPQ)) {
				remrq(p);
				p->p_pri = p->p_usrpri;
				setrq(p);
			} else
				p->p_pri = p->p_usrpri;
		}
		splx(s);
	}
	vmmeter();
	if (runin!=0) {
		runin = 0;
		wakeup((caddr_t)&runin);
	}
	if (bclnlist != NULL)
		wakeup((caddr_t)&proc[2]);
	timeout(schedcpu, (caddr_t)0, hz);
}

/*
 * Recalculate the priority of a process after it has slept for a while.
 */
updatepri(p)
	register struct proc *p;
{
	register int a = p->p_cpu & 0377;
	float scale = filter(avenrun[0]);

	p->p_slptime--;		/* the first time was done in schedcpu */
	while (a && --p->p_slptime)
		a = (int) (scale * a) /* + p->p_nice */;
	p->p_slptime = 0;
	if (a < 0)
		a = 0;
	if (a > 255)
		a = 255;
	p->p_cpu = a;
	(void) setpri(p);
}

#define SQSIZE 0100	/* Must be power of 2 */
#define HASH(x)	(( (int) x >> 5) & (SQSIZE-1))
struct slpque {
	struct proc *sq_head;
	struct proc **sq_tailp;
} slpque[SQSIZE];

/*
 * Give up the processor till a wakeup occurs
 * on chan, at which time the process
 * enters the scheduling queue at priority pri.
 * The most important effect of pri is that when
 * pri<=PZERO a signal cannot disturb the sleep;
 * if pri>PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */
sleep(chan, pri)
	caddr_t chan;
	int pri;
{
	register struct proc *rp;
	register struct slpque *qp;
	register s;
	extern int cold;

	rp = u.u_procp;
	s = splhigh();
	if (cold || panicstr) {
		/*
		 * After a panic, or during autoconfiguration,
		 * just give interrupts a chance, then just return;
		 * don't run any other procs or panic below,
		 * in case this is the idle process and already asleep.
		 * The splnet should be spl0 if the network was being used
		 * by the filesystem, but for now avoid network interrupts
		 * that might cause another panic.
		 */
		(void) splnet();
		splx(s);
		return;
	}
	if (chan==0 || rp->p_stat != SRUN || rp->p_rlink)
		panic("sleep");
	rp->p_wchan = chan;
	rp->p_slptime = 0;
	rp->p_pri = pri;
	qp = &slpque[HASH(chan)];
	if (qp->sq_head == 0)
		qp->sq_head = rp;
	else
		*qp->sq_tailp = rp;
	*(qp->sq_tailp = &rp->p_link) = 0;
	if (pri > PZERO) {
		/*
		 * If we stop in issig(), wakeup may already have happened
		 * when we return (rp->p_wchan will then be 0).
		 */
		if (ISSIG(rp)) {
			if (rp->p_wchan)
				unsleep(rp);
			rp->p_stat = SRUN;
			(void) spl0();
			goto psig;
		}
		if (rp->p_wchan == 0)
			goto out;
		rp->p_stat = SSLEEP;
		(void) spl0();
		u.u_ru.ru_nvcsw++;
		swtch();
		if (ISSIG(rp))
			goto psig;
	} else {
		rp->p_stat = SSLEEP;
		(void) spl0();
		u.u_ru.ru_nvcsw++;
		swtch();
	}
	curpri = rp->p_usrpri;
out:
	splx(s);
	return;

	/*
	 * If priority was low (>PZERO) and
	 * there has been a signal, execute non-local goto through
	 * u.u_qsave, aborting the system call in progress (see trap.c)
	 */
psig:
	longjmp(&u.u_qsave);
	/*NOTREACHED*/
}

/*
 * Remove a process from its wait queue
 */
unsleep(p)
	register struct proc *p;
{
	register struct slpque *qp;
	register struct proc **hp;
	int s;

	s = splhigh();
	if (p->p_wchan) {
		hp = &(qp = &slpque[HASH(p->p_wchan)])->sq_head;
		while (*hp != p)
			hp = &(*hp)->p_link;
		*hp = p->p_link;
		if (qp->sq_tailp == &p->p_link)
			qp->sq_tailp = hp;
		p->p_wchan = 0;
	}
	splx(s);
}

/*
 * Wake up all processes sleeping on chan.
 */
wakeup(chan)
	register caddr_t chan;
{
	register struct slpque *qp;
	register struct proc *p, **q;
	int s;

	s = splhigh();
	qp = &slpque[HASH(chan)];
restart:
	for (q = &qp->sq_head; p = *q; ) {
		if (p->p_rlink || p->p_stat != SSLEEP && p->p_stat != SSTOP)
			panic("wakeup");
		if (p->p_wchan==chan) {
			p->p_wchan = 0;
			*q = p->p_link;
			if (qp->sq_tailp == &p->p_link)
				qp->sq_tailp = q;
			if (p->p_stat == SSLEEP) {
				/* OPTIMIZED INLINE EXPANSION OF setrun(p) */
				if (p->p_slptime > 1)
					updatepri(p);
				p->p_stat = SRUN;
				if (p->p_flag & SLOAD)
					setrq(p);
				/*
				 * Since curpri is a usrpri,
				 * p->p_pri is always better than curpri.
				 */
				runrun++;
				aston();
				if ((p->p_flag&SLOAD) == 0) {
					if (runout != 0) {
						runout = 0;
						wakeup((caddr_t)&runout);
					}
					wantin++;
				}
				/* END INLINE EXPANSION */
				goto restart;
			}
		} else
			q = &p->p_link;
	}
	splx(s);
}

/*
 * Initialize the (doubly-linked) run queues
 * to be empty.
 */
rqinit()
{
	register int i;

	for (i = 0; i < NQS; i++)
		qs[i].ph_link = qs[i].ph_rlink = (struct proc *)&qs[i];
}

/*
 * Set the process running;
 * arrange for it to be swapped in if necessary.
 */
setrun(p)
	register struct proc *p;
{
	register int s;

	s = splhigh();
	switch (p->p_stat) {

	case 0:
	case SWAIT:
	case SRUN:
	case SZOMB:
	default:
		panic("setrun");

	case SSTOP:
	case SSLEEP:
		unsleep(p);		/* e.g. when sending signals */
		break;

	case SIDL:
		break;
	}
	p->p_stat = SRUN;
	if (p->p_flag & SLOAD)
		setrq(p);
	splx(s);
	if (p->p_slptime > 1)
		updatepri(p);
	if (p->p_pri < curpri) {
		runrun++;
		aston();
	}
	if ((p->p_flag&SLOAD) == 0) {
		if (runout != 0) {
			runout = 0;
			wakeup((caddr_t)&runout);
		}
		wantin++;
	}
}

/*
 * Set user priority.
 * The rescheduling flag (runrun)
 * is set if the priority is better
 * than the currently running process.
 */
setpri(pp)
	register struct proc *pp;
{
	register int p;

	p = (pp->p_cpu & 0377)/4;
	p += PUSER + 2 * pp->p_nice;
	if (pp->p_rssize > pp->p_maxrss && freemem < desfree)
		p += 2*4;	/* effectively, nice(4) */
	if (p > 127)
		p = 127;
	if (p < curpri) {
		runrun++;
		aston();
	}
	pp->p_usrpri = p;
	return (p);
}
