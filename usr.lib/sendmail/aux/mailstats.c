/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
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
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mailstats.c	5.5 (Berkeley) 6/29/88";
#endif /* not lint */

#include <sys/file.h>
#include <sendmail.h>
#include <mailstats.h>

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct statistics stat;
	register int i;
	int ch, fd;
	char *sfile, *ctime();

	sfile = "/usr/lib/sendmail.st";
	while ((ch = getopt(argc, argv, "f:")) != EOF)
		switch((char)ch) {
		case 'f':
			sfile = optarg;
			break;
		case '?':
		default:
			fputs("usage: mailstats [-f file]\n", stderr);
			exit(EX_USAGE);
		}
	argc -= optind;
	argv += optind;

	if ((fd = open(sfile, O_RDONLY)) < 0) {
		fputs("mailstats: ", stderr);
		perror(sfile);
		exit(EX_NOINPUT);
	}
	if (read(fd, &stat, sizeof(stat)) != sizeof(stat) ||
	    stat.stat_size != sizeof(stat)) {
		fputs("mailstats: file size changed.\n", stderr);
		exit(EX_OSERR);
	}

	printf("Statistics from %s", ctime(&stat.stat_itime));
	printf(" M msgsfr bytes_from  msgsto   bytes_to\n");
	for (i = 0; i < MAXMAILERS; i++)
		if (stat.stat_nf[i] || stat.stat_nt[i])
			printf("%2d %6ld %10ldK %6ld %10ldK\n", i,
			    stat.stat_nf[i], stat.stat_bf[i],
			    stat.stat_nt[i], stat.stat_bt[i]);
	exit(0);
}
