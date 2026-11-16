#!/bin/bash
#usage: ./Makedir.sh

#Detectors:
#DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)
#For crosstalk corrections
#DETS=(00A 03A 03B 06B 06C 10C 12C 14B)
#For neutron damage corrections
#DETS=(00B 00C 04A 04B 04C 05A 05B 05C 10A 10B 10C 12B 14A 14B 14C)
#DETS=(00B 03A 05B)
DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)
time=( 21.561
 27.467
 22.626
 26.222
 27.226
 23.676
 24.213
-30.125
 20.425
 19.910
-41.033
-27.243
-29.252
-32.354
-31.865
 28.438
 23.260
 18.267
-19.908
-21.806
-26.559
 27.733
-21.990
-28.886)

j=0
for i in "${DETS[@]}"
do 

	#Dets without Neutron Damage
	if [ ${i} == '03A' ] || [ ${i} == '03B' ] || [ ${i} == '03C' ] || [ ${i} == '06A' ] || [ ${i} == '06B' ] || [ ${i} == '06C' ] || [ ${i} == '12A' ] || [ ${i} == '12B' ]
	then

		echo "'${i}': (\"TimeShiftCC  ${time[j]}\",\"ForceSegmentsToCore\"),"
	#Don't use ForceSegmentToCore because the core has broad FWHM
	elif [ ${i} == '05B' ]
	then
		echo "'${i}': (\"TimeShiftCC  ${time[j]}\",\"TrappingFile Trapping.cal\"), #NO FORCE SEG! Core with bad resolution"
	#00A Trapping file missing, Don't use ForceSegmentToCore because the core has ghost peaks
	elif  [ ${i} == '00A' ]
	then
		echo "'${i}': (\"TimeShiftCC  ${time[j]}\"), #NO FORCE SEG! Core with ghost peaks, Trapping.cal file missing"
	else
		echo "'${i}': (\"TimeShiftCC  ${time[j]}\",\"TrappingFile Trapping.cal\",\"ForceSegmentsToCore\"),"
	fi
	((j++))
	
	
done
 #for (( c=1; c<=5; c++ ))
