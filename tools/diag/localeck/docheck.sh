#!/bin/sh
#
# Validate all locales installed in specified directory
# (by default check system locales)
#
# $FreeBSD: src/tools/diag/localeck/docheck.sh,v 1.2.36.1.8.1 2012/03/03 06:15:13 kensmith Exp $
#

LOCALEDIR=/usr/share/locale

if [ "$1" != "" ]; then
	LOCALEDIR=$1
fi

if [ ! -x ./localeck ]; then
	echo "ERROR: build test program first."
	exit 1
fi

PATH_LOCALE=$LOCALEDIR
LOCALES=0
ERRORS=0

echo "Validating locales in $LOCALEDIR"
echo

for i in `ls -1 $LOCALEDIR`
do
	LOCALES=$(($LOCALES + 1))
	./localeck $i || ERRORS=$(($ERRORS + 1))
done

echo
echo "Validation test complete"
echo "$LOCALES locales were checked"
echo "$ERRORS invalid locales were found"

