#!/bin/sh
# $FreeBSD: src/tools/regression/lib/libc/resolv/resolv.t,v 1.1.22.1.8.1 2012/03/03 06:15:13 kensmith Exp $

do_test() {
	number=$1
	comment=$2
	opt=$3
	if ./$executable $opt; then
		echo "ok $number - $comment"
	else
		echo "not ok $number - $comment"
	fi
}

cd `dirname $0`

executable=`basename $0 .t`

make $executable 2>&1 > /dev/null

echo 1..3
do_test 1 'getaddrinfo()'     '-r mach'
do_test 2 'gethostbyname()'   '-rH mach'
do_test 3 'getipnodebyname()' '-rI mach'
