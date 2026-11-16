#!/bin/bash
#usage: ./Time.sh

#Detectors:
DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)


for i in "${DETS[@]}"
do 
	echo "=== Time Alignement Segments Crystal ${i} ==="
	RecalEnergy -spe Data/${i}/Prep__6-40-1000-UI__TT.spec -sub 40 -num 36 -T 500 | tee ${i}/shift.txt
	
done
#nedit ???/shift.txt
