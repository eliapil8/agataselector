/opt/analysis-tools/GANIL/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Data/00A/Prod__4-38-32768-UI__Ampli.spec -sub 38 -num 38 -gain 2
 
 
 DET=(00A 00B 00C 02A 02B 02C 03A 03B 03C 04A 04B 04C 05A 05B 05C 09B 09C 10A 10B 10C 11B 11C 12A 12B 12C 13A 13B 13C 14B 14C)
 source /agata/topology/agatavars
 source /opt/data/GANIL/SourceRuns2016/Replay/agatavars 
 for i in $AGATA_CRYSTALS; do mkdir ${i};done
 
  ---MWD-RISETIME---
 for i in $AGATA_CRYSTALS; do sed -i "s@ 500 @ 250 @g" ${i}/PreprocessingFilterPSA.conf;cat ${i}/PreprocessingFilterPSA.conf; done
 
 ---ENERGY CALIBRATION---
 
for i in $AGATA_CRYSTALS; do /opt/analysis-tools/GANIL/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Data/${i}/Prod__4-38-32768-UI__Ampli.spec -sub 38 -num 38 -gain 2 -dwa 15 2 | tee ${i}/recal.txt;done


 for i in $AGATA_CRYSTALS; do cp  /agatadisks/eagata/sourceruns2016/Replay/Conf/${i}/PreprocessingFilterPSA.conf ${i}/PreprocessingFilterPSA_old.conf ;done
 
 for i in $AGATA_CRYSTALS;do /opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 4 13 ${i}/PreprocessingFilterPSA_old.conf ${i}/recal.txt -o ${i}/PreprocessingFilterPSA.conf; done
 
 
 ---RESOLUTION---
cp macro04Bresol.plt macro04Cresol.plt; sed -i "s@04B@04C@g" macro04Cresol.plt 
for d in 04B 04C 05A 05B 05C 07A 07B 12A 12B 14A 14B 14C;do cp macro04Bresol.plt macro${d}resol.plt; sed -i "s@04B@${d}@g" macro${d}resol.plt; gnuplot macro${d}resol.plt; done
 
 ---TIME ALIGNMENT---
 
 for i in $AGATA_CRYSTALS; do /opt/analysis-tools/GANIL/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Data/${i}/Prep__6-40-1000-UI__TT.spec -sub 40 -num 36 -T 500 | tee ${i}/shift.txt;done
 
 for i in $AGATA_CRYSTALS; do cp  /agatadisks/eagata/sourceruns2016/Replay/Conf/${i}/PreprocessingFilterPSA.conf ${i}/PreprocessingFilterPSA_old.conf ;done
 
for i in $AGATA_CRYSTALS;do /opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 6 13 ${i}/PreprocessingFilterPSA_old.conf ${i}/shift.txt -o ${i}/PreprocessingFilterPSA.conf; done

 ---XTALK---
 
 DETS=(03A 04A 04B 04C 05A 05B 09B 09C 10B 12A 12C 13A)
 
for i in "${DETS[@]}"; do cd ${i};tail -n 38 recal.txt | tee recal_tail.txt;awk '{print "0 " $1 " 2  0 " $14 }' recal_tail.txt | tee ecalF1.cal;cd -; done
 
for i in "${DETS[@]}"; do cd ${i}; /home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/xTalkSort -ifile ../Data/${i}/event_energy.bdat.0000 -ecalF1 ecalF1.cal -egain 5 -specXT -trigewin 1325 1340;  cd -; done

for i in "${DETS[@]}"; do cd ${i}; /home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe xspe__36-37-16384-UI__cal.spec -num 1332 -ener 1332.5 -gain 5 -offs 1000 -Xtalk 37 > xdir_1325-1340.txt ; cd -; done

for i in "${DETS[@]}"; do cd ${i}; grep -v "^#" xdir_1325-1340.txt | grep -v "^ *36 " | cut -b15-102 --complement | tee xdir_1325-1340.cal; cd -; done

