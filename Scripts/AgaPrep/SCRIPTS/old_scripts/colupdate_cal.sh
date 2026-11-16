######PLEASE DO NOT CHANGE the Script, JUST MODIFT THE NAME OF THE FOLDER THAT YOU WANT TO WORK WITH


DETS='00A 00B 00C 01A 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 09A 09B 09C 10A 10B 10C 11A 11B 11C 14A 14B 14C'

for i in $DETS
do 

python /opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful/colupdate.py ../Conf/${i}/PreprocessingFilterPSA.conf ${i}/cal.txt  -c 4 13 -o ../Conf/${i}/PreprocessingFilterPSA.conf;



done
