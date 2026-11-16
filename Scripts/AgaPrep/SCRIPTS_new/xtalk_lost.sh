#!/bin/bash
######PLEASE DO NOT CHANGE THE SCRIFT, JUST MODIFT THE NAME OF THE FOLDER THAT YOU WANT TO WORK WITH
#Detectors e682:
#DETS=(00A 00B 00C 02A 02B 03A 03B 03C 04A 04B 04C 10A 10B 10C 11A 11B 11C 12A 12B 12C 13A 13B 13C)
# ===========================================================================
# ===> CROSS TALK correction for detector with LOST or UNSTABLE segments <===
# ===========================================================================
# Before using the script:
#  - Link to data
#  - cd det directory
#  - Put the calibration coeff to zero in the file ecalF1.cal
#  - Run the command: xTalkSort -ifile ../Data/13A/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -matCCSG
#    to get the deadXsg factor from the matrix CC-SG__50-1500-1500-UI__ma.matr
#

#THIS IS TO CREATE ecal FILE
#for j in {1..38}
	#do 
		#cat ../../Conf/${i}/PreprocessingFilterPSA.conf | head -n $j |awk -v var=$(( $j - 1 )) '{print "0\t" var "\t2\t0\t" $5}' | tail -n 1 >> ecalF1.cal
	#done
	#cat ecalF1.cal


# Usage (execute inside the det directory): ./Xtalk_lost.sh det seg deadXsg deadXcc

DET=$1
SEG=$2
#FactorS in femul
deadXsg=$3
#FactorC in femul, 0 for missing or unstable segments
deadXcc=$4


 	#Build the crosstalk matrix:
	 xTalkSort -ifile ../Data/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -matx1

	#Generate the crosstalk matrix coefficients:
	xTalkMake -f xSG__36-36-100-1536-US__ij.matr
	
        #Generate spectra to check the results:
	xTalkSort -ifile ../Data/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -recalXT xinv_1325-1340.cal | tee coeff_$SEG.log
	
	#TalkSort -ifile ../Data/14B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -specXT -deadSeg 34 0 0 -trigewin 1325 1340
	
#proj__3-38-32768-UI__raw.spec
#spec__3-38-16384-UI__cal.spec
#ssum__2-12-16384-UI__cal.spec
#xspe__36-37-16384-UI__cal.spec

#RecalEnergy -spe xspe__36-37-16384-UI__cal.spec -num 1332 -ener 1332.5 -gain 5 -offs 1000 -Xtalk 37 | tee  dead.txt

#grep -v "^#"  dead.txt | grep -v "^ *36 " | cut -b15-102 --complement | tee xdir_dead.cal
#xTalkInvert -f xdir_dead.cal

#TalkSort -ifile ../Data/14B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -specXT -deadSeg 34 0 0 -trigewin 1325 1340

# =================================================================
# ===> CROSS TALK correction for detector with BROKEN segments <===
# =================================================================
#
# Before launching the commands:
#  - Link to data
#  - cd det directory
#  - Put the calibration coeff to zero in the file ecalF1.cal


#Get the deadXsg and the deadXcc factors from the matrix CC-SG__50-1500-1500-UI__ma.matr 
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -matCCSG
   
#Check the deadXsg and the deadXcc factors from the matrix CC-SG__50-1500-1500-UI__ma.matr
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 5 0.9483 0.01621305 -matCCSG

#Build the crosstalk matrix:
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 5 0.9483 0.01621305 -matx1

#Generate the crosstalk matrix coefficients:
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkMake -f xSG__36-36-1000-100-US__ij.matr -dxy 1000 100 -gate 55 75 -xcal -15 5 -egam 1332.5 -dval 5 0.780120075

#Generate spetra to check the results:
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 5 0.9483 0.01621305 -recalXT xinv_0055-0075.cal | tee coeff_5.log
##03B

#xTalkSort -ifile ../Data/03B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -matCCSG

#xTalkSort -ifile ../Data/03B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 26 0.94067275 0.022965984  -matCCSG

#xTalkSort -ifile ../Data/03B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 26 0.94067275 0.022965984 -matx1

#xTalkMake -f xSG__36-36-1000-100-US__ij.matr -dxy 1000 100 -gate 55 75 -xcal -15 5 -egam 1332.5 -dval 26 0.65881576


#xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 26 0.94067275 0.022965984 -recalXT xinv_0055-0075.cal

#00A
#xTalkSort -ifile ../Data/00A/event_energy.bdat.0001 -ecalF1 ecalF1.cal -egain 5 -deadSeg 29 0.94897140 0.048672204 -matCCSG
#xTalkSort -ifile ../Data/00A/event_energy.bdat.0001 -ecalF1 ecalF1.cal -egain 5 -deadSeg 29 0.94897140 0.048672204 -matx1
#xTalkMake -f xSG__36-36-1000-100-US__ij.matr -dxy 1000 100 -gate 55 75 -xcal -15 5 -egam 1332.5 -dval 29 0.54932315

xTalkSort -ifile ../Out/05C/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -matCCSG

