/*
 * Offsets into structures used from asm.  Must be kept in sync with
 * appropriate headers.
 *
 * $FreeBSD: src/lib/libkse/arch/sparc64/sparc64/assym.s,v 1.3.6.1 2008/11/25 02:59:29 kensmith Exp $
 */

#define	UC_MCONTEXT	0x40

#define	MC_FLAGS	0x0
#define	MC_VALID_FLAGS	0x1
#define	MC_GLOBAL	0x0
#define	MC_OUT		0x40
#define	MC_TPC		0xc8
#define	MC_TNPC		0xc0
