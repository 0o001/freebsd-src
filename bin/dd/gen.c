/*-
 * This program is in the public domain
 *
 * $FreeBSD: src/bin/dd/gen.c,v 1.2.20.1 2009/04/15 03:14:26 kensmith Exp $
 */

#include <stdio.h>

int
main(int argc __unused, char **argv __unused)
{
	int i;

	for (i = 0; i < 256; i++)
		putchar(i);
	return (0);
}
