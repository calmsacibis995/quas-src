/*
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
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
 *
 *	@(#)dev.c	7.4 (Berkeley) 6/29/88
 */

#include "param.h"
#include "inode.h"
#include "fs.h"
#include "saio.h"

/*
 * NB: the value "io->i_ino.i_dev", used to offset the devsw[] array
 * in the routines below, is munged by the vaxstand Makefile to work
 * for certain boots.
 */

devread(io)
	register struct iob *io;
{
	int cc;

	io->i_flgs |= F_RDDATA;
	io->i_error = 0;
	cc = (*devsw[io->i_ino.i_dev].dv_strategy)(io, READ);
	io->i_flgs &= ~F_TYPEMASK;
	return (cc);
}

devwrite(io)
	register struct iob *io;
{
	int cc;

	io->i_flgs |= F_WRDATA;
	io->i_error = 0;
	cc = (*devsw[io->i_ino.i_dev].dv_strategy)(io, WRITE);
	io->i_flgs &= ~F_TYPEMASK;
	return (cc);
}

devopen(io)
	register struct iob *io;
{
	int ret;

	if (!(ret = (*devsw[io->i_ino.i_dev].dv_open)(io)))
		return (0);
	printf("%s(%d,%d,%d,%d): ", devsw[io->i_ino.i_dev].dv_name,
		io->i_adapt, io->i_ctlr, io->i_unit, io->i_part);
	switch(ret) {
	case EIO:
		break;		/* already reported */
	case EADAPT:
		printf("bad adaptor number\n");
		break;
	case ECTLR:
		printf("bad controller number\n");
		break;
	case EUNIT:
		printf("bad drive number\n");
		break;
	case EPART:
		printf("bad partition\n");
		break;
	case ERDLAB:
		printf("can't read disk label\n");
		break;
	case EUNLAB:
		printf("unlabeled\n");
		break;
	case ENXIO:
		printf("bad device specification\n");
		break;
	default:
		printf("unknown open error\n");
		break;
	}
	return (ret);
}

devclose(io)
	register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_close)(io);
}

devioctl(io, cmd, arg)
	register struct iob *io;
	int cmd;
	caddr_t arg;
{
	return ((*devsw[io->i_ino.i_dev].dv_ioctl)(io, cmd, arg));
}

/*ARGSUSED*/
nullsys(io)
	struct iob *io;
{}

/*ARGSUSED*/
nodev(io)
	struct iob *io;
{
	errno = EBADF;
}

/*ARGSUSED*/
noioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{
	return (ECMD);
}
