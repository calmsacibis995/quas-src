/*
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)copy.c	7.6 (Berkeley) 1/9/99
 */

/*
 * Copy from to in 10K units.
 * Intended for use in system
 * installation.
 */
main()
{
	extern int errno;
	register int from, to, record;
	char buffer[10240];

	from = getfile("From", 0);
	to = getfile("To", 1);
	for (record = 0; ; record++) {
		register int rcc, wcc;

		rcc = read(from, buffer, sizeof (buffer));
		if (rcc == 0)
			break;
		if (rcc < 0) {
			printf("Record %d: read error, errno=%d\n",
				record, errno);
			break;
		}
		if (rcc < sizeof (buffer))
			printf("Record %d: read short; expected %d, got %d\n",
				record, sizeof (buffer), rcc);
		/*
		 * For bug in ht driver.
		 */
		if (rcc > sizeof (buffer))
			rcc = sizeof (buffer);
		wcc = write(to, buffer, rcc);
		if (wcc < 0) {
			printf("Record %d: write error: errno=%d\n",
				record, errno);
			break;
		}
		if (wcc < rcc) {
			printf("Record %d: write short; expected %d, got %d\n",
				record, rcc, wcc);
			break;
		}
	}
	printf("Copy completed: %d records copied\n", record);
	/* can't call exit here */
}
