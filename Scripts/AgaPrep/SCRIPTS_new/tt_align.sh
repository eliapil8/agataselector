#!/bin/bash
#for i in 00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C 14A 14B 14C
for i in 00A 00B 00C 01A 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 09A 09B 09C 10A 10B 10C 11A 11B 11C 14A 14B 14C
do

#RecalEnergy -spe ../Out/$i/Prep__6-40-1000-UI__TT.spec -sub 40 -num 36 -T 500 | tee shift_${i}_TT.out

python /opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful/colupdate.py ../Conf/${i}/PreprocessingFilterPSA.conf shift_${i}_TT.out  -c 6 13 -o ../Conf/${i}/PreprocessingFilterPSA.conf

done
