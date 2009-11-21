#!/bin/sh

# $FreeBSD: src/tools/regression/usr.bin/make/archives/fmt_44bsd_mod/test.t,v 1.1.22.1.2.1 2009/10/25 01:10:29 kensmith Exp $

cd `dirname $0`
. ../../common.sh

# Description
DESC="Archive parsing (modified BSD4.4 format)."

# Setup
TEST_COPY_FILES="libtest.a 644"

# Run
TEST_N=7

eval_cmd $*
