#!/bin/bash
cd ~/.nget3/ || exit
ok=0
for a in *.gz ; do
	if [ ! "$a" = "*.gz" ]; then
		ok=1
		break
	fi
done
if [ "$ok" = "0" ]; then
	echo 'hm, nothing to do?  Perhaps you have no cache files sitting around.'
	echo 'Or perhaps you compiled nget without zlib support? (this script only'
	echo 'handles renaming of *.gz files, since I doubt anyone actually uses'
	echo 'nget without zlib.)'
	exit
fi

for a in *.gz; do
	if echo -n $a | grep -q '_midinfo.gz$' ; then
		mv -iv $a `echo $a | sed s/_midinfo.gz/,midinfo.gz/`
	elif echo -n $a | egrep -q ',(midinfo|cache).gz$' ; then
		echo "$a appears to have the correct name already"
	else
		mv -iv $a `echo $a | sed s/.gz/,cache.gz/`
	fi
done

