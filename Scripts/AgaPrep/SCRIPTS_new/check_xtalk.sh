#!/bin/bash


RED='\e[1m\e[38;5;196m'
NC='\e[0m'
#Before running the script do a link to the bdat files and name it Data_bdat (even if it is in Out) ln -s ../../Out ../Data_bdat

DETS='00A 00B 00C 01A 01C 02A 02B 02C 04A 04B 04C 05A 05C 06A 06B 06C 07A 07B 07C 08A 09A 09B 09C 10A 10B 10C 11B 11C 14A 14B 14C'


filename='event_energy.bdat.0000'


 	
        if [ ! -d ../Data_bdat ] ; then
           
           echo -e "${RED} Missing the link of the bdat files directory to Data_bdat ${NC}"
           exit 1
        fi 
	

for i in $DETS

do 
	echo "=====${i}====="
	if [ ! -d ${i} ] ; then   mkdir ${i} ; fi 

	cd ${i}
	
	###Prepare the ecalF1.cal file from recal
	tail -n 38 ../../../1-cal/${i}/cal.txt | tee recal_tail.txt; awk '{print "0 " $1 " 2  0 " $14 }' recal_tail.txt | tee ecalF1.cal
	
	###Copy inverse xtalk coefficents 
	cp ../../../Conf/${i}/xinv_1325-1340.cal .
	
	###Generate spetra to check the calibration and Xtalk:
	xTalkSort -ifile ../Data_bdat/${i}/${filename} -ecalF1 ecalF1.cal -egain 5 -recalXT xinv_1325-1340.cal
	
	cd -

done

#./check_xtalk_dead.sh det seg deadXsg deadXcc

#DET=$1
#SEG=$2
#FactorS in femul
#deadXsg=$3
#FactorC in femul, 0 for missing or unstable segments
#deadXcc=$4

#Generate spetra to check the results:
#xTalkSort -ifile ../Data/${DET}/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -recalXT


# redo normal xtalk 00A 08C 05A 05B 05C 14A 14B 14C 
# redo xtalk missing segs 01B 04B?
