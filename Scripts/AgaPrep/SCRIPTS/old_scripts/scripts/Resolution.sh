#Create first the recal_tail.txt files in the directory ../DETS/
#



#DETSS=(00A 00B 00C 01A 01B 01C 02A 02B 04A 04B 04C 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C)

DETS=(00A 00B 00C 01A 01B 01C 02A 02B 02C 04A 04B 04C 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 08C 09A 09B 09C 10A 10B 10C 11A 11B 11C 13A 13B 13C)
CRYS=(A006  B005  C001 A011  B006  C012 A016  B017  C013 A004  B004  C010  B001  C006 A008  B009  C014 A014  B010  C016 A002  B007  C007 A017 B018 C018 A013 B015 C011 A010 B011 C009 A018 B012 C019)


CLUS=(atc12 atc12 atc12 atc10 atc10 atc10 atc17 atc17 atc17 atc11 atc11 atc11 atc9 atc9 atc6 atc6 atc6 atc14 atc14 atc14 atc3 atc3 atc3 atc18 atc18 atc18 atc15 atc15 atc15 atc1 atc1 atc1 atc19 atc19 atc19)
# atc20 atc20 atc20
DET=(00 01 02 04 05 06 07 08 09 10 11 13)
CLUSDET=(atc12 atc10 at17 atc11 atc9 atc6 atc14 atc6 atc18 act15 atc1 atc19) 
if [[ $1 == "" ]]
   then
#         echo -e "\e[1;34mUsage:"
         echo -e "./Resolution.sh run_number_4_digits"
#	 echo -e "Example:"
#         echo -e "../../scripts/Resolution.sh 0027 \e[0m"
         exit 2
fi

#for i in "${DETSS[@]}"; do
for i in "${DETS[@]}"; do

    cd ${i}; 
    RecalEnergy -spe ../../Data/$i/Prep__2-40-16384-UI__Ener.spec -sub 0 -num 38 -gain 4 -dwa 20 5  | tee recal.txt
   
    tail -n 38 recal.txt | tee recal_tail.txt

   cd -

   done



echo "set terminal png" > Resolution.plt
echo "set xlabel \"Segment\"" >> Resolution.plt
echo "set ylabel \"FWHM @ 1.3MeV [keV]\"" >> Resolution.plt
echo "set xrange [-1:38]" >> Resolution.plt
echo "set yrange [0:*]" >> Resolution.plt
#echo "set format y \"%.2e\"" >> Resolution.plt scientific format
echo "set format y \"%.1f\"" >> Resolution.plt
#echo "set style fill pattern 1">> Resolution.plt
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
    echo "set title \"${CLUS[i]} - ${CRYS[i]} - Crystal ${DETS[i]} (run $1)  \"" >> Resolution.plt
    echo "set output \"Resolution_${DETS[i]}.png\"" >> Resolution.plt
    echo "plot 2.3 lt 3 lc rgb \"red\" notitle,\\" >> Resolution.plt
    echo " \"${DETS[i]}/recal_tail.txt\" using 1:6 title \"${DETS[i]}\" with boxes;" >> Resolution.plt
    
    fi
done

#producing one cluster by one cluster
echo "-------------------------------------"
echo "Producing one cluster by one cluster"
echo "-------------------------------------"
for i in {0..13}; do
    d1=${DET[i]}A
    d2=${DET[i]}B
    d3=${DET[i]}C
    
    if [ ! -d "$d1" ] && [ ! -d "$d2" ] && [ ! -d "$d3" ]
    then
    	echo "Directories for cluster ${DET[i]} not existing"
    
    else
    	echo ${DET[i]}; 
    	echo "set title \"${CLUSDET[i]} - Crystal ${DET[i]}ABC (run $1) \"" >> Resolution.plt
    	echo "set output \"Resolution_${DET[i]}.png\"" >> Resolution.plt
	echo "plot 2.3 notitle,\\" >> Resolution.plt
    	echo "\"${DET[i]}A/recal_tail.txt\" using 1:6 title \"${DET[i]}A - ${CRYS[i*3]}\" with boxes,\\" >> Resolution.plt
    	echo "\"${DET[i]}B/recal_tail.txt\" using 1:6 title \"${DET[i]}B - ${CRYS[i*3+1]}\" with boxes,\\" >> Resolution.plt
    	echo "\"${DET[i]}C/recal_tail.txt\" using 1:6 title \"${DET[i]}C - ${CRYS[i*3+2]}\" with boxes;" >> Resolution.plt
    fi
  
done



echo "Producing the Graphs..."
gnuplot Resolution.plt
echo "Done"
