#!/bin/sh
# $FreeBSD: src/tools/regression/fstest/tests/unlink/13.t,v 1.1.10.1.4.1 2010/06/14 02:09:06 kensmith Exp $

desc="unlink returns EFAULT if the path argument points outside the process's allocated address space"

dir=`dirname $0`
. ${dir}/../misc.sh

echo "1..2"

expect EFAULT unlink NULL
expect EFAULT unlink DEADCODE
