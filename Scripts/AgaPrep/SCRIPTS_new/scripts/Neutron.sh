#!/bin/bash
#usage: ./Neutron.sh

#Detectors with Neutron Damage:
#DETS='00A 00B 00C 01A 01B 01C 02A 02B 02C 11A 11B 11C'
DETS='00A'
pathSoftware=/opt/analysis-tools/LNL/gammaSoftware/agapro/zUseful
##Generate Psa__0-16-F__Hits.fdat files adding the keyword WritePsaHits in the gen_conf.py (PSA level) and do a Replay



for i in $DETS
do 
	if ! test -d ${i} ; then mkdir ${i}; fi
	cd ${i}
#######First step
##Generate lambdaE and lambdaH  coefficients and create the trapping file with the last two columns of coefficientes in best.txt. Pso__7-36-50-100-UI__maxEH.matr will be generated.
	SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -best 1300 1350 -bpar 1 10000 0 | tee best.txt
	
	tail -n 39 best.txt | tee best_tail.txt;head -n 36 best_tail.txt | tee best_tail_head.txt;awk '{print $1 "  1.  1.  "  $10 "  "$11 "  1.  1.  " }' best_tail_head.txt | tee Trapping.cal
	

##Generate Pso__2-4-40-2048-UI__Ener.spec which contains:2*4*40 spectra with the Tapping.cal file:

	SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal
	

### In  Pso__2-4-40-2048-UI__Ener.spec : [0-SG,1-CC][0-orig,1-orig+recal,2-corr,3-corr+recal][0-39]



#0-35 segments
#36 Or of segments/cores
#37 Or of segments/cores M=1
#38 Sum of segments/cores
#39 Average SumSegs+SumCC

##Check [0][2-corr[] and  [1][2-corr][]

#######Second step
##Calibration of SG-orig [0][0] (-num 40 -sub 0) and CC-orig [1][0](-num 40 -sub 160)

	#RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 0 -offs -5000 -gain 5 -noTR -dwa 25 2 | tee Recal_SG_orig.txt
	#RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 160 -offs -5000 -gain 5 -noTR -dwa 25 2| tee Recal_CC_orig.txt
 
	#python ${pathSoftware}/colupdate.py -c 1 13 Trapping.cal Recal_SG_orig.txt -o Trapping.cal
	#python ${pathSoftware}/colupdate.py -c 2 13 Trapping.cal Recal_CC_orig.txt -o Trapping.cal


##Generate Pso__2-4-40-2048-UI__Ener.spec with the Tapping.cal which contains the calibration of the two first columns:

	#SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal
	
##Check [0][1-orig+recal[] and  [1][1-orig+recal][]

#######Third step
##Calibration of SG-corr [0][0] (-num 40 -sub 80) and CC-corr [1][0](-num 40 -sub 240)

	
	#RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 80 -offs -5000 -gain 5 -noTR -dwa 20 2 | tee Recal_SG_corr.txt
	#RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 240 -offs -5000 -gain 5 -noTR -dwa 20 2 | tee Recal_CC_corr.txt

	#python ${pathSoftware}/colupdate.py -c 5 13 Trapping.cal Recal_SG_corr.txt -o Trapping.cal
	#python ${pathSoftware}/colupdate.py -c 6 13 Trapping.cal Recal_CC_corr.txt -o Trapping.cal

##Generate Pso__2-4-40-2048-UI__Ener.spec with the Tapping.cal which contains the calibration of the two first columns and the last two columns:

	#SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal
	
##Check [0][3-corr+recal][] and  [1][3-corr+recal][]
	cd -

done
grep 1.13 */Trapping.cal
grep 0.000000 */Trapping.cal

