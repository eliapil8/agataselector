# PRISMA ANALYSIS ON AGATASELECTOR

The whole analysis of PRISMA is now implemented in the AgataSelector. 
This allows to process the PRISMA steps in the following steps: 
-  Read the raw branches of the PRISMA ROOT Tree
-  Apply thresholds, calibrations and initial gates
-  Run the trajectory reconstruction algorithm
-  Apply Z, Q gates, correct aberrations of the A/Q, calibrate final variables
-  (Optional) Coincidence with gamma-ray detectors
-  Produce plots


To have the same results as what is produced with the PrismaFilters code, here is a guide to set up the selector Conf folder and convert calibration files.

Note: The option PRISMA_UNITS_CUTS in the PRISMA_CONF input file determines whether to use the internal units of the selector for gates and corrections.
YES assumes the conventions used in the output files of the Prismafilters:

| PRISMA_UNITS_CUTS       | Units for lengths |  Factor for A/Q |
|:--------------|:---------------------------------------------------------|:---------------------------------------------------------------------|
|   YES                    | mm |  100 |
|   NO                     | m | 1 |

# Setting gates

The gates can be set automatically with the ```--set_gates [root_file_name]``` condition. This has an interactive menu that opens a canvas. 
On this canvas start a TCut, once you are done, close the tcut with a double click. The program will detect that you are done and will ask you for an id. The id can be the Z in the DE-E gates, the q in the q gates and so on.
Type the id and press enter, you are now able to set a new gate and redo the procedure. Once you are finished setting gates on that particular canvas, use ctrl+c and enter to pass to the next canvas.
The gates will be set in the correct conf folder and will be read at the next analysis procedure. 

The aberration corrections can also be set with the same procedure. In this case you will have to set a TGraph instead and the id will correspond to the mass number. Setting the mass that corresponds to the line is a quick way to do the mass calibration that can be done better late.

# Prisma Manager

Path: ```./Conf/PrismaManger.conf```

In this file you have the possibility of renaming the paths of the gate folders and calibration files

