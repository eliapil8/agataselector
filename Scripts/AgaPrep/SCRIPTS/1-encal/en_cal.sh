#!/bin/bash


### check the directories exists
if ! test -d Data    ; then echo -e $FAIL"Data folder not found (need to run in the 1-encal folder)"$ENDC ; exit 1 ; fi

## check detectors are defined
if [[ -z $currentDets ]] ; then echo -e $FAIL"currentDets variable empty, need to export currentDets=..."$ENDC ; exit 1; fi


 
for i in $currentDets ; do


        echo "=====${i}====="
        if [ ! -d ${i} ] ; then   mkdir ${i} ; fi

        cd ${i}


	RecalEnergy -spe ../Data/$i/Prod__4-38-32768-UI__Ampli.spec -sub 38 -num 38 -gain 2 -60Co -dwa 20 5 -max | tee cal.txt;


	cd -

done



