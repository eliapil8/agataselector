#!/bin/bash
#usage: ./Calibration.sh
#Before running the script do a link to the data you want to calibrate and name it Data (even if it is in Out)
#Detectors:
DETS='00A 00B 00C 01A 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 09A 09B 09C 10A 10B 10C 11A 11B 11C 14A 14B 14C'
#DETS='00B 08A 08B'
for i in $DETS
do
	if [ ! -d ${i} ] ; then   mkdir ${i} ; fi 
	echo "=== Energy Calibration Crystal ${i} ==="
	RecalEnergy -spe Data/${i}/Prod__4-38-32768-UI__Ampli.spec -sub 38 -num 38 -gain 2 | tee ${i}/cal.txt
	
done
#nedit ???/recal.txt