| KEY              |  PATH (default)                                       |               FUNCTION                                                        |
|:--------------|:---------------------------------------------------------|:---------------------------------------------------------------------|
**Gates for raw variables**
| MCP_GATE       | ```/path/to/conf/CUT/PRISMA/RAW/MCP```                         |  Gate for MCP valid events                                            |
| PPAC_GATE      | ```/path/to/conf/CUT/PRISMA/RAW/PPAC      ```                  |  Gates for X_Right+X_Left vs Cathode for each section of the PPAC |
**Calibrations and thresholds for raw variables**
| TOF_CAL        | ```/path/to/conf/CALIBRATION/PRISMA/RAW/tof.cal```             |  Calibration for TOFs|
| PPAC_THRESH    | ```/path/to/conf/CALIBRATION/PRISMA/RAW/ppac_tresh.cal```      |  Thresholds for PPAC positions|
| PPACPOS_CAL    | ```/path/to/conf/CALIBRATION/PRISMA/RAW/ppac_pos_cal.cal```    |  Calibrations for PPAC positions|
| ICE_CAL        | ```/path/to/conf/CALIBRATION/PRISMA/RAW/ice.cal```             |  Thresholds and calibration for IC energy pads|
| ICSIDES_CAL    | ```/path/to/conf/CALIBRATION/PRISMA/RAW/icsides.cal```         |  Thresholds and calibration for IC side pads|
| ICDRIFT_CAL    | ```/path/to/conf/CALIBRATION/PRISMA/RAW/icdrift.cal```         |  Thresholds and calibration for IC drift time|
| MCP_CAL        | ```/path/to/conf/CALIBRATION/PRISMA/RAW/mcp.cal```        |  Calibration for MCP position|
| MCPROT_CAL     | ```/path/to/conf/CALIBRATION/PRISMA/RAW/mcprot.cal```     |  Rotation for MCP position|
| MONITOR_CAL    | ```/path/to/conf/CALIBRATION/PRISMA/RAW/monitor.cal```         |  Thresholds and calibration for monitor energy|
**Gates for analyzed variables**
| TOF_GATE       | ```/path/to/conf/CUT/PRISMA/ANA/TOF```                         |  Gate for TOF vs X_FP|
| Q_GATE         | ```/path/to/conf/CUT/PRISMA/ANA/CHARGE```                      |  Charge state gates|
| Z_GATE         | ```/path/to/conf/CUT/PRISMA/ANA/ZED```                         |  Z gates|
| Q_LINES        | ```/path/to/conf/CUT/PRISMA/ANA/Q_LINES```                     |  Q lines|
| Z_LINES        | ```/path/to/conf/CUT/PRISMA/ANA/Z_LINES```                     |  Z lines|
**Corrections for analyzed variables**
| AOQXMCP_CORR   | ```/path/to/conf/CUT/PRISMA/ANA/AOVERQ_XMCP```                 |  Aberration correction A/Q vs X_MCP|
| AOQYMCP_CORR   | ```/path/to/conf/CUT/PRISMA/ANA/AOVERQ_YMCP```                 |  Aberration correction A/Q vs Y_MCP|
| AOQXFP_CORR    | ```/path/to/conf/CUT/PRISMA/ANA/AOVERQ_XFP```                  |  Aberration correction A/Q vs X_FP|
| ICDEAXFP_CORR  | ```/path/to/conf/CUT/PRISMA/ANA/ICDEA_XFP```                   |  IC dEA vs X_FP correction|
| ICDEAYFP_CORR  | ```/path/to/conf/CUT/PRISMA/ANA/ICDEA_YFP```                   |  IC dEA vs Y_FP correction|
| ICDEABXFP_CORR | ```/path/to/conf/CUT/PRISMA/ANA/ICDEAB_XFP```                  |  IC dEAB vs X_FP correction|
| ICDEABYFP_CORR | ```/path/to/conf/CUT/PRISMA/ANA/ICDEAB_YFP```                  |  IC dEAB vs Y_FP correction|
**Calibrations for analyzed variables**
| AOQ_CAL        | ```/path/to/conf/CALIBRATION/PRISMA/ANA/aoverq.cal```          |  Final calibration of A/Q|
| CHARGE_CAL     | ```/path/to/conf/CALIBRATION/PRISMA/ANA/charge.cal```          |  Final calibration of Q|
| ICE_CORR       | ```/path/to/conf/CALIBRATION/PRISMA/ANA/icecorr.cal```         |  Correction of IC E for non-measured energy|

```
MCP_GATE                /CUT/PRISMA/RAW/MCP
PPAC_GATE               /CUT/PRISMA/RAW/PPAC
TOF_GATE                /CUT/PRISMA/ANA/TOF
Q_GATE                  /CUT/PRISMA/ANA/CHARGE
Z_GATE                  /CUT/PRISMA/ANA/ZED
Z_LINES                 /CUT/PRISMA/ANA/Z_LINES
Q_LINES                 /CUT/PRISMA/ANA/Q_LINES
AOQXMCP_CORR            /CUT/PRISMA/ANA/AOVERQ_XMCP
AOQYMCP_CORR            /CUT/PRISMA/ANA/AOVERQ_YMCP
AOQXFP_CORR             /CUT/PRISMA/ANA/AOVERQ_XFP
ICDEAXFP_CORR           /CUT/PRISMA/ANA/ICDEA_XFP
ICDEAYFP_CORR           /CUT/PRISMA/ANA/ICDEA_YFP
TOF_CAL                 /CALIBRATION/PRISMA/RAW/tof.cal
PPAC_THRESH             /CALIBRATION/PRISMA/RAW/ppac_tresh.cal
PPACPOS_CAL             /CALIBRATION/PRISMA/RAW/ppac_pos_cal.cal
ICE_CAL                 /CALIBRATION/PRISMA/RAW/ice.cal
ICSIDES_CAL             /CALIBRATION/PRISMA/RAW/icsides.cal
ICDRIFT_CAL             /CALIBRATION/PRISMA/RAW/icdrift.cal
MCP_CAL                 /CALIBRATION/PRISMA/RAW/mcp.cal
MCPROT_CAL              /CALIBRATION/PRISMA/RAW/mcprot.cal
MONITOR_CAL             /CALIBRATION/PRISMA/RAW/monitor.cal
AOQ_CAL                 /CALIBRATION/PRISMA/ANA/aoverq.cal
CHARGE_CAL              /CALIBRATION/PRISMA/ANA/charge.cal
ICE_CORR                /CALIBRATION/PRISMA/ANA/icecorr.cal
```


