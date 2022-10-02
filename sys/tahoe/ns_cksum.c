/*
 * Copyright (c) 1982, 1988 Regents of the University of California.
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
 *	@(#)ns_cksum.c	7.3 (Berkeley) 6/29/88
 */

#include "types.h"
#include "mbuf.h"
/*
 * Checksum routine for Network Systems Protocol Packets (Big-Endian).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */

#define ADDCARRY(x)  { if ((x) > 65535) (x) -= 65535; }
#define FOLD(x) {l_util.l = (x); (x) = l_util.s[0] + l_util.s[1]; ADDCARRY(x);}

u_short
ns_cksum(m, len)
	register struct mbuf *m;
	register int len;
{
	register u_short *w;
	register int sum = 0;
	register int mlen = 0;
	register int sum2;

	union {
		u_short s[2];
		long	l;
	} l_util;

	for (;m && len; m = m->m_next) {
		if (m->m_len == 0)
			continue;
		/*
		 * Each trip around loop adds in
		 * word from one mbuf segment.
		 */
		w = mtod(m, u_short *);
		if (mlen == -1) {
			/*
			 * There is a byte left from the last segment;
			 * ones-complement add it into the checksum.
			 */
			sum  += *(u_char *)w; /* Big-Endian, else << 8 */
			sum += sum;
			w = (u_short *)(1 + (char *)w);
			mlen = m->m_len - 1;
			len--;
			FOLD(sum);
		} else
			mlen = m->m_len;
		if (len < mlen)
			mlen = len;
		len -= mlen;
		/*
		 * We can do a 16 bit ones complement sum using
		 * 32 bit arithmetic registers for adding,
		 * with carries from the low added
		 * into the high (by normal carry-chaining)
		 * so long as we fold back before 16 carries have occured.
		 */
		if (1 & (int) w)
			goto uuuuglyy;
#ifndef TINY
/* -DTINY reduces the size from 1250 to 550, but slows it down by 22% */
		while ((mlen -= 32) >= 0) {
			sum += w[0]; sum += sum; sum += w[1]; sum += sum;
			sum += w[2]; sum += sum; sum += w[3]; sum += sum;
			sum += w[4]; sum += sum; sum += w[5]; sum += sum;
			sum += w[6]; sum += sum; sum += w[7]; sum += sum;
			FOLD(sum);
			sum += w[8]; sum += sum; sum += w[9]; sum += sum;
			sum += w[10]; sum += sum; sum += w[11]; sum += sum;
			sum += w[12]; sum += sum; sum += w[13]; sum += sum;
			sum += w[14]; sum += sum; sum += w[15]; sum += sum;
			FOLD(sum);
			w += 16;
		}
		mlen += 32;
#endif
		while ((mlen -= 8) >= 0) {
			sum += w[0]; sum += sum; sum += w[1]; sum += sum;
			sum += w[2]; sum += sum; sum += w[3]; sum += sum;
			FOLD(sum);
			w += 4;
		}
		mlen += 8;
		while ((mlen -= 2) >= 0) {
			sum += *w++; sum += sum;
		}
		goto commoncase;
uuuuglyy:
/* Big-Endian; else reverse ww and vv */
#define ww(n) (((u_char *)w)[n + n + 1])
#define vv(n) (((u_char *)w)[n + n])
		sum2 = 0;
#ifndef TINY
		while ((mlen -= 32) >= 0) {
		    sum += ww(0); sum += sum; sum += ww(1); sum += sum;
		    sum += ww(2); sum += sum; sum += ww(3); sum += sum;
		    sum += ww(4); sum += sum; sum += ww(5); sum += sum;
		    sum += ww(6); sum += sum; sum += ww(7); sum += sum;
		    FOLD(sum);
		    sum += ww(8); sum += sum; sum += ww(9); sum += sum;
		    sum += ww(10); sum += sum; sum += ww(11); sum += sum;
		    sum += ww(12); sum += sum; sum += ww(13); sum += sum;
		    sum += ww(14); sum += sum; sum += ww(15); sum += sum;
		    FOLD(sum);
		    sum2 += vv(0); sum2 += sum2; sum2 += vv(1); sum2 += sum2;
		    sum2 += vv(2); sum2 += sum2; sum2 += vv(3); sum2 += sum2;
		    sum2 += vv(4); sum2 += sum2; sum2 += vv(5); sum2 += sum2;
		    sum2 += vv(6); sum2 += sum2; sum2 += vv(7); sum2 += sum2;
		    FOLD(sum2);
		    sum2 += vv(8); sum2 += sum2; sum2 += vv(9); sum2 += sum2;
		    sum2 += vv(10); sum2 += sum2; sum2 += vv(11); sum2 += sum2;
		    sum2 += vv(12); sum2 += sum2; sum2 += vv(13); sum2 += sum2;
		    sum2 += vv(14); sum2 += sum2; sum2 += vv(15); sum2 += sum2;
		    FOLD(sum2);
		    w += 16;
		}
		mlen += 32;
#endif
		while ((mlen -= 8) >= 0) {
		    sum += ww(0); sum += sum; sum += ww(1); sum += sum;
		    sum += ww(2); sum += sum; sum += ww(3); sum += sum;
		    FOLD(sum);
		    sum2 += vv(0); sum2 += sum2; sum2 += vv(1); sum2 += sum2;
		    sum2 += vv(2); sum2 += sum2; sum2 += vv(3); sum2 += sum2;
		    FOLD(sum2);
		    w += 4;
		}
		mlen += 8;
		while ((mlen -= 2) >= 0) {
			sum += ww(0); sum += sum;
			sum2 += vv(0); sum2 += sum2;
			w++;
		}
		sum += (sum2 << 8);
commoncase:
		if (mlen == -1) {
			sum += *(u_char *)w << 8; /* Big-Endian, else no << 8 */
		}
		FOLD(sum);
	}
	if (mlen == -1) {
		/* We had an odd number of bytes to sum; assume a garbage
		   byte of zero and clean up */
		sum += sum;
		FOLD(sum);
	}
	/*
	 * sum has already been kept to low sixteen bits.
	 * just examine result and exit.
	 */
	if(sum==0xffff) sum = 0;
	return (sum);
}
