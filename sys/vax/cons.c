/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)cons.c	7.5 (Berkeley) 5/7/88
 */

/*
 * VAX console driver (and floppy interface)
 */
#include "param.h"
#include "conf.h"
#include "dir.h"
#include "user.h"
#include "proc.h"
#include "ioctl.h"
#include "tty.h"
#include "systm.h"
#include "uio.h"

#include "cpu.h"
#include "cons.h"
#include "mtpr.h"

/*
 * On some machines (e.g. MicroVAX), a secondary console
 * such as a display may supercede the standard serial console.
 * On such machines, consops will be set to point to the cdevsw
 * entry for the secondary console, and the standard console device
 * (minor number 0) will be redirected.  Other minor numbers still
 * refer to the standard console serial line.
 *
 * Also, console output may be redirected to another tty
 * (e.g. a window); if so, constty will point to the current
 * virtual console.
 */
struct	cdevsw *consops = 0;
struct	tty *constty = 0;
struct	tty cons;
int	cnstart();
int	ttrstrt();
char	partab[];

/*ARGSUSED*/
cnopen(dev, flag)
	dev_t dev;
{
	register struct tty *tp = &cons;

	if (consops && minor(dev) == 0)
		return ((*consops->d_open)(dev, flag));
	tp->t_oproc = cnstart;
	if ((tp->t_state&TS_ISOPEN) == 0) {
		ttychars(tp);
		tp->t_state = TS_ISOPEN|TS_CARR_ON;
		tp->t_flags = EVENP|ECHO|XTABS|CRMOD;
	}
	if (tp->t_state&TS_XCLUDE && u.u_uid != 0)
		return (EBUSY);
	mtpr(RXCS, mfpr(RXCS)|RXCS_IE);
	mtpr(TXCS, mfpr(TXCS)|TXCS_IE);
	return ((*linesw[tp->t_line].l_open)(dev, tp));
}

/*ARGSUSED*/
cnclose(dev)
	dev_t dev;
{
	register struct tty *tp = &cons;

	if (consops && minor(dev) == 0)
		return ((*consops->d_close)(dev));
	(*linesw[tp->t_line].l_close)(tp);
	ttyclose(tp);
	return (0);
}

/*ARGSUSED*/
cnread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &cons;

	if (consops && minor(dev) == 0)
		return ((*consops->d_read)(dev, uio));
	return ((*linesw[tp->t_line].l_read)(tp, uio));
}

/*ARGSUSED*/
cnwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct tty *tp = &cons;

	if (minor(dev) == 0) {
		if (constty && (constty->t_state & (TS_CARR_ON | TS_ISOPEN)) ==
		    (TS_CARR_ON | TS_ISOPEN))
			tp = constty;
		else if (consops)
			return ((*consops->d_write)(dev, uio));
	}
	return ((*linesw[tp->t_line].l_write)(tp, uio));
}

static	int cnpolling = 0;
/*
 * Got a level-20 receive interrupt -
 * the LSI wants to give us a character.
 * Catch the character, and see who it goes to.
 */
/*ARGSUSED*/
cnrint(dev)
	dev_t dev;
{
	register int c;
	register struct tty *tp;

	if (cnpolling)
		return;
	c = mfpr(RXDB);
	if (c&RXDB_ID) {
#if VAX780
		if (cpu == VAX_780)
			cnrfl(c);
#endif
		return;
	}
	tp = &cons;
#ifdef KADB
	if (!kdbrintr(c, tp))
#endif
	(*linesw[tp->t_line].l_rint)(c, tp);
}

/*ARGSUSED*/
cnioctl(dev, cmd, addr, flag)
	dev_t dev;
	caddr_t addr;
{
	register struct tty *tp = &cons;
	int error;
 
	if (consops && minor(dev) == 0)
		return ((*consops->d_ioctl)(dev, cmd, addr, flag));
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, addr);
	if (error >= 0)
		return (error);
	error = ttioctl(tp, cmd, addr, flag);
	if (error < 0)
		error = ENOTTY;
	return (error);
}

int	consdone = 1;
/*
 * Got a level-20 transmission interrupt -
 * the LSI wants another character.  First,
 * see if we can send something to the typewriter.
 * If not, try the floppy.
 */
/*ARGSUSED*/
cnxint(dev)
	dev_t dev;
{
	register struct tty *tp = &cons;

	consdone++;
	tp->t_state &= ~TS_BUSY;
	if (tp->t_line)
		(*linesw[tp->t_line].l_start)(tp);
	else
		cnstart(tp);
#if VAX780
	if (cpu==VAX_780 && (tp->t_state & TS_BUSY) == 0)
		conxfl();
#endif
}

cnstart(tp)
	register struct tty *tp;
{
	register int c, s;

	s = spl5();
	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
		goto out;
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_wsel) {
			selwakeup(tp->t_wsel, tp->t_state & TS_WCOLL);
			tp->t_wsel = 0;
			tp->t_state &= ~TS_WCOLL;
		}
	}
	if (tp->t_outq.c_cc == 0)
		goto out;
	if (consdone == 0)
		goto out;
	c = getc(&tp->t_outq) & 0xff;
	if ((tp->t_flags & (RAW|LITOUT)) == 0) {
		if (c <= 0177)
			c |= (partab[c] & 0200);
		else {
			timeout(ttrstrt, (caddr_t)tp, (c&0177));
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}
	mtpr(TXDB, c);
	consdone = 0;
	tp->t_state |= TS_BUSY;
out:
	splx(s);
}

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 */
cnputc(c)
	register int c;
{
	register int s, timo;

	timo = 30000;
	/*
	 * Try waiting for the console tty to come ready,
	 * otherwise give up after a reasonable time.
	 */
	while ((mfpr(TXCS)&TXCS_RDY) == 0)
		if(--timo == 0)
			break;
	if (c == 0)
		return;
	s = mfpr(TXCS);
	mtpr(TXCS, 0);
	mtpr(TXDB, c&0xff);
	if (c == '\n')
		cnputc('\r');
	cnputc(0);
	mtpr(TXCS, s);
}

#if (defined(KADB) || defined(GENERIC)) && !defined(lint)
/*
 * Get character from console.
 */
cngetc()
{
	register int c, s;

	s = splhigh();
	while ((mfpr(RXCS)&RXCS_DONE) == 0 || (c = mfpr(RXDB)&0177) <= 0)
		;
	if (c == '\r')
		c = '\n';
	(void) splx(s);
	return (c);
}
#endif

#ifdef KADB
cnpoll(onoff)
	int onoff;
{

	cnpolling = onoff;
}
#endif
