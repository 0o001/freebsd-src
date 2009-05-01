#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/symlink/02.t,v 1.1.8.1 2009/04/15 03:14:26 kensmith Exp $

desc="symlink returns ENAMETOOLONG if a component of the name2 pathname exceeded 255 characters"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..7"

n0=`namegen`

expect 0 symlink ${name255} ${n0}
expect 0 unlink ${n0}
expect 0 symlink ${n0} ${name255}
expect 0 unlink ${name255}

expect ENAMETOOLONG symlink ${n0} ${name256}
expect 0 symlink ${name256} ${n0}
expect 0 unlink ${n0}
