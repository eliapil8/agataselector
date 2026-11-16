#!/bin/bash
#usage: ./Xtalk.sh

#Detectors:
#DETS=(03A 06C)
#DETS=(00A 03B 06B 14B)
DETS=(06A)

for i in "${DETS[@]}"
do 

	cd ${i}
	echo "--- Crosstalk det${i} ---"
	cp ../../run_0001/Conf/${i}/PreprocessingFilterPSA.conf .

	head -n 38 PreprocessingFilterPSA.conf | tee PrepFilter_head.txt
  	sed -i -e'/core/s/ 0 /36 /g' PrepFilter_head.txt 
  	sed -i -e'/core/s/ 1 /37/g' PrepFilter_head.txt 
  	awk '{print "0 " $2 " 2 0 " $5}' PrepFilter_head.txt | tee ecalF1.cal
	
	## Matrix for detector with missing segments
	#xTalkSort -ifile ../Data/${i}/event_energy.bdat.0001 -ecalF1 ecalF1.cal -egain 5 -matCCSG
 
 	xTalkSort -ifile ../Data/${i}/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -specXT -trigewin 1325 1340

	RecalEnergy -spe xspe__36-37-16384-UI__cal.spec -num 1332 -ener 1332.5 -gain 5 -offs 1000 -Xtalk 37 > xdir_1325-1340.txt 

	grep -v "^#" xdir_1325-1340.txt | grep -v "^ *36 " | cut -b15-102 --complement | tee xdir_1325-1340.cal

	xTalkInvert -f xdir_1325-1340.cal

	mv xdir_1325-1340.cal.inv xinv_1325-1340.cal
	
	cd -
done
