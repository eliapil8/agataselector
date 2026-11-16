
#!/bin/bash


### check the directories exists
if ! test -d Data_bdat    ; then echo -e $FAIL"Link to Data folder not found"$ENDC ; exit 1 ; fi


# ==================================================================================================
# =========> CROSS TALK correction for detector with BROKEN segments <==============================
# ==================================================================================================
#
# It will perform different tasks depending on the number of input 
# parameters (e.g. DET, SEG, deadXsg...)
#
# Usage (execute inside the nDamage directory): ./xtalk_broken.sh DET SEG (deadXsg deadXcc slopeDeadSG)
#
# ==================================================================================================


#Detector and Segment
DET=$1
SEG=$2

#Generate detector directory
	if [ ! -d ${DET} ] ; then   mkdir ${DET} ; cd ${DET} ; fi

#------------------- 2 Input values (DET SEG) ---------------------

if [ $# -eq 2 ]; then

	#Prepare the ecalF1.cal file from PreprocessingFilterPSA.conf
		for j in {1..38}
		do
        		cat ../Conf/$DET/PreprocessingFilterPSA.conf | head -n $j |awk -v var=$(( $j - 1 )) '{print "0\t" var "\t2\t0\t" $5}' | tail -n 1 >> ecalF1.cal
		done
	
		cat ecalF1.cal

	#Generate CC-SG__50-1500-1500-UI__ma.matr to get the deadXsg and deadXcc factors
		xTalkSort -ifile Data_bdat/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -matCCSG

fi


#------------- 4 Input values (DET SEG deadXsg deadXcc) -----------

if [ $# -eq 4 ]; then

	#FactorS in femul
	deadXsg=$3
	#FactorC in femul, 0 for missing or unstable segments
	deadXcc=$4

	#Generate matrix to check the deadXsg and deadXcc factors and get the slopeDeadSG factor
		xTalkSort -ifile Data_bdat/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -matCCSG

	#Build the crosstalk matrix:
	        xTalkSort -ifile Data_bdat/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -matx1

fi


#-------- 5 Input values (DET SEG deadXsg deadXcc slopeDeadSG) ----

if [ $# -eq 5 ]; then

	#FactorS in femul
	deadXsg=$3
	#FactorC in femul, 0 for missing or unstable segments
	deadXcc=$4
	#SlopeDeadSG in femul
	slopeDeadSG=$5


	#Isolate the crosstalk matrix coefficients:
	        xTalkMake -f xSG__36-36-1000-100-US__ij.matr -dxy 1000 100 -gate 55 75 -xcal -15 5 -egam 1332.5 -dval $SEG $slopeDeadSG

	#Rename the matrices to be consistent with the Xtalk file names:
	        mv xdir_0055-0075.cal xdir_1325-1340.cal; mv xinv_0055-0075.cal xinv_1325-1340.cal

	#Generate spetra to check the results:
	        xTalkSort -ifile Data_bdat/$DET/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -deadSeg $SEG $deadXsg $deadXcc -recalXT xinv_1325-1340.cal | tee coeff_$SEG.log

fi




# Text message for copying the xtalk files
echo -e $HEADER"\n You can now copy the xtalk files with: "$ENDC
echo -e $HEADER"cp xinv_1325-1340.cal ../Conf/DET/.; cp xdir_1325-1340.cal ../Conf/DET/. "$ENDC



#Files generated:
# xinv_1325-1340.cal
#proj__3-38-3768-UI__raw.spec    raw projections:      [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__cal.spec    calibrated spectra:   [0]=Tot [1]=SG@F1 [2]=CC@F1
#spec__3-38-16384-UI__adj.spec    adjusted spectra:     [0]=Tot [1]=SG@F1 [2]=CC@F1

