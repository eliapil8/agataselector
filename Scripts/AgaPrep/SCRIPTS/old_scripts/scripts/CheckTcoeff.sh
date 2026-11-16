#Create first the recal_tail.txt files in the directory ../DETS/
#

#Detectors at GANIL 06-04-2016
#DETS=(00A 00B 00C 01A 01B 01C 02A 02B 02C 03A 03B 03C 04A 04B 04C 10A 10B 10C 12A 12B 12C 13A 13B 13C 14A 14B 14C 09B 09C)
#CRYS=(A001 B004 C010 A010 B012 C012 A009 B005 C008 A005 B002 C009 A004 B008 C013 A003 B003 C005 A002 B010 C001 A007 B007 C007 A008 B001 C003 B001 C003)
#CLUS=(atc6 atc6 atc6 atc10 atc10 atc10 atc8 atc8 atc8 atc5 atc5 atc5 atc9 atc9 atc9 atc2 atc2 atc2 atc3 atc3 atc3 atc4 atc4 atc4 atc1 atc1 atc1 adc3 adc3)
#DET=(00 01 02 03 04 10 12 13 14 09)
#CLUSDET=(atc6 atc10 atc8 atc5 atc9 atc2 atc3 atc4 atc1 adc3) 


#DETSS=(00A 00B 00C 02A 02B 03A 03B 03C 04A 04B 04C 10A 10B 10C 11A 11B 11C 12A 12B 12C 13A 13B 13C)
#for all detectors--24 02C was not operational 28-06-2015
DETSS=(00A 00B 00C 02A 02B 03A 03B 03C 04A 04B 04C 10A 10B 10C 11A 11B 11C 12A 12B 12C 13A 13B 13C)
DETS=(00A 00B 00C 02A 02B 02C 03A 03B 03C 04A 04B 04C 10A 10B 10C 11A 11B 11C 12A 12B 12C 13A 13B 13C)
CRYS=(A001 B004 C010 A009 B005 C008 A005 B002 C009 A008 B001 C003 A003 B003 C005 A006 B013 C006 A002 B010 C001 A007 B007 C007)
CLUS=(atc6 atc6 atc6 atc8 atc8 atc8 atc5 atc5 atc5 atc1 atc1 atc1 atc2 atc2 atc2 atc7 atc7 atc7 atc3 atc3 atc3 atc4 atc4 atc4)
DET=(00 02 03 04 10 11 12 13)
CLUSDET=(atc6 atc8 atc5 atc1 atc2 atc7 atc3 atc4) 

if [[ $1 == "" ]]
    then
         echo -e "\e[1;34mUsage:"
         echo -e "./CheckTcoeff.sh run_number_4_digits"
	 echo -e "Example:"
         echo -e "../../scripts/CheckTcoeff.sh 0027 \e[0m"
         exit 2
fi


for i in "${DETSS[@]}"; do

    cd ../${i}; 
   
    tail -n 36 shift.txt | tee shift_tail.txt
    awk '{print $1 " 1. " $14 }' shift_tail.txt | tee checkTcoeff.txt 

   /d2/data/Rosa/AgataSoftware/agapro/zUseful/colupdate.py -o checkTcoeff.txt -c 1,6 checkTcoeff.txt PreprocessingFilterPSA_oldTime.conf

   cd -

   done
echo "set terminal png" > CheckTcoeff.plt
echo "set xlabel \"Segments\"" >> CheckTcoeff.plt
echo "set ylabel \" T shift coeff \"" >> CheckTcoeff.plt
echo "set xrange [-1:38]" >> CheckTcoeff.plt

echo "set grid xtics mxtics" >> CheckTcoeff.plt  # draw lines for each ytics and mytics
echo "set xtics 0,2,37 " >> CheckTcoeff.plt # star,step,end
#echo "set xtics offset 0,-1,0" >> CheckTcoeff.plt
echo "set mxtics 1 " >> CheckTcoeff.plt          # set the spacing for the mytics
echo "set grid" >> CheckTcoeff.plt             # enable the grid
#echo "set yrange [0.999999:1.000001]" >> CheckTcoeff.plt
#echo "set format y \"%.2e\"" >> CheckTcoeff.plt scientific format
echo "set format y \"%.6f\"" >> CheckTcoeff.plt
#echo "set style fill pattern 1">> CheckTcoeff.plt
#producing one by one
echo "-------------------------------------"
echo "Producing one crystal by one crystal"
echo "-------------------------------------"




for i in {0..23}; do 
    
    d=../${DETS[i]}
    if [ ! -d "$d" ]
    then
    	echo "Directory ${DETS[i]} not existing"
    else
    echo ${DETS[i]}
    echo "set title \"${CLUS[i]} - ${CRYS[i]} - Crystal ${DETS[i]} ($1)  \"" >> CheckTcoeff.plt
    echo "set output \"CheckTcoeff_${DETS[i]}.png\"" >> CheckTcoeff.plt
    echo "plot 1 lt 3 lc rgb \"red\" notitle,\\" >> CheckTcoeff.plt
   # echo " 0.999 lt 3 lc rgb \"black\" notitle,\\" >> CheckTcoeff.plt
   # echo " 1.001 lt 3 lc rgb \"black\" notitle,\\" >> CheckTcoeff.plt
    echo " \"../${DETS[i]}/checkTcoeff.txt\" using 1:(\$2/\$3) notitle with points ls 3;" >> CheckTcoeff.plt
    
    fi
done

#producing one cluster by one cluster
#echo "-------------------------------------"
#echo "Producing one cluster by one cluster"
#echo "-------------------------------------"
#for i in {0..7}; do
#    d1=../${DET[i]}A
#    d2=../${DET[i]}B
#    d3=../${DET[i]}C
#    
#    if [ ! -d "$d1" ] && [ ! -d "$d2" ] && [ ! -d "$d3" ]
#    then
#	 echo "Directories for cluster ${DET[i]} not existing"
#    
#    else
#	 echo ${DET[i]}; 
#	 echo "set title \"${CLUSDET[i]} - Crystal ${DET[i]}ABC (run_0027) \"" >> CheckTcoeff.plt
#	 echo "set output \"Resolution_${DET[i]}.png\"" >> CheckTcoeff.plt
#	 echo "plot 2.3 notitle,\\" >> CheckTcoeff.plt
#	 echo "\"../${DET[i]}A/recal_tail.txt\" using 1:6 title \"${DET[i]}A - ${CRYS[i*3]}\" with boxes,\\" >> CheckTcoeff.plt
#	 echo "\"../${DET[i]}B/recal_tail.txt\" using 1:6 title \"${DET[i]}B - ${CRYS[i*3+1]}\" with boxes,\\" >> CheckTcoeff.plt
#	 echo "\"../${DET[i]}C/recal_tail.txt\" using 1:6 title \"${DET[i]}C - ${CRYS[i*3+2]}\" with boxes;" >> CheckTcoeff.plt
#    fi
#  
#done
#


echo "Producing the Graphs..."
gnuplot CheckTcoeff.plt
echo "Done"
