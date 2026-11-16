#!/bin/bash
#Before running the script do a link to the data you want to calibrate and name it Data_bdat (even if it is in Out)


### check the directories exists
if ! test -d Data_bdat    ; then echo -e $FAIL"Data_bdat folder not found"$ENDC ; exit 1 ; fi
 
## check detectors are defined 
if [[ -z $currentDets ]] ; then echo -e $FAIL"currentDets variable empty, need to export currentDets=..."$ENDC ; exit 1; fi


for i in $currentDets

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