# Calibration files

## General Structure

Calibration files have one line for each channel involved.\
**Keywords** are followed by the numbers associated to them:
- **id**    indicates the indices corresponding to the channel. Followed by 1 or 2 numbers, depending on the file
- **thr**   indicates lower and upper thresholds on the raw channel. Followed by 2 numbers
- **cal**   indicates the coefficients for a polynomial calibration of the channel, when the value is within thresholds. Followed by how many numbers are preferred. The degree of the polynomial is given by the number of coefficients

```
   #Example calibration file
   id 0 0       thr min max     cal p0 p1 p2      
   id 0 1       thr min max     cal p0 p1 p2      
   id 1 0       thr min max     cal p0 p1 p2      
   id 1 1       thr min max     cal p0 p1 p2      
```

## Raw variables calibration files (examples)

### PPAC

Thresholds and calibration of TOF in **TOF_CAL**. \
Thresholds of single position signals in **PPAC_THRESH**, calibrations of difference of position signals in  **PPACPOS_CAL**.

---

**TOF_CAL** \
1 single **id** indicating the PPAC section \
**thr** taken from ```PrismaConf/threshold/tof.thres``` \
**cal** taken combining the calibration of TOF in ```PrismaConf/cal/tof.cal```, the offsets in ```PrismaConf/cal/allineamento-ns.cal``` and the TOF offset in ```PrismaConf/ppac.conf```
```
#Tof calibration file
id 0    cal 330.95377 	-0.0496     thr 150. 4000.
id 1    cal 336.40707 	-0.05028    thr 150. 4000.
id 2    cal 341.47085 	-0.04937    thr 150. 4000.
id 3    cal 318.2776 	-0.04915    thr 150. 3860.
id 4    cal 317.91215 	-0.04952    thr 150. 3781.
id 5    cal 316.89904 	-0.04951    thr 150. 3767.
id 6    cal 331.59844 	-0.04948    thr 150. 3877.
id 7    cal 331.24438 	-0.04928    thr 150. 3755.
id 8    cal 330.63445 	-0.04949    thr 150. 3777.
id 9    cal 328.97052 	-0.04908    thr 150. 3778.   
```

---

