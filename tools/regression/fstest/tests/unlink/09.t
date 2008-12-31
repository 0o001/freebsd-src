#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/unlink/09.t,v 1.1.6.1 2008/11/25 02:59:29 kensmith Exp $

desc="unlink returns EPERM if the named file has its immutable, undeletable or append-only flag set"

dir=`dirname $0`
. ${dir}/../misc.sh

require chflags

echo "1..30"

n0=`namegen`

expect 0 create ${n0} 0644
expect 0 chflags ${n0} SF_IMMUTABLE
expect EPERM unlink ${n0}
expect 0 chflags ${n0} none
expect 0 unlink ${n0}

expect 0 create ${n0} 0644
expect 0 chflags ${n0} UF_IMMUTABLE
expect EPERM unlink ${n0}
expect 0 chflags ${n0} none
expect 0 unlink ${n0}

expect 0 create ${n0} 0644
expect 0 chflags ${n0} SF_NOUNLINK
expect EPERM unlink ${n0}
expect 0 chflags ${n0} none
expect 0 unlink ${n0}

expect 0 create ${n0} 0644
expect 0 chflags ${n0} UF_NOUNLINK
expect EPERM unlink ${n0}
expect 0 chflags ${n0} none
expect 0 unlink ${n0}

expect 0 create ${n0} 0644
expect 0 chflags ${n0} SF_APPEND
expect EPERM unlink ${n0}
expect 0 chflags ${n0} none
expect 0 unlink ${n0}

expect 0 create ${n0} 0644
expect 0 chflags ${n0} UF_APPEND
expect EPERM unlink ${n0}
expect 0 chflags ${n0} none
expect 0 unlink ${n0}
