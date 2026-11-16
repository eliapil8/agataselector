#!/bin/bash
# automatic Efficiency plotter for AGATA data.
# It calls the RecalEnergy program of Dino Bazzacco and extracts the corresponding areas
# Usage plotEff.sh InputLocal InputGlobal SourceType Activity Lifetime Duration
#  The InputLocal and InputGlobal have to point to the root directory
#  of the replayed data, i.e. the folder of the InputLocal has to contain subfolders
#  with the crystals like 00A etc.
#  The InputGlobal has to have a subfolder called Global with the femul spectra
#  
# Sourcetype should be 152Eu, 133Ba, 60Co or 137Cs
# 
# Activity is the activity at the time of the calibration in Bq
#
#  Lifetime is 1-deadtime
# Duration  is the real length of the run in seconds
# 
# Created by P.R.John, R.Perez Vidal and D.Testov 2015
# Questions, suggestions and improvements are welcome
# to be communicated to Philipp: philipp.john@pd.infn.it



#Copyright (c) 2015 P.R.John, R.Perez Vidal and D.Testov
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.


#source /agata/topology/agatavars


DETS=(00A 00B 00C 02A 02B 02C 03A 03B 03C 04A 04B 04C 05A 05B 05C 09B 09C 10A 10B 10C 11B 11C 12A 12B 12C 13A 13B 13C 14B 14C)

RecalProg="/home/prototype/GANIL_ONLINE/Workspace/AgataSoftware/agapro/zPrograms/RecalEnergy"

#source activity

sourceType=$1;

#run=$2;

localDir=$2;
globalDir=$3;

#for Eu source, we want to see the X-rays in the efficiency curve
# Hence modify the parameters for the call of 152Eu, -kill 411.111
if [ "$sourceType" = "152Eu" ];
then 
  sourceLine="-152Eu  -kill 1085.793 -ener 40  -v 2 ";
else 
  sourceLine="-${sourceType}-dwa 2 7 -v 2 ";
fi


if [ "$sourceType" = "152Eu" ];
then 
  #select the columns of the files 
  echo "Energy       Intensity">  ref.txt
  echo "40 	         59.4"  >> ref.txt
  #echo "45 	13.37" >> ref.txt
  echo "121.7824 	28.37" >> ref.txt
  echo "244.6989 	7.53 " >> ref.txt
  echo "344.2811 	26.57" >> ref.txt
  echo "411.126 	2.238" >> ref.txt
  echo "443.965 	3.125" >> ref.txt
  echo "778.903 	12.97" >> ref.txt
  echo "964.055 	14.63" >> ref.txt
  echo "1112.087 	13.54" >> ref.txt
  echo "1408.022 	20.85" >> ref.txt
fi

if [ "$sourceType" = "133Ba" ];
then 
  echo "Energy       Intensity">  ref.txt
  echo "53.161    21.99" >> ref.txt
  echo "80.8      36.68" >> ref.txt
  echo "160.631   0.645" >> ref.txt
  echo "223.234   0.450" >> ref.txt
  echo "276.398   7.164" >> ref.txt
  echo "302.853   18.33" >> ref.txt
  echo "356.017   62.05" >> ref.txt
  echo "383.851   8.94"  >> ref.txt
fi

if [ "$sourceType" = "60Co" ];
then 
  echo "Energy       Intensity">  ref.txt
  echo "1173.238    99.9736" >> ref.txt
  echo "1332.513    99.9856" >> ref.txt
fi

if [ "$sourceType" = "137Cs" ];
then 
  echo "Energy       Intensity">  ref.txt
  echo "661.657    85.1" >> ref.txt
fi


echo "Fit spectra with ${sourceType} using the energies"
cat ref.txt
echo "Fitting the spectra ..."





for i in {0..29}; do ${RecalProg} -spe ${globalDir}/Merger/Track__2-30-16384-UI__EC.spec -sub ${i} -num 1 ${sourceLine}  -dwa 2 7  > ${DETS[i]}_CoreE0_recal.txt ; 
done
for i in "${DETS[@]}"; do ${RecalProg} -spe ${localDir}/${i}/Prod__4-38-32768-UI__Ampli.spec -sub 36 -num 1 ${sourceLine} -dwa 15 10  > ${i}_prod_recal.txt ; 
done

#fitting the tracked spectrum
${RecalProg}  -spe ${globalDir}/Merger/Track__2-15-16384-UI__EE.spec -sub 0 -num 1 ${sourceLine}  -dwa 2 7 > Glob_Cores_recal.txt
#${RecalProg}  -spe ${globalDir}/Global/Track__2-15-16384-UI__EE.spec -sub 2 -num 1 ${sourceLine}   > Glob_Tracked_recal.txt
#${RecalProg}  -spe ${globalDir}/Global/Track__2-15-16384-UI__EE.spec -sub 8 -num 1 ${sourceLine}   > Glob_Calorimetric_recal.txt



#putting the single textfiles together
for i in "${DETS[@]}"; do
    echo $i;
    
    echo "CoreE0 Counts" >  ${i}_CoreE0_Counts.txt
    echo "CoreE0 Counts" >  ${i}_prod_Counts.txt
    
    cat ${i}_CoreE0_recal.txt | grep "#2     0    " | awk '{print $9,$5}' >> ${i}_CoreE0_Counts.txt
    cat ${i}_prod_recal.txt | grep "#2     0    " | awk '{print $9,$5}' >> ${i}_prod_Counts.txt
    
    paste ref.txt `ls ${i}_CoreE0_Counts.txt` > list_${i}.txt;
    paste ref.txt `ls ${i}_prod_Counts.txt` > list_prod_${i}.txt;
         
done




#make the global tracked list files
echo "Global";
echo "Cores Counts" > Cores_Counts.txt;

cat Glob_Cores_recal.txt  | grep "#2     0    " | awk '{print $9,$5}' >> Cores_Counts.txt;

paste ref.txt Cores_Counts.txt  > list_Glob.txt;


#cleaning up
echo "Cleaning up..."

rm -f ref.txt
rm -f *_Counts.txt
rm -f *_recal.txt


echo "Done"


