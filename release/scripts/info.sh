#!/bin/sh
#
# $ANA: info.sh,v 1.3 1996/06/04 16:25:30 wollman Exp $
# $FreeBSD: src/release/scripts/info.sh,v 1.5.36.1.2.1 2009/10/25 01:10:29 kensmith Exp $
#

ls $1.[a-z][a-z] | wc | awk '{ print "Pieces = ",$1 }'
for FILE in $1.[a-z][a-z]; do
       PIECE=`echo $FILE | cut -d . -f 2`
       echo -n "cksum.$PIECE = "
       cksum $FILE | awk ' { print $1,$2 } '
done
