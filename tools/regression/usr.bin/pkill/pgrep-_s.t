#!/bin/sh
# $FreeBSD: src/tools/regression/usr.bin/pkill/pgrep-_s.t,v 1.1.2.1.4.1 2010/06/14 02:09:06 kensmith Exp $

base=`basename $0`

echo "1..2"

name="pgrep -S"
pid=`pgrep -Sx g_event`
if [ "$pid" = "2" ]; then
	echo "ok 1 - $name"
else
	echo "not ok 1 - $name"
fi
pid=`pgrep -x g_event`
if [ "$pid" != "2" ]; then
	echo "ok 2 - $name"
else
	echo "not ok 2 - $name"
fi
