/*	$NetBSD: trap.h,v 1.1 2001/02/23 03:48:19 ichiro Exp $	*/
/* $FreeBSD: src/sys/arm/include/trap.h,v 1.2.22.1.8.1 2012/03/03 06:15:13 kensmith Exp $ */

#ifndef _MACHINE_TRAP_H_
#define _MACHINE_TRAP_H_
#define GDB_BREAKPOINT		0xe6000011
#define GDB5_BREAKPOINT		0xe7ffdefe
#define PTRACE_BREAKPOINT	0xe7fffff0
#define KERNEL_BREAKPOINT	0xe7ffffff
#endif /* _MACHINE_TRAP_H_ */
