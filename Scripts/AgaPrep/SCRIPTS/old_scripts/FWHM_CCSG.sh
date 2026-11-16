#!/bin/bash
RED='\e[1m\e[38;5;196m'
NC='\e[0m'

# Crystals with trapping file

DETND="00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C 14A 14B 14C"
#DETND="14A"



 if [ ! -d Data ] ; then
           
           echo -e "${RED} Missing the link of the postpsa files directory to Data ${NC}"
           exit 1
        fi 


for i in $DETND
do
        if [ ! -d ${i} ] ; then   mkdir ${i} ; fi 
	
	#Core before neutron damage correction

	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 119 -num 1 -gain 4 | tee ${i}/fwhmCore.log
	
	tail -n 1 ${i}/fwhmCore.log |tee ${i}/fwhmCore_nohead.log
	echo -n ${i} | tee ${i}/FWHMCore_final.log
	awk -F' ' '{printf "  \"FWHM %6.3f \"\n",$6}' ${i}/fwhmCore_nohead.log  | tee -a ${i}/FWHMCore_final.log


        #SumSegments before neutron damage correction
	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 38 -num 1 -gain 4 -dwa 30 2  | tee ${i}/fwhmSSeg.log
	
	tail -n 1 ${i}/fwhmSSeg.log |tee ${i}/fwhmSSeg_nohead.log
	echo -n ${i} | tee ${i}/FWHMSSeg_final.log
	awk -F' ' '{printf "  \"FWHM %6.3f \"\n",$6}' ${i}/fwhmSSeg_nohead.log  | tee -a ${i}/FWHMSSeg_final.log
	
	
	#-------------------------------
	
	#Core after neutron damage correction

	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 159 -num 1 -gain 4 | tee ${i}/fwhmCoreNDC.log
	
	tail -n 1 ${i}/fwhmCoreNDC.log |tee ${i}/fwhmCoreNDC_nohead.log
	echo -n ${i} | tee ${i}/FWHMCoreNDC_final.log
	awk -F' ' '{printf "  \"FWHM %6.3f \"\n",$6}' ${i}/fwhmCoreNDC_nohead.log  | tee -a ${i}/FWHMCoreNDC_final.log


        #SumSegments after neutron damage correction
	RecalEnergy -spe Data/${i}/Post__5-40-16384-UI__Ener.spec -sub 78 -num 1 -gain 4 -dwa 30 2   | tee ${i}/fwhmSSegNDC.log
	
	tail -n 1 ${i}/fwhmSSegNDC.log |tee ${i}/fwhmSSegNDC_nohead.log
	echo -n ${i} | tee ${i}/FWHMSSegNDC_final.log
	awk -F' ' '{printf "  \"FWHM %6.3f \"\n",$6}' ${i}/fwhmSSegNDC_nohead.log  | tee -a ${i}/FWHMSSegNDC_final.log

done
echo "===Core before ND correction==="

cat ???/FWHMCore_final.log | tee FWHMCore_final.log

echo "===SumSeg before ND correction==="

cat ???/FWHMSSeg_final.log | tee FWHMSSeg_final.log

echo "===Core after ND  correction==="

cat ???/FWHMCoreNDC_final.log | tee FWHMCoreNDC_final.log


echo "===SumSeg after ND correction==="

cat ???/FWHMSSegNDC_final.log | tee FWHMSSegNDC_final.log
