######PLEASE DO NOT CHANGE the Script, JUST MODIFT THE NAME OF THE FOLDER THAT YOU WANT TO WORK WITH
for i in $currentDets

do

python /opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful/colupdate.py ../../Conf/${i}/PreprocessingFilterPSA.conf shift_${i}_TT.out -c 6 13 -o ../../Conf/${i}/PreprocessingFilterPSA.conf;


done
