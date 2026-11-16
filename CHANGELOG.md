# AgataSelector changelog

## v-0.6.0

### Notes

The major change is adding the prisma analysis with complete calibrations, tracking algorithm and gates. This allows to run the optimizer on the prisma optical parameters. Other additions are the MCP of the exotic beam production line. 


### Changes

- [x] Adds the complete prisma analysis
- [x] Adds the EXOTIC mcps
- [x] Adds particle identification in SAURON

## v-0.5.0

### Notes

This update focuses on the optimizer(s). A new fully working optimizer was added and can be used to optimize parameters in the lookup table of spider, instead of the selector.conf file.
It still needs to be updated to be used on other detectors but the functionality should be there.

### Changes

- [x] Adds the grid search optimizer
- [x] Updates on the readme
- [x] Adds the reader and event builder for raw files of the 64 ch. digitizer
- [x] Fixes a bug of the optimizer that was not changing some of the parameters

## v-0.4.0

### Notes

Adds some important bug fixes when the coincidence window for building events was set too long. 
Additionally some features are added such as background subtraction (to be tested better).
New detectors are also added (oscar and sauron).

### Changes

- [x] Adds OSCAR
- [x] Adds SAURON
- [x] Fixes bug that occurs when very big event building windows are set
- [x] Adds background subtraction
- [x] Fixes a problem of the optimizer that would not optimize parameters that were not calculated on an event-by-event basis
- [x] Adds LUT files in the root file to improve reproducibility
- [x] Adds the scan option to the optimizer as well as a mode detailed guide on how to optimize under Scripts/

## v-0.3.0

### Notes

Major change of the underlying code. Some bugs were resolved but overall the analysis should remain the same.
Now detectors and coincidences are handled in different classes called by the selector. This makes the way to create coincidences common for all detectors.
Also, it adds the possibility to have some histograms compulsory for every detector such as the count rate.

### Changes

- [x] Adds the possibility to select the position of the reaction in the target used for energy loss reconstruction.
- [x] Adds the possibility of adding the doppler correction in the output tree.
- [x] Adds the gamma_gate to do gamma-gamma-gamma coincidences and angle-gamma corelations
- [x] Adds separate folders in Z and A in Agata-Prisma
- [x] Adds Q-value gate in Agata-prisma for gamma gammas and other stuff with the keyword EX_VALUES in AGATAPRISMA_CONF
- [x] Fixes problem with agata-dante-prisma where the analyze function was called muliple times multiplying artificially the statistics
- [x] Changed an issue with Ex gate in AgataSpider: when EX_GATE e1 e2 in conf, the kinematical lines were generated thinking e1 and e2 as initial energy and final energy, while the gate was applied with e1 starting energy e2 width of the gate (condition was abs(spiderEk-e1)<e2 ). Now in both cases they are Ei and Ef, therefore the condition has been changed to spiderEk>e1 spiderEk<e2

## v-0.2.0

### Notes

No notes.

### Changes

- [x] From now on, the tilt angle of the target set in the selector.conf will be relative to the beam and not relative to the prisma angle.
- [x] Allows to perform the prisma analysis even when one of the parameters is not flagged as ok. This can be set in the selector.conf
- [x] Fixes some problem with energy loss calculations and the computed angles.
- [x] Significant changes in the dante analysis to allow for events where not all x,y,t are found
- [x] Some bug fixes in the Euclides analysis
- [x] The readrawcaen script reads the parameters from a file
- [x] Adds the --sum_all option
- [x] Other minor fixes and changes

## v-0.1.0

### Notes

No notes.

### Changes

- [x] Densities are read from the srim table. This means that the TARGET_CONF has changed. Now one should add the thickness and degrader thickness in mg/cm2
- [x] Adds the Script/selector.sh file in the build folder. If the file is sourced the right folders are added to the path and the tab completition is implemented
- [x] Optimizer is implemented and tested via a root minimizer which can be full configured to optimize any double parameter in the selector.conf file
