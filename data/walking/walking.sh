#!/bin/sh

while : ; do
	for i in *.txt ; do
		clear
		echo $i
		echo
		cat $i
		sleep 0.25
	done

done
