#!/bin/bash
#Before running the script do a link to the data you want to calibrate and name it Data_bdat (even if it is in Out)

#for i in 00A 00B 00C 01A 01B 01C 02A 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C 
#for i in 00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C


DETS='00A 00B 00C 01A 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 09A 09B 09C 10A 10B 10C 11A 11B 11C 14A 14B 14C'

for i in $DETS

do 

	echo "=====${i}====="
	if [ ! -d ${i} ] ; then   mkdir ${i} ; fi 

	cd ${i}
	###Prepare the ecalF1.cal file from recal
	tail -n 38 cal.txt | tee cal_tail.txt; awk '{print "0 " $1 " 2  0 " $14 }' cal_tail.txt | tee ecalF1.cal
	###Generate spetra to check the calibration:
	xTalkSort -ifile ../Data_bdat/${i}/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5
	cd -

done

#Spectra generated:
#proj__3-38-32768-UI__raw.spec    raw projections:      [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__cal.spec    calibrated spectra:   [0]=Tot [1]=SG@F1 [2]=CC@F1
#ssum__2-12-16384-UI__cal.spec    sumEnergy[segFold]:   [0]=SG  [1]=CC
#(check spec__3-38-16384-UI__cal.spec[0][0-37] in $i directory) 

