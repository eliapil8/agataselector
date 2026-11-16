#### SCRIPT USED TO UPDATE THE ENERGY CALIBRATION COLUMNS IN PreprocessingFilterPSA.conf ####

for i in $currentDets

do 

python /opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful/colupdate.py ../Conf/${i}/PreprocessingFilterPSA.conf ${i}/cal.txt  -c 4 13 -o ../Conf/${i}/PreprocessingFilterPSA.conf;



done
