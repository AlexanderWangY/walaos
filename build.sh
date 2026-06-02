#!/bin/sh
set -e
. ./headers.sh

# Enforce architecture boundaries (and any other static checks) before building.
./check.sh

for PROJECT in $PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done
