#!/bin/bash

############################## SCRIPT TO RECALIBRATE ONLY SEGMENTS/CORES ################################
##
##Before running the script do a link to the postpsa files and name it Data_post (even if it is in Out)
##
##Crystals should all have the trapping file (the ones without ND have to be with a fake trapping file)
##
#########################################################################################################


#Defining extra command to set the FWHM ranges  
ExtraCmd='-dwa 10 5'
#Defining the source used for recalibration
Source='-226Ra'



if [ ! -d Data_post ] ; then
           
	echo -e "${RED} Missing the link of the post files directory to Data_post ${NC}"
        exit 1

fi 



## All the crystals need to be here
DETND="00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C 14A 14B 14C"


### Cores ###
for i in $DETND
do
        if [ ! -d ${i} ] ; then   mkdir ${i} ; fi


# Getting the re-calibration parameters for the Core
	RecalEnergy -spe Data_post/${i}/Post__5-40-16384-UI__Ener.spec -sub 159 -num 1 -gain 4 -poly1 ${Source} ${ExtraCmd} | tee ${i}/recalCore.log
	
# Extracting the re-calibration parameters for the Core
	tail -n 1 ${i}/recalCore.log |tee ${i}/recalCore_nohead.log

	test=$(awk -F' ' '{printf $17}' ${i}/recalCore_nohead.log)


# Checking if the values obtained make sense	
	if [[ (${test} > 1.009) ||  (${test} < 0.998) ]] ; then
	
		echo -n ${i} | tee ${i}/RecalCore_final.log
		awk -F' ' '{printf "  \"RecalCC %6.3f %.6f\" <==============\n",$16,$17}' ${i}/recalCore_nohead.log  | tee -a ${i}/RecalCore_final.log
	else 
		echo -n ${i} | tee ${i}/RecalCore_final.log
		awk -F' ' '{printf "  \"RecalCC %6.3f %.6f\"\n",$16,$17}' ${i}/recalCore_nohead.log  | tee -a ${i}/RecalCore_final.log
	fi
	sed -i 's/RecalCC  0.000 0.000000" <==============/RecalCC  0.000 1.000000" file not found/g' ${i}/RecalCore_final.log 
	
	
done



### Sum of Segments ###
for i in $DETND
do
        if [ ! -d ${i} ] ; then   mkdir ${i} ; fi


# Getting the re-calibration parameters for the Sum of Segments
	RecalEnergy -spe Data_post/${i}/Post__5-40-16384-UI__Ener.spec -sub 78 -num 1 -gain 4 -poly1 ${Source} ${ExtraCmd} | tee ${i}/recalSSeg.log
	
# Extracting the re-calibration parameters for the Sum of Segments
	tail -n 1 ${i}/recalSSeg.log |tee ${i}/recalSSeg_nohead.log
        
	test=$(awk -F' ' '{printf $17}' ${i}/recalSSeg_nohead.log)
	

# Checking if the values obtained make sense  
	if [[ (${test} > 1.009) ||  (${test} < 0.998) ]] ; then
	
		echo -n ${i} | tee ${i}/RecalSSeg_final.log
		awk -F' ' '{printf "  \"RecalSG %6.3f %.6f\" <==============\n",$16,$17}' ${i}/recalSSeg_nohead.log  | tee -a ${i}/RecalSSeg_final.log
	else 	
		echo -n ${i} | tee ${i}/RecalSSeg_final.log
		awk -F' ' '{printf "  \"RecalSG %6.3f %.6f\"\n",$16,$17}' ${i}/recalSSeg_nohead.log  | tee -a ${i}/RecalSSeg_final.log
	fi
	sed -i 's/RecalSG  0.000 0.000000" <==============/RecalSG  0.000 1.000000" file not found/g' ${i}/RecalSSeg_final.log 

done





echo "===Core==="

cat ???/RecalCore_final.log | tee RecalCore_final.log

echo "===SumSeg==="

cat ???/RecalSSeg_final.log | tee RecalSSeg_final.log


echo -e $HEADER"\n ########### Remember to update the gen_conf.py! ############ "$ENDC 

