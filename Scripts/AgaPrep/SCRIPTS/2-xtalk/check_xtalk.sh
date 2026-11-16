#!/bin/bash


#Before running the script do a link to the bdat files and name it Data_bdat (even if it is in Out) ln -s ../Out Data_bdat

#Check detectors are defined
if [[ -z $currentDets ]] ; then echo -e $FAIL"currentDets variable empty, need to export currentDets=..."$ENDC ; exit 1; fi 


filename='event_energy.bdat.0000'


# Check the directories exists
if ! test -d Data_bdat ; then echo -e $FAIL"Data_bdat folder not found"$ENDC ; exit 1 ; fi

	
# Info message 
echo -e $HEADER"You are going to check the crosstalk correction for detectors: $currentDets "$ENDC
echo -e $HEADER"Remember that 05B, 08B and 11A have been included with the dead segment correction."$ENDC
echo -e $HEADER"Do you want to continue? Y/N"$ENDC
read a
if [ $a != 'Y' ]; then exit 0 ; fi



for i in $currentDets

do 
	echo "=====${i}====="
	if [ ! -d ${i} ] ; then   mkdir ${i} ; fi 
	
			
	
	###Detectors with dead segments, check the xtalk correction coefficients!
	
	if [ ${i} == '05B' ]; then
		./check_xtalk_dead.sh 05B 24  0.952100  0.
	
	elif [ ${i} == '11A' ]; then 
		./check_xtalk_dead.sh 11A 30  0.949938  0.
	
	elif [ ${i} == '08B' ]; then 
		./check_xtalk_dead.sh 08B 8  0.949936  0.
	###-------------
	
	
	###Detectors with normal xtalk
	else
		cd ${i}
	
		###Prepare the ecalF1.cal file from recal
		tail -n 38 ../../1-encal/${i}/cal.txt | tee recal_tail.txt; awk '{print "0 " $1 " 2  0 " $14 }' recal_tail.txt | tee ecalF1.cal
	
		###Copy inverse xtalk coefficents 
		cp ../../Conf/${i}/xinv_1325-1340.cal .
	
		###Generate spetra to check the calibration and Xtalk:
		xTalkSort -ifile ../Data_bdat/${i}/${filename} -ecalF1 ecalF1.cal -egain 5 -recalXT xinv_1325-1340.cal
	
		cd -
	###-------------
	fi
done



# Info message 
echo -e $HEADER"Check the crosstalk correction for detector/s: $currentDets"$ENDC
echo -e $HEADER"in files:"$ENDC
echo -e $HEADER"spec__3-38-16384-UI__adj.spec -> check subspectra 0 (Total multiplicity) "$ENDC
echo -e $HEADER"ssum__2-12-16384-UI__adj.spec -> compare sum__2-12-16384-UI__ADJ.spec subspectra 0 (sum of segments after xtalk correction) "$ENDC
echo -e $HEADER"                                 with ssum__2-12-16384-UI__CAL.spec subspectra 0 (sum of segments, before xtalk correction)"$ENDC

## Files generated:
# xinv_1325-1340.cal
#proj__3-38-32768-UI__raw.spec    raw projections:      [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__cal.spec    calibrated spectra:   [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__adj.spec    adjusted spectra:     [0]=Tot [1]=SG@F1 [2]=CC@F1
#ssum__2-12-16384-UI__cal.spec    sumEnergy[segFold]:   [0]=SG  [1]=CC
#ssum__2-12-16384-UI__adj.spec    sumAdjusted[segFold]: [0]=SG  [1]=CC

