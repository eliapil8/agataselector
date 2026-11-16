# AGATA preprocessing scripts
This directory contains various folders with the scripts used for the AGATA preprocessing (crystal by crystal):

## ```1-en_cal```

- "en_cal.sh" used to obtain the energy calibration parameters of segments and cores
- "check_cal.sh" to check if the calibration was done properly
- "colupdate_cal.sh" to update the Conf/PreprocessingFilterPSA.conf files with the new energy calibration parameters 


## ```2-xtalk```

- "xtalk.sh" used to obtain the x-talk parameters if all segments are working fine
- "xtalk_broken.sh" used to obtain the x-talk parameters if a segment is broken
- "xtalk_lost.sh" used to obtain the x-talk parameters if a segment is lost
- "check_xtalk.sh" to check if the x-talk was done properly (if all segments are working fine)
- "check_xtalk_dead.sh" to check if the x-talk was done properly (if a segment is lost/broken)


## ```3-tt```

- "tt_align.sh" used to obtain the time alignment coefficients for the segments (to the cores)
- "colupdate_TT.sh" to update the Conf/PreprocessingFilterPSA.conf files with the new time calibration parameters


### At this point one could expect the ```4-Tzero``` folder, but typically no scripts are used for the Tzero alignment (time alignment of cores)


## ```5-nd```

- "nd.sh" used for neutron damage corrections


## ```6-recal```

- "recal.sh" for recalibrations of both segments and cores
- "recal_CCSG.sh" for recalibrations of only segments/only cores


## ```7-GTA```

- "GTA.sh" to perform Global Time Alignment of the cores


#### -> In the `old_scripts` folder one can find the old versions of these scripts.


#### Every time an AGATA slave starts the PreProcessing for a new experiment, they should:
- [x] copy the above directories (and the `env_crys.sh` file) directly in the preprocessing-dedicated folders
- [x] source the  `env_crys.sh` script to define gloabally the crystals in use  