**PPAC_THRESH** \
2 **id** numbers. \
First **id** is the type of signal: 0 for X_right, 1 for X_left, 2 for X_Cathode, 3 for Y \
Second **id** is the index of the channel: 0-9 for all X signals, 0-1 (up-down) for Y \
No calibration needed at this step.
```
#PPAC position threshold file 
# Xright
id 0 0 thr 2192 3701
id 0 1 thr 1355 2872
id 0 2 thr 1272 2739
id 0 3 thr 1272 2764
id 0 4 thr 1325 2824
id 0 5 thr 1199 2664
id 0 6 thr 1363 2833
id 0 7 thr 1295 2661
id 0 8 thr 1494 3033
id 0 9 thr 1358 2866
# Xleft
id 1 0 thr 1366 2896
id 1 1 thr 1079 2669
id 1 2 thr 1164 2650
id 1 3 thr 1133 2646
id 1 4 thr 1303 2790
id 1 5 thr 1255 2777
id 1 6 thr 1299 2833
id 1 7 thr 1390 2757
id 1 8 thr 1495 3010
id 1 9 thr 1508 3028
# XCathode
id 2 0 thr 500. 3500.
id 2 1 thr 500. 3500.
id 2 2 thr 500. 3500.
id 2 3 thr 500. 3500.
id 2 4 thr 500. 3500.
id 2 5 thr 500. 3500.
id 2 6 thr 500. 3500.
id 2 7 thr 500. 2442.
id 2 8 thr 500. 3500.
id 2 9 thr 500. 3500.
# YUp
id 3 0 thr 1 4090
# YDown
id 3 1 thr 1 4090  
```
---
**PPACPOS_CAL** \
2 **id** numbers. \
First **id** is the type of signal: 0 for X_right-X_left, 1 for X_right-X_cathode, 2 for X_cathode-X_left, 3 for Y_up-Y_down \
Second **id** is the index of the channel (section of the PPAC) 
```
#PPAC position calibration file 
# XRight - XLeft
id 0 0 thr -10000 10000 cal 22.31750  0.0335680
id 0 1 thr -10000 10000 cal 142.10362 0.0324775
id 0 2 thr -10000 10000 cal 246.59517 0.0340482
id 0 3 thr -10000 10000 cal 345.65764 0.0334886
id 0 4 thr -10000 10000 cal 449.04379 0.0337485
id 0 5 thr -10000 10000 cal 552.74964 0.0336691
id 0 6 thr -10000 10000 cal 648.90747 0.0334444
id 0 7 thr -10000 10000 cal 753.29750 0.0370506
id 0 8 thr -10000 10000 cal 849.44794 0.0331231
id 0 9 thr -10000 10000 cal 955.18705 0.0333215
# XRight - XCathode
id 1 0 thr -10000 10000 cal 0.544008 0.066592
id 1 1 thr -10000 10000 cal 145.48962 0.066005
id 1 2 thr -10000 10000 cal 255.25030 0.0684821
id 1 3 thr -10000 10000 cal 362.36919 0.067468
id 1 4 thr -10000 10000 cal 452.16685 0.0670162
id 1 5 thr -10000 10000 cal 564.32474 0.0688689
id 1 6 thr -10000 10000 cal 659.74339 0.0679771
id 1 7 thr -10000 10000 cal 769.99095 0.0734061
id 1 8 thr -10000 10000 cal 866.47145 0.0650189
id 1 9 thr -10000 10000 cal 974.99448 0.0661813
# XCathode - XLeft
id 2 0 thr -10000 10000 cal 44.7848	0.0662941
id 2 1 thr -10000 10000 cal 142.088	0.0622925
id 2 2 thr -10000 10000 cal 238.179	0.0672878
id 2 3 thr -10000 10000 cal 329.314	0.0664416
id 2 4 thr -10000 10000 cal 445.765	0.0675675
id 2 5 thr -10000 10000 cal 541.622	0.0663105
id 2 6 thr -10000 10000 cal 638.496	0.0653581
id 2 7 thr -10000 10000 cal 737.105	0.0729875
id 2 8 thr -10000 10000 cal 832.018	0.0668445
id 2 9 thr -10000 10000 cal 935.327	0.0660937
# YUp - YDown
id 3 0 thr -10000 10000 cal 6.56989 0.05591398
id 3 1 thr -10000 10000 cal 6.56989 0.05591398
id 3 2 thr -10000 10000 cal 6.56989 0.05591398
id 3 3 thr -10000 10000 cal 6.56989 0.05591398
id 3 4 thr -10000 10000 cal 6.56989 0.05591398
id 3 5 thr -10000 10000 cal 6.56989 0.05591398
id 3 6 thr -10000 10000 cal 6.56989 0.05591398
id 3 7 thr -10000 10000 cal 6.56989 0.05591398
id 3 8 thr -10000 10000 cal 6.56989 0.05591398
id 3 9 thr -10000 10000 cal 6.56989 0.05591398
```

### Ionization chamber

Threshold and calibrations for energy pads in **ICE_CAL**, for side pads in **ICSIDES_CAL**, for drift time **ICDRIFT_CAL**

