#!/bin/bash
#usage: ./Makedir.sh

#Detectors:
#DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)
#For crosstalk corrections
#DETS=(00A 03A 03B 06B 06C 10C 12C 14B)
#For neutron damage corrections
#DETS=(00A 00B 00C 04A 04B 04C 05A 05B 05C 10A 10B 10C 12B 14A 14B 14C)
#All dets except 00C and 13C
DETS=(00A 00C 01A 01B 01C 02A 02B 02C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 09A 09C 10A 10B 10C 12A 12B 12C 13A 13B 14A 14B 14C)

for i in "${DETS[@]}"
do 
	d=${i}
	if [ ! -d "$d" ]
	then
		mkdir ${i}
		echo "Directory ${i} generated"
	else
		echo "Directory ${i} exists"
	fi
	
done
#ls -l
