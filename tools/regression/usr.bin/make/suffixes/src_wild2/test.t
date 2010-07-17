#!/bin/sh

# $FreeBSD: src/tools/regression/usr.bin/make/suffixes/src_wild2/test.t,v 1.1.22.1.4.1 2010/06/14 02:09:06 kensmith Exp $

cd `dirname $0`
. ../../common.sh

# Description
DESC="Source wildcard expansion (2)."

# Setup
TEST_COPY_FILES="TEST1.a 644	TEST2.a 644"

# Reset
TEST_CLEAN="TEST1.b"

# Run
TEST_N=1
TEST_1="-r test1"

eval_cmd $*
