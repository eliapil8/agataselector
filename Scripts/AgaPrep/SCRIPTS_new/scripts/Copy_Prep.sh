#!/bin/bash
#usage: ./Copy_Prep.sh <option>
#<option>: time, energy,mix

#Detectors:
DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)
#DETS=(12A)
pathConf=.
pathSoftware=/opt/analysis-tools/analysis-tools/GANIL/AgataSoftware_WithRootTrees/agapro/zUseful/
for i in "${DETS[@]}"
do 

	

	if [ $1 = "energy" ]	
	then
		echo "=== Crystal ${i} ==="
		echo
		echo "Copying PreprocessingFilterPSA.conf crystal ${i}... to ...PreprocessingFilterPSA_oldEnergy.conf"
		
		cp -v $pathConf/Conf/${i}/PreprocessingFilterPSA.conf ${i}/PreprocessingFilterPSA_oldEnergy.conf
		
		
		echo "Updating Energy calibration coefficients in PreprocessingFilterPSA.conf crystal ${i}..."
		python $pathSoftware/colupdate.py -o ${i}/PreprocessingFilterPSA_Energy.conf -c 4 13 ${i}/PreprocessingFilterPSA_oldEnergy.conf ${i}/recal.txt 
		#/d2/data/Rosa/AgataSoftware/agapro/zUseful/colupdate.py -c 4 13 ${i}/PreprocessingFilterPSA_oldEnergy.conf ${i}/recal.txt -o ${i}/PreprocessingFilterPSA_Energy.conf
		
	else if [ $1 = "time" ]
	then
		echo "=== Crystal ${i} ==="
		echo
		echo "Copying PreprocessingFilterPSA.conf crystal ${i}... to ...PreprocessingFilterPSA_oldTime.conf "
		cp -v $pathConf/Conf/${i}/PreprocessingFilterPSA.conf ${i}/PreprocessingFilterPSA_oldTime.conf 
		
		echo "Updating Time alignment coefficients in PreprocessingFilterPSA.conf crystal ${i}..."
		python $pathSoftware/colupdate.py -o ${i}/PreprocessingFilterPSA_Time.conf -c 6 13 ${i}/PreprocessingFilterPSA_oldTime.conf ${i}/shift.txt
		#/d2/data/Rosa/AgataSoftware/agapro/zUseful/colupdate.py -c 6 13 ${i}/PreprocessingFilterPSA_oldTime.conf ${i}/shift.txt -o ${i}/PreprocessingFilterPSA_Time.conf
	
	else if [ $1 = "mix" ]
	then
		echo "=== Crystal ${i} ==="
		echo
		cp -v ${i}/PreprocessingFilterPSA_Energy.conf ${i}/PreprocessingFilterPSA.conf 
		python $pathSoftware/colupdate.py -o ${i}/PreprocessingFilterPSA.conf -c 6 6 ${i}/PreprocessingFilterPSA.conf ${i}/PreprocessingFilterPSA_Time.conf

	fi
	fi
	fi

	echo
	


done
