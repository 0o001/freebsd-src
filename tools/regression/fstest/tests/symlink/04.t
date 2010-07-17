#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/symlink/04.t,v 1.1.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $

desc="symlink returns ENOENT if a component of the name2 path prefix does not exist"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..3"

n0=`namegen`
n1=`namegen`

expect 0 mkdir ${n0} 0755
expect ENOENT symlink test ${n0}/${n1}/test
expect 0 rmdir ${n0}
