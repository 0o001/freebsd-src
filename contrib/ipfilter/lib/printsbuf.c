/*	$FreeBSD: src/contrib/ipfilter/lib/printsbuf.c,v 1.3.14.1 2010/12/21 17:10:29 kensmith Exp $	*/

/*
 * Copyright (C) 2002-2004 by Darren Reed.
 * 
 * See the IPFILTER.LICENCE file for details on licencing.  
 *   
 * $Id: printsbuf.c,v 1.2.4.2 2006/06/16 17:21:14 darrenr Exp $ 
 */     

#ifdef	IPFILTER_SCAN

#include <ctype.h>
#include <stdio.h>
#include "ipf.h"
#include "netinet/ip_scan.h"

void printsbuf(buf)
char *buf;
{
	u_char *s;
	int i;

	for (s = (u_char *)buf, i = ISC_TLEN; i; i--, s++) {
		if (ISPRINT(*s))
			putchar(*s);
		else
			printf("\\%o", *s);
	}
}

#endif
