#!/bin/sh
# $FreeBSD: src/tools/regression/netinet/rawconnect/rawconnect.t,v 1.1.10.1.8.1 2012/03/03 06:15:13 kensmith Exp $

cd `dirname $0`

executable=`basename $0 .t`

make $executable 2>&1 > /dev/null

echo 1..1

comment="rawconnect # open raw ip socket, connect it and then close"

uid=`id -u`

if [ $uid -ne 0 ]; then
	echo "ok 1 - rawconnect # skipped: you need to be root to run this test"
elif ./$executable; then
	echo "ok 1 - $comment"
else
	echo "not ok 1 - $comment"
fi
