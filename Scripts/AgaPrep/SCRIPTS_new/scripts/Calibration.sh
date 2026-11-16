#!/bin/bash
#usage: ./Calibration.sh


#DETS=(00B 08A 08B)
#for i in "${DETS[@]}"

#Detectors:
#DETS='00A 00C 01A 01B 01C 02A 02B 02C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 09A 09C 10A 10B 10C 12A 12B 12C 13A 13B 14A 14B 14C'
DETS='00B 08A 08B'
for i in $DETS
do 
	echo "=== Energy Calibration Crystal ${i} ==="
	RecalEnergy -spe Data/${i}/Prod__4-38-32768-UI__Ampli.spec -sub 38 -num 38 -gain 2 -dwa 30 2 | tee ${i}/recal.txt
	
done
#nedit ???/recal.txt