---

**ICE_CAL** \
2 **id** numbers. \
First **id** is the row of the IC: 0-A, 1-B, 2-C, 3-D   \
Second **id** is the index of the column 0-9

```
#IC Pads energy calibration file 
# Layer A 
id 0 0 cal -62.603056   0.869968 thr 78. 3980.
id 0 1 cal -75.346507   0.864128 thr 100. 3980.
id 0 2 cal -76.460709   0.875566 thr 96. 3980.
id 0 3 cal -93.248450   0.849726 thr 120. 3980.
id 0 4 cal -86.267566   0.877762 thr 107. 3980.
id 0 5 cal -74.314566   0.870261 thr 94. 3980.
id 0 6 cal -84.972948   0.871055 thr 101. 3980.
id 0 7 cal -69.765214   0.861910 thr 95. 3980.
id 0 8 cal -61.901326   0.869995 thr 79. 3980.
id 0 9 cal -48.075377   0.881899 thr 76. 3980.
# Layer B
id 1 0 cal -57.448534	0.846074 thr 75. 3980.
id 1 1 cal -62.818705	0.877912 thr 77. 3980.
id 1 2 cal -65.865541	0.876581 thr 85. 3980.
id 1 3 cal -67.789381	0.883898 thr 84. 3980.
id 1 4 cal -76.247806	0.861720 thr 98. 3980.
id 1 5 cal -75.164742	0.880693 thr 92. 3980.
id 1 6 cal -81.219868	0.872663 thr 109. 3980.
id 1 7 cal -73.445292	0.868968 thr 97. 3980.
id 1 8 cal -67.139027	0.881685 thr 87. 3980.
id 1 9 cal -55.027943	0.882714 thr 95. 3980.
# Layer C
id 2 0 cal -83.764401	0.870126 thr 200. 3980.
id 2 1 cal -69.676637	0.872239 thr 89. 3980.
id 2 2 cal -75.537068	0.857441 thr 98. 3980.
id 2 3 cal -77.017493	0.875974 thr 96. 3980.
id 2 4 cal -66.292309	0.858697 thr 84. 3980.
id 2 5 cal -82.273510	0.871664 thr 103. 3980.
id 2 6 cal -85.508083	0.876121 thr 108. 3980.
id 2 7 cal -87.799888	0.860388 thr 115. 3980.
id 2 8 cal -67.629896	0.869621 thr 87. 3980.
id 2 9 cal -52.797933	0.878218 thr 82. 3980.
# Layer D
id 3 0 cal -63.464976	0.878909 thr 81. 3980.
id 3 1 cal -77.664255	0.880612 thr 96. 3980.
id 3 2 cal -71.947027	0.877465 thr 90. 3980.
id 3 3 cal -75.015023	0.869164 thr 96. 3980.
id 3 4 cal -85.113416	0.881170 thr 111. 3980.
id 3 5 cal -82.530376	0.871133 thr 109. 3980.
id 3 6 cal -76.727374	0.863861 thr 107. 3980.
id 3 7 cal -66.545695	0.882009 thr 89. 3980.
id 3 8 cal -51.254737	0.879499 thr 69. 3980.
id 3 9 cal -61.431468	0.899307 thr 91. 3980.
```

---

**ICSIDES_CAL** \
2 **id** numbers. \
First **id** is the row of the IC: 0-A, 1-B, 2-C, 3-D   \
Second **id** is the index of the side pad 0-1 \
Only thresholds are needed here.

```
#IC Sides threshold file 
# Layer A 
id 0 0 cal 0 1 thr 160.  4000.
id 0 1 cal 0 1 thr 275.  4000.
# Layer B 
id 1 0 cal 0 1 thr 160.  4000.
id 1 1 cal 0 1 thr 140.  4000.
# Layer C
id 2 0 cal 0 1 thr 272.  4000.
id 2 1 cal 0 1 thr 252.  4000.
# Layer D
id 3 0 cal 0 1 thr 128.  4000.
id 3 1 cal 0 1 thr 263.  4000.  
```
---

