/*	$FreeBSD: src/contrib/ipfilter/lib/binprint.c,v 1.4.6.1 2008/11/25 02:59:29 kensmith Exp $	*/

/*
 * Copyright (C) 2000-2002 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: binprint.c,v 1.8.4.1 2006/06/16 17:20:56 darrenr Exp $
 */

#include "ipf.h"


void binprint(ptr, size)
void *ptr;
size_t size;
{
	u_char *s;
	int i, j;

	for (i = size, j = 0, s = (u_char *)ptr; i; i--, s++) {
		j++;
		printf("%02x ", *s);
		if (j == 16) {
			printf("\n");
			j = 0;
		}
	}
	putchar('\n');
	(void)fflush(stdout);
}
