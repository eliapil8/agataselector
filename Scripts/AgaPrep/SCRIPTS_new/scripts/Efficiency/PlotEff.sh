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


activity=$1; #in Bq ex:19727.18
lifetime=$2;
#deadtime=expr 1 - $5; #0.532
duration=$3; # in seconds ex: 2537


#prepare the gnuplot file
echo "set terminal png" > plotEffAuto.plt

echo "set xlabel \"Energy (keV)\"" >> plotEffAuto.plt
echo "set ylabel \"Absolute Efficiency \"" >> plotEffAuto.plt

echo "set yrange [0:0.3]" >> plotEffAuto.plt

echo "activity=${activity}" >> plotEffAuto.plt
echo "deadtime=1 - ${lifetime}" >> plotEffAuto.plt
echo "duration=${duration}" >> plotEffAuto.plt


#putting the single textfiles together
for i in "${DETS[@]}"; do
    echo $i;
    
    #gnuplotFile
    echo "set output \"plot${i}_Eff.png\"" >> plotEffAuto.plt
    echo "plot \"list_${i}.txt\" using 1:(\$4/((\$2)/100*activity*duration*(1-deadtime))*100):(sqrt(\$4)/(\$2*activity*duration*(1-deadtime))*100) with yerrorlines title \"${i} CoreE0\";" >> plotEffAuto.plt
    
    echo "set output \"plot${i}_prodEff.png\"" >> plotEffAuto.plt
    echo "plot \"list_prod_${i}.txt\" using 1:(\$4/((\$2)/100*activity*duration*(1-deadtime))*100):(sqrt(\$4)/(\$2*activity*duration*(1-deadtime))*100) with yerrorlines title \"${i} prod CoreE0\";" >> plotEffAuto.plt 
done


#make the global tracked list files


echo ""  >> plotEffAuto.plt
echo "set output \"plotGlobal.png\"" >> plotEffAuto.plt
echo "set yrange [0:9]" >> plotEffAuto.plt
#echo "set autoscale y" >> plotEffAuto.plt
echo "plot \"list_Glob.txt\" using 1:(\$4/((\$2)/100*activity*duration*(1-deadtime))*100):(sqrt(\$4)/(\$2*activity*duration*(1-deadtime))*100) with yerrorlines title \" Sum Cores\";" >> plotEffAuto.plt
  


echo "Producing the Graphs..."
gnuplot plotEffAuto.plt


echo "Done"


