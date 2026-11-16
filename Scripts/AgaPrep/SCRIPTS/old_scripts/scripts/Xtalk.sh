#!/bin/bash
#usage: ./Xtalk.sh

#Detectors:
DETS=(00A 00B 00C 02A 02B 03A 03B 03C 04A 04B 04C 10A 10B 10C 11A 11B 11C 12A 12B 12C 13A 13B 13C)
#DETS=(12A)


for i in "${DETS[@]}"
do 

	cd ${i}

	tail -n 38 recal.txt | tee recal_tail.txt; awk '{print "0 " $1 " 2  0 " $14 }' recal_tail.txt | tee ecalF1.cal
 
	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/${i}/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -specXT -trigewin 1325 1340

	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/RecalEnergy -spe xspe__36-37-16384-UI__cal.spec -num 1332 -ener 1332.5 -gain 5 -offs 1000 -Xtalk 37 > xdir_1325-1340.txt 

	grep -v "^#" xdir_1325-1340.txt | grep -v "^ *36 " | cut -b15-102 --complement | tee xdir_1325-1340.cal

	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkInvert -f xdir_1325-1340.cal

	mv xdir_1325-1340.cal.inv xinv_1325-1340.cal
	
	cd -
done
