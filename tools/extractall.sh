#!/bin/sh

start=25
end=130
i=$start

while ! [ "$i" = "$end" ]
do
	dir=0.0.0.$i
	echo $i
	if test -d $dir
	then
		./unraf $dir/Archive_*.raf $dir/Archive_*.raf.dat
		ret=$?
		if ! [ "$ret" = "0" ]
		then
			echo Exiting due to command failure.
			exit 1
		fi
	fi
	i=$(($i + 1))
done
