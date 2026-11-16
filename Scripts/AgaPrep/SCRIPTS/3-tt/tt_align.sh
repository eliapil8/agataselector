#!/bin/bash


### check the Data_tt directory exists
if ! test -d Data_tt    ; then echo -e $FAIL"Data_tt folder not found (Have you run the Replay after xtalk? Data_tt should be a link to the Replay Output)"$ENDC ; exit 1 ; fi


## check detectors are defined
if [[ -z $currentDets ]] ; then echo -e $FAIL"currentDets variable empty, need to export currentDets=..."$ENDC ; exit 1; fi
 

for i in $currentDets ; do
##Calculate TT shifts from the data in Out (after running a Replay)
RecalEnergy -spe Data_tt/$i/Prep__6-40-1000-UI__TT.spec -sub 40 -num 36 -T 500 | tee shift_${i}_TT.out

##Update TT columns in the Conf
#python /opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful/colupdate.py ../Conf/${i}/PreprocessingFilterPSA.conf shift_${i}_TT.out  -c 6 13 -o ../Conf/${i}/PreprocessingFilterPSA.conf


done
