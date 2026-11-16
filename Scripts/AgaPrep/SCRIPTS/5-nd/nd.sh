### check the directories exists
if ! test -d Data_nd    ; then echo -e $FAIL"Data folder not found (need to run in the ndamage folder)"$ENDC ; exit 1 ; fi

## check detectors are defined
if [[ -z $currentDets ]] ; then echo -e $FAIL"currentDets variable empty, need to export currentDets=..."$ENDC ; exit 1; fi



for i in $currentDets ; do 


    ## 1. CREATE WORKING DIRECTORIES FOR THE CRYSTALS
    if [ ! -d "$i" ]; then
        mkdir "$i"
        echo "Directory $i created."
    else
        echo "Directory $i already exists."
    fi

    cd $i

    ## 2. REMOVES OLD FILE
    rm -f Trapping.cal

    ## 3. CREATES A DUMMY TRAPPING FILE
    for j in {0..35} ; do echo -e "$j\t1.\t1.\t999999.9\t999999.9\t1.\t1." >> Trapping.cal ; done;
        
    cat Trapping.cal

    SortPsaHits -f ../Data_nd/$i/Psa__0-16-F__Hits.fdat -best 1300 1350 -bpar 1 10000 0 |tee sort_hit.log

    tail -n 39 sort_hit.log |head -n 36 > sort_hit_nohead.log;

    ## 4. UPDATES DUMMY TRAPPING FILE
    awk 'FNR==NR{a[NR]=$4;next}{$4=a[FNR]}1' sort_hit_nohead.log Trapping.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' > Trapping_tmp.cal;
    awk 'FNR==NR{a[NR]=$5;next}{$5=a[FNR]}1' sort_hit_nohead.log Trapping_tmp.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping.cal;

    ## 5. GENERATES "Pso__2-4-40-2048-UI__Ener.spec" 
    SortPsaHits -f ../Data_nd/$i/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal



    ##-------------------------------------------------------------------------------------------------------------------------
    ##       UNCOMMENT THIS BLOCK TO USE THE CALIBRATION BEFORE THE CORRECTION 
    ##-------------------------------------------------------------------------------------------------------------------------

    ## 6. IF NECESSARY RECALIBRATION OF THE SEGMENTS & CORES BEFORE!! NEUTRON DAMAGE CORRECTION

    ## recalibration for the segments:
    #RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 0 -gain 5 -offs -5000 -noTR -dwa 30 5 | tail -n 36 |tee log_sg_pre.cal

    ## recalibration for the cores:
    #RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 160 -gain 5 -offs -5000 -noTR -dwa 30 5 | tail -n 36 |tee log_cc_pre.cal

    ## 7. INSERT THE PRE-TRAPPING PARAMETERS IN THE TRAPPING FILE

    ## segments:
    #awk 'FNR==NR{a[NR]=$14;next}{$2=a[FNR]}1' log_sg_pre.cal Trapping.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping_tmp.cal

    ## cores:
    #awk 'FNR==NR{a[NR]=$14;next}{$3=a[FNR]}1' log_cc_pre.cal Trapping_tmp.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping.cal

    ## 8. APPLY (WHEN USED) THE CALIBRATION BEFORE!! TRAPPING, IT REGENERATES THE "Pso__2-4-40-2048-UI__Ener.spec"
    #SortPsaHits -f ../Data_nd/$i/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal



    ##-------------------------------------------------------------------------------------------------------------------------
    ##       UNCOMMENT THIS BLOCK TO USE THE CALIBRATION AFTER THE CORRECTION 
    ##-------------------------------------------------------------------------------------------------------------------------

    ## RECALIBRATION OF THE SEGMENTS & CORES AFTER!! NEUTRON DAMAGE CORRECTION

    ## segments (family 0, library 3) uses family 0, library 2 (-sub 80) to calculate the calibration coefficients:
    #RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 80 -gain 5 -offs -5000 -dwa 30 5 | tail -n 36 |tee log_sg_post.cal

    ## cores (family 1, library 3)  uses family 1, library 2 (-sub 240) to calculate the calibration coefficients:
    #RecalEnergy -spe Pso__2-4-40-2048-UI__Ener.spec -num 36 -sub 240 -gain 5 -offs -5000 -dwa 30 5 | tail -n 36 |tee log_cc_post.cal

    ## 9. UPDATE THE CALIBRATION COEFFICIENTS AFTER!! NEUTRON DAMAGE CORRECTION ON THE TRAPPING FILE: 
    #awk 'FNR==NR{a[NR]=$14;next}{$6=a[FNR]}1' log_sg_post.cal Trapping.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping_tmp.cal

    #awk 'FNR==NR{a[NR]=$14;next}{$7=a[FNR]}1' log_cc_post.cal Trapping_tmp.cal |awk '{printf "%2s %10s %10s %10s %10s %10s %10s \n", $1,$2,$3,$4,$5,$6,$7}' |tee Trapping.cal

    ## 10. REGENERATE THE "Pso__2-4-40-2048-UI__Ener.spec" with both calibrations (before & after neutron damage correction)
    ## CAREFUL!! A GAIN OF 5 AND AN OFFSET OF 5000 IS APPLIED, THE 1333 PEAK WILL THUS BE LOCATED AT CHANNEL 1665

    #SortPsaHits -f ../Data_nd/$i/Psa__0-16-F__Hits.fdat -gain 5 -offs 5000 -fcal Trapping.cal

    cd ..   

done;



## Spectra generated in Pso file
## Family 0: Segments
## – Library 0: original spectra
## – Library 1: original spectra + recal before neutron damage correction
## – Library 2: spectra after neutron damage correction
## – Library 3: neutron correction + final recalibration
## • Family 1: Core
## – Library 0: ...