**ICDRIFT_CAL** \
2 **id** numbers. \
First **id** is the row of the IC: 0-A, 1-B, 2-C\
Second **id** is the index of the column 0-9 \

```
#IC Drift time calibration
# Layer A
id 0 0 cal 0 1 thr 0 4096
id 0 1 cal 0 1 thr 0 4096
id 0 2 cal 0 1 thr 0 4096
id 0 3 cal 0 1 thr 0 4096
id 0 4 cal 0 1 thr 0 4096
id 0 5 cal 0 1 thr 0 4096
id 0 6 cal 0 1 thr 0 4096
id 0 7 cal 0 1 thr 0 4096
id 0 8 cal 0 1 thr 0 4096
id 0 9 cal 0 1 thr 0 4096
# Layer B
id 1 0 cal 0 1 thr 0 4096
id 1 1 cal 0 1 thr 0 4096
id 1 2 cal 0 1 thr 0 4096
id 1 3 cal 0 1 thr 0 4096
id 1 4 cal 0 1 thr 0 4096
id 1 5 cal 0 1 thr 0 4096
id 1 6 cal 0 1 thr 0 4096
id 1 7 cal 0 1 thr 0 4096
id 1 8 cal 0 1 thr 0 4096
id 1 9 cal 0 1 thr 0 4096
# Layer C
id 2 0 cal 0 1 thr 0 4096
id 2 1 cal 0 1 thr 0 4096
id 2 2 cal 0 1 thr 0 4096
id 2 3 cal 0 1 thr 0 4096
id 2 4 cal 0 1 thr 0 4096
id 2 5 cal 0 1 thr 0 4096
id 2 6 cal 0 1 thr 0 4096
id 2 7 cal 0 1 thr 0 4096
id 2 8 cal 0 1 thr 0 4096
id 2 9 cal 0 1 thr 0 4096
```


### MCP

Calibration of X and Y positions.
First, roation matrix is applied to raw X_raw and Y_raw variables, then the transformed coordinates are calibrated.

---

**MCPROT_CAL** \
Only **id** needed is 0\
Contains coefficients for rotation matrix between X and Y  \
**cal** x0 x1 y0 y1

>X = X_raw * x0 + Y_raw * x1     \
>Y = X_raw * y0 + Y_raw * y1 

```
#Rotation matrix for MCP positions
# x0 x1 y0 y1
id 0 cal 1 -0.0818247 0.0448284 1
```
---

**MCP_CAL** \
1 **id** number: 0-X. 1-Y \
Only calibration is needed (thresholds are set with the 2D gate)

>X_final = cal( X )    \
>Y_final = cal( X )

```
#MCP position calibration file
#X
id 0  cal 40.7527 -0.029482
#Y
id 1  cal -74.0692 0.0299315
```

### Monitors

---


**MONITOR_CAL** \
1 **id** number: index of monitor
```
#Monitor calibration file
id 0  cal 0 1 thr 0 4096
id 1  cal 0 1 thr 0 4096
```


### Reconstructed variables

After running the solver and checking the Z and Q gates, A/Q is calibrated and the mass is calculated.

> A = cal( A/Q ) * cal( Q )

The calibration might depend on Z and Q

**AOQ_CAL** \
Z-dependent calibration of A/Q \
1 **id** number: Z value \
No thresholds needed. \ 
```
#Calibration file
id 28  cal 0 1
id 29  cal 0 1
id 30  cal 0 1
```
---

**CHARGE_CAL** \
Effective charge assigned to correct the calibration of the mass. \
2 **id** numbers: Z and Q value \
No thresholds needed.
```
#Calibration file
id 28 25 cal 0 1
id 28 26 cal 0 1
id 28 27 cal 0 1
id 29 25 cal 0 1
id 29 26 cal 0 1
id 29 27 cal 0 1
id 30 26 cal 0 1
id 30 27 cal 0 1
id 30 28 cal 0 1
```

---

