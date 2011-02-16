#!/bin/sh

# $FreeBSD: src/tools/regression/usr.bin/make/shell/meta/test.t,v 1.2.22.1.6.1 2010/12/21 17:09:25 kensmith Exp $

cd `dirname $0`
. ../../common.sh

# Description
DESC="Check that a command line with meta characters is passed to the shell."

# Setup
TEST_COPY_FILES="sh 755"

# Run
TEST_N=2
TEST_1="-B no-meta"
TEST_2="-B meta"

eval_cmd $*
