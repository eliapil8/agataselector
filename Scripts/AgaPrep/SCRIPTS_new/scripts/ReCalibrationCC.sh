#!/bin/bash
#usage: ./Calibration.sh

#Detectors:
#DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)
DETS=(03A 03B 03C 06A 06B 06C 12A  12C)
DETS2=(00A 00B 00C  04A 04B 04C 05A 05B 05C 10A 10B 10C 12B 14A 14B 14C)

for i in "${DETS[@]}"
do 
	echo "=== Energy ReCalibration Crystal ${i} ==="
	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 80 -num 36 -gain 4 -152Eu -dwa 30 7 -poly1 | tee ${i}/recalCC.txt
	
done



for i in "${DETS2[@]}"
do 
	echo "=== Energy ReCalibration Crystal ${i} ==="
	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 120 -num 36 -gain 4 -152Eu -dwa 30 7 -poly1 | tee ${i}/recalCC.txt
	
done

#nedit ???/recal.txt
#pathSoftware=/opt/analysis-tools/analysis-tools/GANIL/AgataSoftware_WithRootTrees/agapro/zUseful/
#DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)
#for i in "${DETS[@]}"; do python $pathSoftware/colupdate.py -o ${i}/RecalEnergy2_newCC.cal -c 2 15 ${i}/RecalEnergy2.cal ${i}/recalCC.txt ; done
#for i in "${DETS[@]}"; do python $pathSoftware/colupdate.py -o ${i}/RecalEnergy2_newCC2.cal -c 3 16 ${i}/RecalEnergy2_newCC.cal ${i}/recalCC.txt ; done
