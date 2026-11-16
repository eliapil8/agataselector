#!/bin/bash                                                                                                                   


#### SCRIPT USED TO PERFORM GLOBAL TIME ALIGNMENT ####

N=39; NN=$(( $N*$N ));

RecalEnergy -spe ../Out/Merger/Track__${N}-${N}-1000-UI__TT.spec -T 500 -num ${NN} |tee recalT.dat;

tail -n ${NN} recalT.dat |tee recalT_nohead.dat;

python /opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful/solveTT.py -f recalT_nohead.dat -n ${N} -c 13 -p 500 |tee solveTT_tmp.dat

tail -n $(( $N+3 )) solveTT_tmp.dat |tee solveTT.dat |head -n ${N} |tee solveTT.dat;

awk -F' ' '{printf "\"TimeShiftCC %7.3f \"\n",$1}' solveTT.dat | tee gta_output.dat
