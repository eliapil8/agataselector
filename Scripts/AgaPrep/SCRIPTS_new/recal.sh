#!/bin/bash

# Crystals with no neutron damage correction

#DET="00A 00B 00C 01A 01B 01C 02A 02B 02C 06A 06B 07A 07B 09A 09B 09C 10A 10B 10C 11C"

#for i in $DET
#do
#

# 	 RecalEnergy -spe Out_norecal/${i}/Post__5-40-16384-UI__Ener.spec -sub 80 -num 36 -gain 4 -poly1 -152Eu |tee Recal_output/recal_${i}.log

# 	 tail -n 36 Recal_output/recal_${i}.log |tee Recal_output/recal_nohead_${i}.log 
 	 
# 	 awk -F' ' '{printf "segm %2.2s %6.3f %.6f \n",$1,$16,$17}' Recal_output/recal_nohead_${i}.log |tee Conf_recal_seg/${i}/RecalEnergy2.cal
#


 #done

#
 # Crystals with neutron trapping file

DETND="00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C 14A 14B 14C"

for i in $DETND
do
	mkdir ${i}
	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 120 -num 36 -gain 4 -poly1 -226Ra |tee recal_${i}.log

	tail -n 36 recal_${i}.log |tee recal_nohead_${i}.log 
	
	awk -F' ' '{printf "segm %2.2s %6.3f %.6f \n",$1,$16,$17}' recal_nohead_${i}.log |tee ${i}/RecalEnergy2.cal

done

