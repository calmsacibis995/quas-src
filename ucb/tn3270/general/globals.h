/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *	@(#)globals.h	3.2 (Berkeley) 3/28/88
 */

/*
 *	This file contains all the globals used by tn3270.
 *
 *	Since various files may want to reference this file,
 *	and since they may only want subsets of the globals,
 *	we assume they have #include'd all the other .h files
 *	first, and we only give those globals relevant to
 *	the #include'd .h files.
 *
 */

#if	defined(DEFINING_INSTANCES)
#define	EXTERN
#else
#define	EXTERN extern
#endif


EXTERN int
		/*
		 * shell_active ==>
		 *		1.  Don't do input.
		 *		2.  Don't do output.
		 *		3.  Don't block in select.
		 *		4.  When nothing to do, call shell_continue()
		 */
	shell_active;


#if	defined(INCLUDED_OPTIONS)
EXTERN int	OptHome;		/* where home should send us */

EXTERN int	OptLeftMargin;		/* where new line should send us */

EXTERN char	OptColTabs[80];		/* local tab stops */

EXTERN int	OptAPLmode;

EXTERN int	OptNullProcessing;	/* improved null processing */

EXTERN int	OptZonesMode;		/* zones mode off */

EXTERN int	OptEnterNL;		/* regular enter/new line keys */

EXTERN int	OptColFieldTab;		/* regular column/field tab keys */

EXTERN int	OptPacing;		/* do pacing */

EXTERN int	OptAlphaInNumeric;	/* allow alpha in numeric fields */

EXTERN int	OptHome;

EXTERN int	OptLeftMargin;

EXTERN int	OptWordWrap;
#endif

#if	defined(INCLUDED_SCREEN)
EXTERN ScreenImage
	Host[MAXSCREENSIZE];		/* host view of screen */

EXTERN char	Orders[256];			/* Non-zero for orders */

			/* Run-time screen geometry */
EXTERN int
	MaxNumberLines,		/* How many rows the 3270 COULD have */
	MaxNumberColumns,	/* How many columns the 3270 COULD have */
	NumberLines,		/* How many lines the 3270 screen contains */
	NumberColumns,		/* How many columns the 3270 screen contains */
	ScreenSize;

EXTERN int CursorAddress;			/* where cursor is */
EXTERN int BufferAddress;			/* where writes are going */

EXTERN int Lowest, Highest;

extern char CIABuffer[];

EXTERN int UnLocked;		/* is the keyboard unlocked */
EXTERN int AidByte;

#endif

#if	defined(INCLUDED_STATE)
#endif

#if	defined(INCLUDED_OIA)

EXTERN OIA OperatorInformationArea;

EXTERN int
    oia_modified,		/* Has the oia been modified */
    ps_modified;		/* Has the presentation space been modified */

#endif	/* defined(INCLUDED_OIA) */