for i in "${DETS[@]}"; do cd ${i}; /home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/xTalkInvert -f xdir_1325-1340.cal; cd -; done

for i in "${DETS[@]}"; do cd ${i}; mv xdir_1325-1340.cal.inv xinv_1325-1340.cal; cd -; done

 ---RECALIBRATION---

for i in "${DETS[@]}"; do  /home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Data/${i}/Prep__2-40-16384-UI__Ener.spec -sub 36 -num 2 -gain 2 -152Eu -poly1 | tee ${i}/recal.txt; done

for i in "${DETS[@]}"; do python /home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zUseful/colupdate.py recal1_core.dat ${i}/recal.txt -c 3 16 -o  ${i}/recal1.dat; done
for i in "${DETS[@]}"; do python /home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zUseful/colupdate.py ${i}/recal1.dat ${i}/recal.txt -c 2 15 -o  ${i}/recal1.dat; done
for i in "${DETS[@]}"; do cat recal1_seg.dat ${i}/recal1.dat | tee ${i}/recal1.dat; done


 ---NEUTRON DAMAGE---
 
 DETS=(00A 00B 00C 02C 03B 03C  09B 10A 10B 10C 11B 11C 12A 12B 12C 13A 13B 13C 14B 14C)
 
#Generate Psa__0-16-F__Hits.fdat files adding the keyword WritePsaHits in the gen_conf.py (PSA level) and do a Replay

#Generate lambdaE and lambdaH  coefficients and create the trapping file with the last two columns of coefficientes in best.txt. Pso__7-36-50-100-UI__maxEH.matr will be generated.

for i in "${DETS[@]}"; do cd ${i}; /home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -best 1300 1350 -bpar 1 10000 0 | tee best.txt; cd -;done

for i in "${DETS[@]}"; do cd ${i}; tail -n 39 best.txt | tee best_tail.txt;head -n 36 best_tail.txt | tee best_tail_head.txt;awk '{print $1 "  1.  1.  "  $10 "  "$11 "  1.  1.  " }' best_tail_head.txt | tee Trapping.cal;cd -;done


#Generate Pso__2-4-40-2048-UI__Ener.spec which contains:2*4*40 spectra with the Tapping.cal file:

for i in "${DETS[@]}"; do cd ${i};/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal;cd -;done

[0-SG,1-CC][0-orig,1-orig+recal,2-corr,3-corr+recal][0-39]

0-35 segments
36 Or of segments/cores
37 Or of segments/cores M=1
38 Sum of segments/cores
39 Average SumSegs+SumCC

chan=E*5-5000


#Calibration of SG-orig [0][0] (-num 40 -sub 0) and CC-orig [1][0](-num 40 -sub 160)

for i in "${DETS[@]}"; do cd ${i};/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 0 -offs -5000 -gain 5 -noTR | tee Recal_SG_orig.txt; /opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 2 13 Trapping.cal Recal_SG_orig.txt -o Trapping.cal  ;cd -;done

for i in "${DETS[@]}"; do cd ${i};/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 160 -offs -5000 -gain 5 -noTR | tee Recal_CC_orig.txt;/opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 2 13 Trapping.cal Recal_CC_orig.txt -o Trapping.cal  ;cd -;done


#Generate Pso__2-4-40-2048-UI__Ener.spec with the Tapping.cal which contains the calibration of the two first columns:

for i in "${DETS[@]}"; do cd ${i};/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal;cd -;done


#Calibration of SG-corr [0][0] (-num 40 -sub 80) and CC-corr [1][0](-num 40 -sub 240)

for i in "${DETS[@]}"; do cd ${i};/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 80 -offs -5000 -gain 5 -noTR | tee Recal_SG_corr.txt;/opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 5 13 Trapping.cal Recal_SG_corr.txt -o Trapping.cal ;cd -;done

