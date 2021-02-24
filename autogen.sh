#!/bin/sh

# Does everything needed to get a fresh cvs checkout to compilable state.

set -e
# echo "running aclocal"
# aclocal
# echo "running automake"
# automake --add-missing
echo "running autoconf"
autoconf
echo "running autoheader"
autoheader
# autoreconf -vfi
echo "running ./configure $@"
./configure "$@"

