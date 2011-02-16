/*	$FreeBSD: src/contrib/ipfilter/ipsend/linux.h,v 1.2.26.1 2010/12/21 17:10:29 kensmith Exp $	*/

/*
 * Copyright (C) 1995-1998 by Darren Reed.
 *
 * This code may be freely distributed as long as it retains this notice
 * and is not changed in any way.  The author accepts no responsibility
 * for the use of this software.  I hate legaleese, don't you ?
 *
 * @(#)linux.h	1.1 8/19/95
 */

#include <linux/config.h>
#ifdef MODULE
#include <linux/module.h>
#include <linux/version.h>
#endif /* MODULE */

#include "ip_compat.h"
