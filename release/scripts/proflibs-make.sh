#!/bin/sh
#
# $FreeBSD: src/release/scripts/proflibs-make.sh,v 1.8.30.1.6.1 2010/12/21 17:09:25 kensmith Exp $
#

# Move the profiled libraries out to their own dist
for i in ${RD}/trees/base/usr/lib/*_p.a; do
	mv $i ${RD}/trees/proflibs/usr/lib
done
