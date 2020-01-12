#!/bin/sh

# Compare http://unix.stackexchange.com/questions/8430/how-to-remove-all-empty-directories-in-a-subtree

if [ "$1" = "" ]
then
	echo Use $0 --sure to start for security reasons.
elif [ "$1" = "--sure" ]
then
	find . -depth -type d -empty -exec rmdir {} \;
fi
