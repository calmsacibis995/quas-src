/*
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* @(#)strfile.h	5.2 (Berkeley) 9/14/99 */

# define	MAXDELIMS	3

/*
 * bits for flag field
 */

# define	STR_RANDOM	0x1
# define	STR_ORDERED	0x2

struct	strfile {		/* information table */
	unsigned int	str_numstr;		/* # of strings in the file */
	unsigned int	str_longlen;		/* length of longest string */
	unsigned int	str_shortlen;		/* length of shortest string */
	long		str_delims[MAXDELIMS];	/* delimiter markings */
	int		str_flags;		/* bit field for flags */
};

typedef struct strfile	STRFILE;