**ICE_CORR** \
Corrections to IC energy for the calculation of the charge states according to the formula
$$Q=\frac{E}{ u (\gamma - 1) A/q}$$
Thresholds should be set wide.

**id** 0 is to calculate the non-measured energy **ic_e_nm** to correct the IC energy. The calibration is dependent on IC_DE_A.\
**id** 1 is to correct the charge state **Q**  against the non measured energy (temporary fix).\
**id** 2 is to calibrate the charge state to the real value (temporary fix). \
**id** 3 (temporary fix) \
**id** 4 (temporary fix) \
**id** 5 calibrates the energy measurements of the IC to MeV. To be estimated by looking at the calibration of the mass from the IC: $$A_E=\frac{E}{ u (\gamma - 1)}$$

Note: recommended to use either only **id = 0** or **id = 5**

```
#Calibration file
id 0  cal 0 0.0779105 5.61303e-05 
id 1 cal 0 0
id 2 cal 0 1
id 5 cal 0 1 thr -10000 10000 
```
or
```
#Calibration file
id 0 cal 0 0
id 1 cal 0 0
id 2 cal 0 1
id 5 cal -13.5 0.174377 thr -10000 10000 
```
---


# 2D Gates

2-dimensional gates are used for different selections in the analysis. \
Their structure is flexible to be able to have input **ids** and output **labels** for the identification. \
The gates must be stored in `TCutG*` format inside a `.root` file. \
In the prismaManger.conf the folder of the type of gates is indicated and the files are read automatically.

The name is very important as it contains all of the input and output **ids**.

> TFile * icerbeta__0_32__30.root  \
> TCutG * icerbeta__0_32__30

The first string `icerbeta` can be anything, then the characters `__` **label** the start of the input **ids**. \
If there is more than 1 **id**, they are separated by single `_`. \
The next characters `__` indicate the output **label**. \
Also in this case there can be more than one output **label**, but not in the case of the Prisma analysis.

## Raw variable gates


**MCP_GATE** 

Gate for labeling the valid events from the MCP.
Applied on Y_raw vs X_raw matrix. \
In the PrismaTree: `MCP_raw[1]:MCP_raw[0]`

The **ids** and **labels** are fixed: 
>mcp__0__1.root

---

**PPAC_GATE**

Gate for labeling the valid events from the PPAC positions.
There is one gate per PPAC section (**0-9**).

In the PrismaTree:
`PPAC_Xright[i]+PPAC_Xright[i]:PPAC_Cathode_raw[i]`

1 single **id** which indicates the PPAC section, **label** is fixed to 1:
>cath__5__1.root

---

## Analyzed variable gates and graphs

**TOF_GATE**

Gate for rejecting invalid TOF events (retriggering). \
To be set in the `TOF:X_FP` plot.

The **ids** and **labels** are fixed: 
> tof__0__1.root

---

**Z_GATE**

Gates for selecting the **Z** number from the ionization chamber. \
Different types of selection plots can be chosen and set in the **PRISMA_CONF** with the value of **DE_TYPE**:

- **0** :   `IC_DE_AB vs IC_E`
- **1** :   `IC_DE_A vs IC_E`
- **2** :   `IC_E vs IC_Range` (not implemented)
- **3** :   `IC_DE_A_corr vs IC_Eres_corr`
- **4** :   `IC_DE_A_corr vs IC_Eres_corr` (only as input for the continuous Z id, see ZED_LINES section)
- **6** :   `IC_DE_AB_corr vs IC_Eres_corr`

The names have 1 **id** and 1 **label**.
The **id** is the  **DE_TYPE**, while the **label** is the identified **Z**.
> dee__1__30.root

---
**Z_LINES**

TGraphs for defining a continuous value of atomic number.
The option can be used with  **DE_TYPE = 4**.
The graph should be a line that follows the top of the band relative to one **Z** in the `IC_DE_A_corr vs IC_Eres_corr` plot.

The names have 1 **id** and 1 **label**.
Both the **id** and the **label** have to be the identified **Z**
> zed__30__30.root

