#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/mkfifo/02.t,v 1.1.10.1.2.1 2009/10/25 01:10:29 kensmith Exp $

desc="mkfifo returns ENAMETOOLONG if a component of a pathname exceeded 255 characters"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..3"

expect 0 mkfifo ${name255} 0644
expect 0 unlink ${name255}
expect ENAMETOOLONG mkfifo ${name256} 0644
