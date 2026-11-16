#!/bin/bash
#./check_xtalk_dead.sh det seg deadXsg deadXcc



filename='event_energy.bdat.0000'

DET=$1
SEG=$2
#FactorS in femul
deadXsg=$3
#FactorC in femul, 0 for missing or unstable segments
deadXcc=$4
###Prepare the ecalF1.cal file from recal

if [ ! -d Data_bdat ] ; then
           
	echo -e "${RED} Missing the link of the bdat files directory to Data_bdat ${NC}"
	exit 1
fi 

if [ ! -d ${DET} ] ; then   mkdir ${DET} ; fi 

cd ${DET}
tail -n 38 ../../1-encal/${DET}/cal.txt | tee recal_tail.txt; awk '{print "0 " $1 " 2  0 " $14 }' recal_tail.txt | tee ecalF1.cal

###Copy inverse xtalk coefficents 
cp ../../Conf/${DET}/xinv_1325-1340.cal .
#Generate spetra to check the results:
xTalkSort -ifile Data_bdat/${DET}/${filename} -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -recalXT xinv_1325-1340.cal
cd -



## Files generated:
# xinv_1325-1340.cal
#proj__3-38-32768-UI__raw.spec    raw projections:      [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__cal.spec    calibrated spectra:   [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__adj.spec    adjusted spectra:     [0]=Tot [1]=SG@F1 [2]=CC@F1



# redo xtalk missing segs 01B 04A?
#'04A' : ("DeadSegment    8 0.956613 0.002750"),
#'08A' : ("DeadSegment    8  0.9555363  0."),
#'11A' : ("DeadSegment    30  0.9505  0."),
#'01B' : ("DeadSegment      1  0.94747  0."),
#'00B' : ("DeadSegment    12  0.936377  0"),
#'05B' : ("DeadSegment    24  0.952100  0."),
#'06C' : ("DeadSegment    17  0.953663  0"),
#'08A' : ("DeadSegment    9  0.956071  0"),
#'08B' : ("DeadSegment    8  0.949936  0"),
#'11A' : ("DeadSegment    30  0.949938  0."),
#'06C' : ("DeadSegment    18  0.949078  0"),

#./check_xtalk_dead.sh 05B 24  0.952100  0.
#./check_xtalk_dead.sh 11A 30  0.949938  0.
#./check_xtalk_dead.sh 08B 8  0.949936  0.