A graph with **id** z__0__0 should be present to define a rejection line that surrounds the region of identification. Every event out of this region is assigned **Z=0**.

---
**Q_GATE**

Gates for selecting the **Q** number comparing the energy in the IC to the results of the trajectory reconstruction. \
Each Q gate is set on a single Z value. \
Different types of selection plots can be chosen and set in the **PRISMA_CONF** with the value of **CHARGE_TYPE**:

- **0** :   `IC_E vs RBeta`
- **1** :   `IC_E/RBeta vs IC_E`
- **2** :   `IC_E vs X_FP` (not recommended)
- **3** :   `Q vs A/Q` 

**CHARGE_TYPE = 4** assigns the value of **Q** based on the rounding of **Q** (see ICE_CORR section).

Gates starting from **200** onwards will assign the Q value based on the intersection of two gates (e.g. **202** will consider assign a charge value Q only if the point is first in the gate of type **0**, then in the gate of type **2**). It might be useful in case of dirty gates in one variable, but cleaner in the other.

The names have 2 **id** and 1 **label**.
The **ids** are the  **CHARGE_TYPE** and the **Z** , while the **label** is the identified **Q**.
> icerbeta__0_32__30.root

---
**Q_LINES**

TGraphs for defining (or overwriting) a continuous value of atomic number.
The option can be used with  **CHARGE_TYPE = 5**.
The graph should be a line that follows the top of the band relative to one **Q** in the `Q vs Beta` plot.

The names have 1 **id** and 1 **label**.
Both the **id** and the **label** have to be the identified **Q**
> q__30__30.root

A graph with **id** q__0__0 should be present to define a rejection line that surrounds the region of identification. Every event out of this region is assigned **Q=0**.

---

# Corrections

The corrections refer to a 2D plot, where the variable on the Y axis is corrected with respect to the value on the X axis. \
These are used for correcting aberrations of the A/Q with respect to the MCP and PPAC positions, and correcting the IC_DE_A against the PPAC position to improve the separation in the Z matrix.   \

The structure and name of the files is analogous to the 2D gates, except that the object in the `.root` file is a `TGraph*` instead of a `TCutG*`. \
The `TGraph*` is fitted with a `TSpline3` and the variable on the Y axis is corrected by the difference of the value of the spline at the corresponding value on the X axis to a reference Y value.

## A/Q aberration corrections

To correct the aberration in A/Q and improve the mass resolution it is needed to correct the dependence on the positions at entrance and focal plane. The major corrections are for the **MCP_X**, **MCP_Y**, **X_FP**.  \
The positions for drawing the spline are expressed in `mm` and the A/Q has the natural range (around [2,5]).

The files for all these corrections have the same structure.    \
The name has 2 **ids**, corresponding to the value of **Z** and **Q** where to apply the correction. \
There is 1 **label** and it indicates the mass **A** corresponding to the peak around which the spline is centered.

**AOQXMCP_CORR**
> aoverqxmcp__26_40__120.root

**AOQYMCP_CORR**

> aoverymcp__26_40__120.root

**AOQXFP_CORR**

> aoverqxfp__26_40__120.root


## IC_DE_A corrections

These corrections are used to improve the selection resolution in the Z matrix.    \
When using **DE_TYPE = 3/4/5**.
**IC_DE_A** is correted against  **X_FP** and **Y_FP**. \
The **id** and **label** are fixed.
The value of positions are expressed in `m`.

**ICDEAXFP_CORR**
>icdeaxfp__0__0.root

**ICDEAYFP_CORR**
>icdeayfp__0__0.root

## IC_DE_AB corrections

As before, but when using **IC_DE_AB** instead of **IC_DE_A**.
When using **DE_TYPE = 6**
The **id** and **label** are fixed.
The value of positions are expressed in `m`.

**ICDEABXFP_CORR**
>icdeabxfp__0__0.root

**ICDEABYFP_CORR**
>icdeabyfp__0__0.root
