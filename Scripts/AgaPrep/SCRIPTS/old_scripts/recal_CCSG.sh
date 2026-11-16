#!/bin/bash

# Crystals without trapping file

#DET="00A 00B 00C 01A 01B 01C 02A 02B 02C 06A 06B 07A 07B 09A 09B 09C 10A 10B 10C 11C"


#for i in $DET
#do



	#RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 119 -num 1 -gain 4 -poly1 -226Ra | tee recalCore_${i}.log
	
	#tail -n 1 recalCore_${i}.log |tee Recal_output/recalCore_nohead_${i}.log

	#awk -F' ' '{printf "\"RecalCC %6.3f %.6f\"\n",$16,$17}' recalCore_nohead_${i}.log  |tee Recal_output/RecalCore_final_${i}.log


#done


# Crystals with trapping file

DETND="00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C 14A 14B 14C"

#DETND="14A"

for i in $DETND
do
        mkdir ${i}

	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 159 -num 1 -gain 4 -poly1 -226Ra | tee ${i}/recalCore.log
	
	tail -n 1 ${i}/recalCore.log |tee ${i}/recalCore_nohead.log

	awk -F' ' -v j=${i} 'BEGIN{printf "%s \t,j"} {printf "\"RecalCC %6.3f %.6f\"\n",$16,$17}' ${i}/recalCore_nohead.log  | tee ${i}/RecalCore_final.log
	echo -n ${i} | tee ${i}/RecalCore_final.log
	awk -F' ' '{printf "  \"RecalCC %6.3f %.6f\"\n",$16,$17}' ${i}/recalCore_nohead.log  | tee -a ${i}/RecalCore_final.log

done

cat ???/RecalCore_final.log | tee RecalCore_final.log


for i in $DETND
do
        mkdir ${i}

	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 78 -num 1 -gain 4 -poly1 -226Ra | tee ${i}/recalSSeg.log
	
	tail -n 1 ${i}/recalSSeg.log |tee ${i}/recalSSeg_nohead.log

	awk -F' ' -v j=${i} 'BEGIN{printf "%s \t,j"} {printf "\"RecalCC %6.3f %.6f\"\n",$16,$17}' ${i}/recalSSeg_nohead.log  | tee ${i}/RecalSSeg_final.log
	echo -n ${i} | tee ${i}/RecalSSeg_final.log
	awk -F' ' '{printf "  \"RecalSG %6.3f %.6f\"\n",$16,$17}' ${i}/recalSSeg_nohead.log  | tee -a ${i}/RecalSSeg_final.log

done

cat ???/RecalSSeg_final.log | tee RecalSSeg_final.log
