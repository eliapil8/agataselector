#!/bin/bash

RED='\e[1m\e[38;5;196m'
NC='\e[0m'
#Before running the script do a link to the bdat files and name it Data_bdat (even if it is in Out)

DET="01A 01C 02A 02B 02C 04A 04B 04C 07B 07C 09A 09B 09C 10A 10B 10C 11B 11C 14A 14B 14C" 
#DET="00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 14A 14B 14C" 
#DET="00B 00C 06A 06B 06C 07A 08A 08B"

filename='event_energy.bdat.0000'


        if [ ! -d Data_bdat ] ; then
           
           echo -e "${RED} Missing the link of the bdat files directory to Data_bdat ${NC}"
           exit 1
        fi 

for i in $DET
do

     
        if [ ! -d ${i} ] ; then   mkdir ${i} ; fi 
			
	cd $i
	#for j in {1..38}
	#do 
		#cat ../../Conf/${i}/PreprocessingFilterPSA.conf | head -n $j |awk -v var=$(( $j - 1 )) '{print "0\t" var "\t2\t0\t" $5}' | tail -n 1 >> ecalF1.cal
	#done
	#cat ecalF1.cal
	###Prepare the ecalF1.cal file from recal
        tail -n 38 ../../1-cal/${i}/cal.txt | tee recal_tail.txt; awk '{print "0 " $1 " 2  0 " $14 }' recal_tail.txt | tee ecalF1.cal


	xTalkSort -ifile ../Data_bdat/${i}/${filename} -ecalF1 ecalF1.cal -egain 5 -specXT -trigewin 1325 1340

	RecalEnergy -spe xspe__36-37-16384-UI__cal.spec -num 1332 -ener 1332.5 -gain 5 -offs 1000 -Xtalk 37 > 1325-1340.txt; grep -v "^#"  1325-1340.txt | grep -v "^ *36 " | cut -b15-102 --complement > xdir_1325-1340.cal

	xTalkInvert -f xdir_1325-1340.cal; mv xdir_1325-1340.cal.inv xinv_1325-1340.cal

	xTalkSort -ifile ../Data_bdat/${i}/${filename} -ecalF1 ecalF1.cal -egain 5 -recalXT xinv_1325-1340.cal

	cd -



done

