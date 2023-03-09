#!/bin/sh

while : ; do
#for i in *.txt; do
#for i in 30 10 15 20 00 ; do
#for i in 30 10 00 15 20 40 ; do
#for i in 50 10 40 25 20 ; do
#for i in 25 40 20 10 50 ; do
#	for i in 25 40 20 10 50 ; do
	for i in 00 40 60 50 ; do
		clear
		echo $i
		echo
		cat ${i}.txt
		sleep 0.20
	done
done

# 00 -> 15 = looks bad
# 20 -> 40 = looks bad

# 30 -> 10 = left foot in
# 10 -> 20 = left foot in
# 15 -> 40 = left foot in
# 30 -> 40 = left foot in

# 15 -> 20 = bend right leg
