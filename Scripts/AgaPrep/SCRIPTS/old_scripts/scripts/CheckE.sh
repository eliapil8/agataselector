#Create first the recal_tail.txt files in the directory ../DETS/
#

#Detectors at GANIL 06-04-2016
#DETS=(00A 00B 00C 01A 01B 01C 02A 02B 02C 03A 03B 03C 04A 04B 04C 10A 10B 10C 12A 12B 12C 13A 13B 13C 14A 14B 14C 09B 09C)
#CRYS=(A001 B004 C010 A010 B012 C012 A009 B005 C008 A005 B002 C009 A004 B008 C013 A003 B003 C005 A002 B010 C001 A007 B007 C007 A008 B001 C003 B001 C003)
#CLUS=(atc6 atc6 atc6 atc10 atc10 atc10 atc8 atc8 atc8 atc5 atc5 atc5 atc9 atc9 atc9 atc2 atc2 atc2 atc3 atc3 atc3 atc4 atc4 atc4 atc1 atc1 atc1 adc3 adc3)
#DET=(00 01 02 03 04 10 12 13 14 09)
#CLUSDET=(atc6 atc10 atc8 atc5 atc9 atc2 atc3 atc4 atc1 adc3) 


#DETS=(00A 00B 00C 02A 02B 03A 03B 03C 04A 04B 04C 10A 10B 10C 11A 11B 11C 12A 12B 12C 13A 13B 13C)
#for all detectors--24 02C was not operational 28-06-2015
DETS=(00A 00B 00C 03A 03B 03C 04A 04B 04C 05A 05B 05C 06A 06B 06C 10A 10B 10C 12A 12B 12C 14A 14B 14C)
CRYS=(A001 B004 C010 A009 B005 C008 A005 B002 C009 A008 B001 C003 A003 B003 C005 A006 B013 C006 A002 B010 C001 A007 B007 C007)
CLUS=(atc6 atc6 atc6 atc8 atc8 atc8 atc5 atc5 atc5 atc1 atc1 atc1 atc2 atc2 atc2 atc7 atc7 atc7 atc3 atc3 atc3 atc4 atc4 atc4)
DET=(00 02 03 04 10 11 12 13)
CLUSDET=(atc6 atc8 atc5 atc1 atc2 atc7 atc3 atc4) 


if [[ $1 == "" ]]
    then
         echo -e "\e[1;34mUsage:"
         echo -e "./CheckE.sh run_number_4_digits"
	 echo -e "Example:"
         echo -e "../../scripts/CheckE.sh 0027 \e[0m"
         exit 2
fi



echo "set terminal png" > CheckE.plt
echo "set xlabel \"Segments\"" >> CheckE.plt
echo "set ylabel \" E [keV]\"" >> CheckE.plt
echo "set xrange [-1:38]" >> CheckE.plt

echo "set grid xtics mxtics" >> CheckE.plt  # draw lines for each ytics and mytics
echo "set xtics 0,2,37 " >> CheckE.plt # star,step,end
#echo "set xtics offset 0,-1,0" >> CheckE.plt
echo "set mxtics 1 " >> CheckE.plt          # set the spacing for the mytics
echo "set grid" >> CheckE.plt             # enable the grid
#echo "set yrange [1332.34<*:*]" >> CheckE.plt
#echo "set format y \"%.2e\"" >> CheckE.plt scientific format
#echo "set format y \"%.1f\"" >> CheckE.plt
#echo "set style fill pattern 1">> CheckE.plt
#producing one by one
echo "-------------------------------------"
echo "Producing one crystal by one crystal"
echo "-------------------------------------"
for i in {0..23}; do 
    
    d=${DETS[i]}
    if [ ! -d "$d" ]
    then
    	echo "Directory ${DETS[i]} not existing"
    else
    echo ${DETS[i]}
    echo "set title \"${CLUS[i]} - ${CRYS[i]} - Crystal ${DETS[i]} ($1)  \"" >> CheckE.plt
    echo "set output \"CheckE_${DETS[i]}.png\"" >> CheckE.plt
    echo "plot 1332.5 lt 3 lc rgb \"red\" notitle,\\" >> CheckE.plt
    echo " 1332.65 lt 3 lc rgb \"black\" notitle,\\" >> CheckE.plt
    echo " 1332.35 lt 3 lc rgb \"black\" notitle,\\" >> CheckE.plt
    echo " \"${DETS[i]}/recal_tail.txt\" using 1:5 notitle with points ls 3;" >> CheckE.plt
    
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
#	 echo "set title \"${CLUSDET[i]} - Crystal ${DET[i]}ABC (run_0027) \"" >> CheckE.plt
#	 echo "set output \"Resolution_${DET[i]}.png\"" >> CheckE.plt
#	 echo "plot 2.3 notitle,\\" >> CheckE.plt
#	 echo "\"../${DET[i]}A/recal_tail.txt\" using 1:6 title \"${DET[i]}A - ${CRYS[i*3]}\" with boxes,\\" >> CheckE.plt
#	 echo "\"../${DET[i]}B/recal_tail.txt\" using 1:6 title \"${DET[i]}B - ${CRYS[i*3+1]}\" with boxes,\\" >> CheckE.plt
#	 echo "\"../${DET[i]}C/recal_tail.txt\" using 1:6 title \"${DET[i]}C - ${CRYS[i*3+2]}\" with boxes;" >> CheckE.plt
#    fi
#  
#done
#


echo "Producing the Graphs..."
gnuplot CheckE.plt
echo "Done"
