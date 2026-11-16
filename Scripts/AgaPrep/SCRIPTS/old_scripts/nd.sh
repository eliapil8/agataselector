#!/bin/bash

RED='\e[1m\e[38;5;196m'
NC='\e[0m'

if [ ! -d Data_nd ] ; then
   echo -e "${RED} Missing the link of the bdat files directory to Data_bdat ${NC}"
   exit 1 
fi


DETS='00A 00B 00C 01A 01C 02A 02B 02C 04A 04B 04C 05A 05B 05C 06A 06B 06C 07A 07B 07C 08A 08B 09A 09B 09C 10A 10B 10C 11A 11B 11C 14A 14B 14C'

for i in ${DETS}  

do

  if [ ! -d ${i} ] ; then mkdir ${i} ; fi

  cd $i

  rm -f Trapping.cal

  for j in {0..35} ; do echo -e "$j\t1.\t1.\t999999.9\t999999.9\t1.\t1." >> Trapping.cal ; done;

  cat Trapping.cal

  SortPsaHits -f ../Data_nd/${i}/Psa__0-16-F__Hits.fdat -best 1300 1350 -bpar 1 10000 0 |tee sort_hit.log

  tail -n 39 sort_hit.log |head -n 36 > sort_hit_nohead.log;

  awk 'FNR==NR{a[NR]=$4;next}{$4=a[FNR]}1' sort_hit_nohead.log Trapping.cal |awk '{if ($4 == 0.0) {printf "%2s %10s %10s 999999.9 %10s %10s %10s \n", $1,$2,$3,$5,$6,$7 } else { printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7} }' > Trapping_tmp.cal;
  awk 'FNR==NR{a[NR]=$5;next}{$5=a[FNR]}1' sort_hit_nohead.log Trapping_tmp.cal |awk '{if ($5 == 0.0) {printf "%2s %10s %10s %10s 999999.9 %10s %10s \n", $1,$2,$3,$4,$6,$7 } else { printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7} }' |tee Trapping.cal;

  cat Trapping.cal

  SortPsaHits -f ../Data_nd/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal

  #RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 0 -gain 5 -offs -5000 -noTR -dwa 30 5 | tail -n 36 |tee log_sg_pre.cal

  #RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 160 -gain 5 -offs -5000 -noTR -dwa 30 5 | tail -n 36 |tee log_cc_pre.cal

  #awk 'FNR==NR{a[NR]=$14;next}{$2=a[FNR]}1' log_sg_pre.cal Trapping.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping_tmp.cal

  #awk 'FNR==NR{a[NR]=$14;next}{$3=a[FNR]}1' log_cc_pre.cal Trapping_tmp.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping.cal

  #SortPsaHits -f ../../Out/$i/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal



  RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 80 -gain 5 -offs -5000 -dwa 30 5 | tail -n 36 |tee log_sg_post.cal

  RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 240 -gain 5 -offs -5000 -dwa 30 5 | tail -n 36 |tee log_cc_post.cal

  awk 'FNR==NR{a[NR]=$14;next}{$6=a[FNR]}1' log_sg_post.cal Trapping.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping_tmp.cal

  awk 'FNR==NR{a[NR]=$14;next}{$7=a[FNR]}1' log_cc_post.cal Trapping_tmp.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping.cal

  SortPsaHits -f ../Data_nd/${i}/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal

  cd ..

done;

