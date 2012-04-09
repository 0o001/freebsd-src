#!/bin/sh
#
# $FreeBSD: src/release/scripts/info-make.sh,v 1.4.36.1.8.1 2012/03/03 06:15:13 kensmith Exp $
#

# Move the info files out to their own dist
if [ -d ${RD}/trees/base/usr/share/info ]; then
	tar -cf - -C ${RD}/trees/base/usr/share/info . |
		tar -xpf - -C ${RD}/trees/info/usr/share/info &&
	rm -rf ${RD}/trees/base/usr/share/info;
fi
