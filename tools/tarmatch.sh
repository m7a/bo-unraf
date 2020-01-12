#!/bin/sh

term=leona
dir="League of Legends"
out=sel_$$.tar

if test -f "$out"
then
	echo $out already exists.
	exit 1
fi

list=/tmp/ma_list_$$.txt
grep -FRsli $term "$dir" > $list
tar -cf $out -T $list
rm $list
