#!/bin/bash                                                                                                                   


#### SCRIPT USED TO PERFORM GLOBAL TIME ALIGNMENT ####

# N is number of detectors
	N=39; NN=$(( $N*$N ));

# fit the time spectra
	RecalEnergy -spe ../Out/Merger/Track__${N}-${N}-1000-UI__TT.spec -T 500 -num ${NN} |tee recalT.dat;

	tail -n ${NN} recalT.dat |tee recalT_nohead.dat;


# apply the solveTT.py script
	python /opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful/solveTT.py -f recalT_nohead.dat -n ${N} -c 13 -p 500 |tee solveTT_tmp.dat


# extract the time shift values
	tail -n $(( $N+3 )) solveTT_tmp.dat |tee solveTT.dat |head -n ${N} |tee solveTT.dat;

	awk -F' ' '{printf "\"TimeShiftCC %7.3f \"\n",$1}' solveTT.dat | tee gta_output.dat


echo -e $HEADER"\n Finally you can check that the spectra in Out/Global/Track_$N-$N-1000-UI__TT.spec (in grid mode) are all aligned to 500 "$ENDC 
