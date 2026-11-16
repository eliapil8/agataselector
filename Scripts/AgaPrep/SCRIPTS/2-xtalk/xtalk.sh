#!/bin/bash

#Before running the script do a link to the bdat files and name it Data_bdat (even if it is in Out)

### check the directories exists
if ! test -d Data_bdat    ; then echo -e $FAIL"Data_bdat folder not found"$ENDC ; exit 1 ; fi


## check detectors are defined
if [[ -z $currentDets ]] ; then echo -e $FAIL"currentDets variable empty, need to export currentDets=..."$ENDC ; exit 1; fi 


filename='event_energy.bdat.0000'


for i in $currentDets
do

     
        if [ ! -d ${i} ] ; then   mkdir ${i} ; fi 
			
	cd $i
	
	###Prepare the ecalF1.cal file from PreprocessingFilterPSA.conf
	#for j in {1..38}
	#do 
		#cat ../../Conf/${i}/PreprocessingFilterPSA.conf | head -n $j |awk -v var=$(( $j - 1 )) '{print "0\t" var "\t2\t0\t" $5}' | tail -n 1 >> ecalF1.cal
	#done
	#cat ecalF1.cal
	

	###Prepare the ecalF1.cal file from cal.txt
        tail -n 38 ../../1-encal/${i}/cal.txt | tee recal_tail.txt; awk '{print "0 " $1 " 2  0 " $14 }' recal_tail.txt | tee ecalF1.cal


	xTalkSort -ifile ../Data_bdat/${i}/${filename} -ecalF1 ecalF1.cal -egain 5 -specXT -trigewin 1325 1340

	RecalEnergy -spe xspe__36-37-16384-UI__cal.spec -num 1332 -ener 1332.5 -gain 5 -offs 1000 -Xtalk 37 > 1325-1340.txt; grep -v "^#"  1325-1340.txt | grep -v "^ *36 " | cut -b15-102 --complement > xdir_1325-1340.cal

	xTalkInvert -f xdir_1325-1340.cal; mv xdir_1325-1340.cal.inv xinv_1325-1340.cal

	xTalkSort -ifile ../Data_bdat/${i}/${filename} -ecalF1 ecalF1.cal -egain 5 -recalXT xinv_1325-1340.cal

	cd -



done



# Text message for copying the xtalk files
echo -e $HEADER"\n You can now copy the xtalk files with: "$ENDC
echo -e $HEADER"for i in \$currentDets; do  cp \${i}/xinv_1325-1340.cal ../Conf/\${i}/.; cp \${i}/xdir_1325-1340.cal ../Conf/\${i}/. ; done; \n "$ENDC



#Files generated:
# xdir_1325-1340.cal
# xinv_1325-1340.cal
#proj__3-38-32768-UI__raw.spec    raw projections:      [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__cal.spec    calibrated spectra:   [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__adj.spec    adjusted spectra:     [0]=Tot [1]=SG@F1 [2]=CC@F1
#ssum__2-12-16384-UI__cal.spec    sumEnergy[segFold]:   [0]=SG  [1]=CC
#ssum__2-12-16384-UI__adj.spec    sumAdjusted[segFold]: [0]=SG  [1]=CC