for i in "${DETS[@]}"; do cd ${i};/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 240 -offs -5000 -gain 5 -noTR | tee Recal_CC_corr.txt;/opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 6 13 Trapping.cal Recal_CC_corr.txt -o Trapping.cal;cd -;done


#Generate Pso__2-4-40-2048-UI__Ener.spec with the Tapping.cal which contains the calibration of the two first columns and the last two columns:

for i in "${DETS[@]}"; do cd ${i};/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/SortPsaHits -f ../Data/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal;cd -;done

#Check [0][3-corr+recal][] and  [1][3-corr+recal][]


 ---NEUTRON DAMAGE Correction example 1 detector---

/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/SortPsaHits -f ../Data/12A/Psa__0-16-F__Hits.fdat -best 1300 1350 -bpar 1 10000 0 | tee best.txt


tail -n 39 best.txt | tee best_tail.txt

head -n 36 best_tail.txt | tee best_tail_head.txt

awk '{print $1 "  1.  1.  "  $10 "  "$11 "  1.  1.  " }' best_tail_head.txt | tee Trapping.cal

/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/SortPsaHits -f ../Data/12A/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal

/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 0 -offs -5000 -gain 5 -noTR | tee Recal_SG.txt

/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 160 -offs -5000 -gain 5 -noTR | tee Recal_CC.txt

 /opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 1 13 Trapping.cal Recal_SG.txt -o Trapping.cal

/opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 2 13 Trapping.cal Recal_CC.txt -o Trapping.cal
/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/SortPsaHits -f ../Data/12A/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal


/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 80 -offs -5000 -gain 5 -noTR | tee Recal_SG_corr.txt

/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 40 -sub 240 -offs -5000 -gain 5 -noTR | tee Recal_CC_corr.txt

/opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 5 13 Trapping.cal Recal_SG_corr.txt -o Trapping.cal


/opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/colupdate.py -c 6 13 Trapping.cal Recal_CC_corr.txt -o Trapping.cal


 ---TKT---
--------------------------------
wine /opt/analysis-tools/GANIL/AgataSoftware/agapro/zUseful/TkT.exe &


for i in "${DETS[@]}"; do cd ${i}; cat ../../../run_0008.dat.05-12-14_12h38m24s/Data/${i}/event_energy.bdat.0000 ../../../run_0009.dat.05-12-14_14h00m06s/Data/${i}/event_energy.bdat.0000 > event_energy.bdat;cd -; done
 
 
 env |grep -i ggp
 
 
[,00,02,03,04,05,09,10,11,12,13,14](ABC)

###Macro TkT###

DM1 (define macro #1)

macro #1 commands:

ss00 1 ss01 1 ss02 1 ss03 1 ss04 1 ss05 1 ss06 1 ss07 1 ss08 1 ss09 1 ss10 1 ss11 1 ss12 1 ss13 1 ss14 1 ss15 1 ss16 1 ss17 1 ss18 1 ss19 ss20 1 ss21 1 ss22 1 ss23

(ssnn->store spectrum in nn, 1-> next spectrum)

M1 (to execute the macro)

--------------------------------


# to create a Topology.conf file per detector

source agatavars 
DIR=(04B 04C 05A 05B 05C 09B 09C)
for i in $AGATA_CRYSTALS
do
	if [ ${i} = 00A ]
	then
		echo "Topology_${i}.conf"
	else 
		cp Topology_00A.conf Topology_${i}.conf
		sed -i "s@Chain 4    00A@Chain 4    ${i}@g" Topology_${i}.conf
		echo "Topology_${i}.conf copied" 
	fi
		
done

for i in "${DIR[@]}"
do 
	sed -i "s@Chain 4    ${i}@Chain 3    ${i}@g" Topology_${i}.conf
	sed -i "s@Filter     PSAFilter@#Filter     PSAFilter@g" Topology_${i}.conf
done



