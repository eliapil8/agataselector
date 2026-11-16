#!/bin/bash
#Detectors e682:
#DETS=(00A 00B 00C 02A 02B 03A 03B 03C 04A 04B 04C 10A 10B 10C 11A 11B 11C 12A 12B 12C 13A 13B 13C)
# ===========================================================================
# ===> CROSS TALK correction for detector with LOST or UNSTABLE segments <===
# ===========================================================================
# Before using the script:
#  - Link to data
#  - cd det directory
#  - Put the calibration coeff to zero in the file ecalF1.cal
#  - Run the command: /d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/13A/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -matCCSG
#    to get the deadXsg factor from the matrix CC-SG__50-1500-1500-UI__ma.matr
#
# Usage (execute inside the det directory): ./Xtalk_lost.sh det seg deadXsg deadXcc

DET=$1
SEG=$2
#FactorS in femul
deadXsg=$3
#FactorC in femul, 0 for missing or unstable segments
deadXcc=$4


 	#Build the crosstalk matrix:
	 /d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -matx1

	#Generate the crosstalk matrix coefficients:
	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkMake -f xSG__36-36-100-1536-US__ij.matr
	
        #Generate spetra to check the results:
	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -recalXT xinv_1325-1340.cal | tee coeff_$SEG.log
	
	

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
   
#Get the deadXsg and the deadXcc factors from the matrix CC-SG__50-1500-1500-UI__ma.matr
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 5 0.9483 0.01621305 -matCCSG

#Build the crosstalk matrix:
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 5 0.9483 0.01621305 -matx1

#Generate the crosstalk matrix coefficients:
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkMake -f xSG__36-36-1000-100-US__ij.matr -dxy 1000 100 -gate 55 75 -xcal -15 5 -egam 1332.5 -dval 5 0.780120075

#Generate spetra to check the results:
#	/d2/data/Rosa/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/12B/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg 5 0.9483 0.01621305 -recalXT xinv_0055-0075.cal | tee coeff_5.log









