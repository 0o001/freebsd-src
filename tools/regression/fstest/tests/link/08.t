#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/link/08.t,v 1.1.14.1 2010/12/21 17:10:29 kensmith Exp $

desc="link returns ELOOP if too many symbolic links were encountered in translating one of the pathnames"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..10"

n0=`namegen`
n1=`namegen`
n2=`namegen`

expect 0 symlink ${n0} ${n1}
expect 0 symlink ${n1} ${n0}
expect ELOOP link ${n0}/test ${n2}
expect ELOOP link ${n1}/test ${n2}
expect 0 create ${n2} 0644
expect ELOOP link ${n2} ${n0}/test
expect ELOOP link ${n2} ${n1}/test
expect 0 unlink ${n0}
expect 0 unlink ${n1}
expect 0 unlink ${n2}
