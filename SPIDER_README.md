# SPIDER ANALYSIS ON AGATASELECTOR

SPIDER analysis is fully carried out in the agataselector. The analysis requires the presence of a LUT file, which is customarily called LUT_SPIDER.dat.
This file has the following structure

```
####### SPIDER ######                                                                                                                                                                                              
#                                                                                                                                                                                                                  
# the "map" number conversion into detector and strip:                                                                                                                                                             
# strip = (map % 10) + 1                                                                                                                                                                                           
# detector = (map / 10) + 1                                                                                                                                                                                        
#                                                                                                                                                                                                                  
#                                                                                                                                                                                                                  
# Board ch  map   name  thr_lo thr_hi  theta   phi    TimeOffset npar   calpars [p0, p1]                                                                                                                  
    2    0   11   D1S1   0.29   2.92   155.2   103.99  104.651    2     0.0395306   0.00148346                                                                                                                         
    2    1   10   D1S0   0.29   2.92   159.6   103.99  104.504    2     0.0276359   0.00145279             
...
```
Board and channel mapping to theta and phi will be given by the local group. The parameters to change are ```thr_low``` and ```thr_hi```, which represent the range of accepted energies in SPIDER (in MeV), TimeOffset to align in time the events with AGATA channel-by-channel and the calibration of each channel to MeV.
Notice that the calibration at this state can be of whatever order (npar indicates the polynomial order for the calibration), but usually a linear calibration is used (  p0 + ADC*p1).

## SPIDER analysis
The analysis of SPIDER is consists of several steps: 
- **CALIBRATION**: each event is calibrated according to the LUT, plus a global linear recalibration ```E_SHIFT```+ E*```E_GAIN``` as defined in the selector.conf;
- **KINEMATICS RECONSTRUCTION**: each event is reconstructed according to the kinematics defined in selector.conf in ```TARGET_CONF``` and ```REACTION_CONF```. We get therefore the excitation energy Ex. This excitation energy can be recalibrated in several ways to try remove extraneous dependencies:
        - often due to a misplacement of the detector in analysis, Ex shows an oscillatory dependence from phi. This dependence is corrected by subtraction of a function of the kind $$A\cos(\phi/B+C),$$ where A is ```COS_AMPLI```, B is ```COS_PERIOD```, C is ```COS_PHASE```;
        - sometimes bad calibration results in random shifts in Ex on a channel-by-channel basis. These can be corrected à la Prisma, with a TGraph found in ```EX_ID_PATH```
        - sometimes finding the right position of SPIDER is difficult, and the minimizer cannot converge properly. This leaves some residual dependence of Ex as a function of theta and phi, with no clear analytical form. These are corrected à la Prisma, with a TGraph saved in a file that the selector finds in ```EX_THETA_PATH``` and ```EX_PHI_PATH``` (the corrections are performed in this order);
        - since recalibrations might move the peaks around, a global quadratic recalibration (```EX_SHIFT```, ```EX_GAIN```, ```EX_QUAD```) is performed as last step.
- **HISTOGRAMMING**: in order to manage memory more efficiently, histograms are defined in theta and energy according to the selector.conf. Events outside these ranges are not discarded, they will just be overflow in the histograms.


It is highly recommended to perform only ONE of the recalibrations of Ex, so either in Theta and Phi, or according to ID, and then put everything in the proper place via quadratic global recalibration.

Since the code is of Prisma origin, there is some limitation on the naming convention. The TGraph name must end with ```__0__1```, e.g. ```miao__0__1```, while the file can be called whatever, e.g. ```whatever.root```.


## AGATA-SPIDER coincidence analysis

The analysis of AGATA-SPIDER coincidences consists in:
- **DOPPLER CORRECTION**: the keywords ```FRAGMENT_POSITION``` and ```BP_FRAGMENT_POSITION``` define at what energy the Doppler correction is performed. These can be ```BEGIN_TARGET```, ```MID_TARGET```, ```END_TARGET```, ```MID_DEGRADER```, ```END_DEGRADER```, according to the lifetime of the state of interest.
- **HISTOGRAMMING**: the keywords ```THETA_BINS```, ```THETA_MIN``` e ```THETA_MAX``` define the histograms axis of the gamma-particle relative angle. In the case of SPIDER alone, these keywords do not exist since the particle theta is defined by the spider angular coverage, i.e. (50, 120., 170.).
        - histograms can be created according to different conditions. In the AgataSpider/ folder, you will always find the ungated histograms. Then, several conditions can be put, either on the kinematic line, or on excitation energy. A routine that performs p-gamma-gamma coincidences is also implemented (see later).


## Example configuration file

The following keywords in the configuration file are accepted:


| KEY                     |  default                  |               FUNCTION                                       |
|:------------------------|:--------------------------|:-------------------------------------------------------------|
**SPIDER_CONF**
| LUT                     | ```LUT_SPIDER.dat```      |  Name of LUT file                                            |
| COINC_W_LEFT            |  ```-5```                 |  Usually there is no coincidence in spider, fake val         |
| COINC_W_RIGHT           |  ```5```                  |  Usually there is no coincidence in spider, fake val         |
| COINC_W_LEFT_LEFT_BKG   |  ```-10```                |  Usually there is no coincidence in spider, fake val         |
| COINC_W_LEFT_RIGHT_BKG  |  ```-5```                 |  Usually there is no coincidence in spider, fake val         |
| COINC_W_RIGHT_LEFT_BKG  |   ```5```                 |  Usually there is no coincidence in spider, fake val         |
| COINC_W_RIGHT_RIGHT_BKG |   ```10```                |  Usually there is no coincidence in spider, fake val         |
| ANGLE                   |   ```0. deg```            |  Rotation in theta                                           |
| PHI                     |   ```0. deg```            |  Rotation around beam axis                                   |
| THETA_X                 |   ```0. deg```            |  Small angle for optimization purposes                       |
| THETA_Y                 |   ```0. deg```            |  Small angle for optimization purposes                       |
| X_SHIFT                 |   ```0. mm```             |  Shift in X                                                  |
| Y_SHIFT                 |   ```0. mm```             |  Shift in Y                                                  |
| Z_SHIFT                 |  ```-8.5 mm```            |  Shift in Z  (>0 means closer to target)                     |
| TIME_UNIT               |   ```10. ns```            |  AGATA GTS default                                           |
| CFD_UNIT                |   ```2.  ps```            |  Depends on digitizer used                                   |
| ENABLE_HISTS            |   ```YES```               |                                                              |
| ENABLE_TREE             |   ```NO```                |  Please don't put yes                                        |
| E_BINS                  | ```1000```                |  Number of bins for E axis                                   |
| E_MAX                   | ```200 MeV      ```       |  Maximum value of E axis                                     |
| E_MIN                   | ```0 MeV      ```         |  Minimum value of E axis                                     |
| EX_BINS                 | ``` 1000     ```          |  Number of bins for Ex axis                                  |
| EX_MIN                  | ``` -10 MeV      ```      |  Minimum value for Ex axis                                   |
| EX_MAX                  | ```  90 MeV      ```      |  Maximum value for Ex axis                                   |
| E_SHIFT                 | ``` 0. MeV      ```       |  Global energy linear recalibration : constant term          |
| E_GAIN                  | ```  1.      ```          |  Global energy linear recalibration : linear term            |
| EX_SHIFT                | ``` 0. MeV      ```       |  Global Ex quadratic  recalibration : constant term          |
| EX_GAIN                 | ```  1.      ```          |  Global energy linear recalibration : linear term            |
| EX_QUAD                 | ```  0.      ```          |  Global energy linear recalibration : quadratic term         |
| COS_AMPLI               | ``` 0.      ```           |  Cosine dependence of Ex vs Phi: amplitude                   |
| COS_PHASE               | ``` 0.      ```           |  Cosine dependence of Ex vs Phi: phase                       |
| COS_PERIOD              | ``` 1. deg  ```           |  Cosine dependence of Ex vs Phi: period                      |
| DEADLAYER_THICKNESS     | ``` 0. mg/cm2      ```    |  Thickness of Al dead layer of SPIDER (almost never observed)|
| DEARLAYER_MATERIAL      | ```  27-13      ```       |  Material of dead layer of SPIDER (traditionally Al for Si detectors) |
| BROKEN_CHANNEL          | ```  0 0      ```         |  BoardId and ChannelId to exclude from analysis              |
| E_THETA_CUT_PATH        | ``` /CUT/SPIDER/ETheta ```  |  Path to TCutG for E-Theta gating                          |
| EX_THETA_PATH           | ``` /CUT/SPIDER/ExTheta ``` |  Path to TGraph for Ex-Theta correction                    |
| EX_PHI_PATH             | ``` /CUT/SPIDER/ExPhi ```   |  Path to TGraph for Ex-Phi correction                      |
| EX_ID_PATH              | ``` /CUT/SPIDER/ExID ```    |  Path to TGraph for Ex-ID correction                       |
| KEEP_HIGH_ENE_EVT       | ```NO```                  |  See following                                             |
**AGATASPIDER_CONF**
| FRAGMENT_POSITION       | ```  MID_TARGET    ```    |  Where to perform DC                                         |
| BP_FRAGMENT_POSITION    | ```  MID_TARGET    ```    |  Where to perform DC of binary                               |
| COINC_W_LEFT            |  ```-5```                 |  Window limit on the time coincidence peak                   |
| COINC_W_RIGHT           |  ```5```                  |  Window limit on the time coincidence peak                   |
| COINC_W_LEFT_LEFT_BKG   |  ```-10```                |  Window limit outside the time coincidence peak              |
| COINC_W_LEFT_RIGHT_BKG  |  ```-5```                 |  Window limit outside the time coincidence peak              |
| COINC_W_RIGHT_LEFT_BKG  |   ```5```                 |  Window limit outside the time coincidence peak              |
| COINC_W_RIGHT_RIGHT_BKG |   ```10```                |  Window limit outside the time coincidence peak              |
| ENABLE_HISTS            |   ```YES```               |                                                              |
| ENABLE_TREE             |   ```NO```                |  Please don't put yes                                        |
| ENABLE_REDBRANCH        |   ```NO```                |                                                              |
| THETA_BINS              | ``` 1000     ```          |  Number of bins for Ex axis                                  |
| THETA_MIN               | ``` -10 MeV      ```      |  Minimum value for Ex axis                                   |
| THETA_MAX               | ```  90 MeV      ```      |  Maximum value for Ex axis                                   |
| EX_VALUES               | ```  0.  1.    ```        |  Condition histograms to have events in this range of Ex     |
| GAMMA_GATE              | ```  0 4096 DC 0 0```     |  Condition histograms to have events with a g-g coincidence  |
| EX_GAMMA_GATE           | ```  0 4096 DC 0 0```     |  Logic and between the previous two                          |

Notice that to perform background subtraction, i.e. activate COINC_W_LEFT_LEFT_BKG, COINC_W_LEFT_RIGHT_BKG, ... the --subtract_bkg is required when launching the selector.


## Conditioning histograms
Accepting or rejecting events can be done in different ways in SPIDER. The most basic one is to put the flag ```KEEP_HIGH_ENE_EVT``` to YES. If there are multiple spider hits in the same event, only the highest energy one will be kept.
Otherwise, there are two complementary way to do it - either in energy or in excitation energy. 
Gating in energy can be done in the Spider/m_Theta_Energy matrix. Since in binary reactions the emission energy depends on the angle, this cut is performed via a TCutG. Once you've drawn a TCutG in the region of interest, give it a name, and save it in ```E_THETA_CUT_PATH```.
In this case, there is no general limitation on the name of the TCutG or file, e.g. it can be called ```myBeautifulGate``` inside the ```SpiderBestDetector.root``` file.


Gating in excitation energy is different. Since excitation energy should be independent of kinematics, and only depend on structure, this gate is implemented via a if( Ex > gate1 && Ex < gate2 ) clause. The gate extrema are defined in the ```AGATASPIDER_CONF``` via the ```EX_VALUES``` key. These can be put by looking at the Spider/h_Ex histogram. Generally, gating in E-Theta is preferred if you have a broad structure in excitation energy with some noise around. Gating in Ex is instead preferred if you have narrow peaks in the Ex spectrum, and you want to exclude feeders. The use of one or the other highly depends on the experiment.

Triple coincidences can also be required. The ```GAMMA_GATE``` keyword puts a condition on the existence of a gamma-gamma coincidence. The syntax is

``` GAMMA_GATE [int] [int] [string] [int] [int]```

-  the first two integer values define the energy of the gamma required in the gate. Therefore, ```GAMMA_GATE 1160 1180``` will be filled with all gammas seen in coincidence with a gamma ray of energy between 1160 and 1180 keV.
- the string after defines which kind of energy needs to be considered when putting the gate. Putting this to none will consider the bare energy, while specifying DC or DCBP will put a gate on the Doppler-corrected or Doppler-corrected for binary partner energy.
- the last two integer values define the range of angles to exclude from the gate. That is because DC and non-DC energy match closely around 90 deg, so you might want to exclude the area between 80 and 100 deg.

```EX_GAMMA_GATE``` has the same syntax, but requires also that the coincidence is with a proton defined in the excitation energy ranges defined in ```EX_VALUES``` or ```E_THETA_CUT_PATH```.

```GAMMA_GATE``` histograms are the only ones that have a slightly longer name. This is because maybe one would like the gamma-gamma analysis but not the singles one.
If you are therefore using the --only_enabled_histos option (as you always should), you will not see them in the output file. Therefore, it is recommended to change the name of the enabled_histos.conf file in the selector.conf (e.g. to temp_histos.conf) and run

```
 ./RunSelector 21 --conf selector.conf --nrthr 1 --nrevts 1
```
this will print out a file called temp_histos.conf with all histograms enabled. Vimdiff the enabled_histos.conf and temp_histos.conf files to add to the former the new histograms, remove temp_histos.conf, change back the selector.conf to enabled_histos.conf and run your analysis with --only_enabled_histos.

## An example
37S has a first excited state at 646 keV, fed by a higher lying state at 2638 keV. The nucleus is populated in 36S(d,p) and the p is detected in SPIDER. Therefore, 37S is considered the binary.
If the keyword
> ```GAMMA_GATE 620 660 DCBP 90 110```

is found in the selector.conf file, the folder GammaGate_620_660_DCBP will be created, containing a series of histograms (h_E, h_EDC, h_EDCBP, m_Theta_E, m_Theta_EDC, m_ThetaBP_E, m_ThetaBP_EDCBP, ...).
An event comes containing a proton and three gammas, one at 614 keV, one at 1926 keV, the last at 800 keV; the three energies doppler corrected for the binary partner (37S) are 646, 1992, 720 keV. The selector will fill the AgataSpider/ histograms with both gammas. Then it will see that 646 is inside the GAMMA_GATE defined, as it is put in the DCBP energy. 

* If the relative angle between the proton and the 646 keV gamma ray is outside the range [90-110] degree, then the histograms in this folder are filled with other two gamma rays, i.e. h_E will be filled with 1926 and 800, h_EDCBP with 1992 and 720, m_EDCBP_EDCBP will be filled with (1992, 720) and (720, 1992) and so on.

* If the relative angle is inside the range, nothing in this folder is filled.
    
Notice that if the keyword was
> ```GAMMA_GATE 620 660 DCBP 0 0```

the h_E and h_EDCBP in the GammaGate_620_660_DCBP/ would be exactly equal to the projections of the m_E_EDCBP and m_EDCBP_EDCBP matrices in the range 620 660 keV of the EDCBP axis. The exclusion range in this routine allows to avoid double counting.

Suppose instead you have
> ```EX_VALUES 0. 1.```    
> ```EX_VALUES 1.8 2.8```   
> ```EX_GAMMA_GATE 620 660 DCBP 90 110```

the output file will have the Agata/ folder, Spider/ folder and AgataSpider/ folder. Inside the last one, two folders will be created, ExGate_Ei0.000_Ef1.000 and ExGate_Ei1.800_Ef2.800, corresponding to the two ```EX_VALUES``` keywords. Inside each of them, the GammaGate_620_660_DCBP/ folder will also be present. If nothing is commented out, you have 17 histograms containing gamma-ray informations. These will be found in Agata/, AgataSpider/, ExGate_Ei0.000_Ef1.000/, ExGate_Ei1.800_Ef2.800/, ExGate_Ei0.000_Ef1.000/GammaGate_620_660_DCBP/, 
ExGate_Ei1.800_Ef2.800/GammaGate_620_660_DCBP/, for a total of 6 folders. So with three keywords you just required the creation of 102 histograms/matrices, instead of just the 34 if those keywords were not present. 

## Suggestions

- We are not sure that using ```E_THETA_CUT_PATH``` and `EX_VALUES` are really complementary. We suggest using either one or the other, namely if you do a cut with EX_VALUES you should put `E_THETA_CUT_PATH` to `none/` or something of the sort to make sure some gates are not found by accident.
- All gates create A LOT of histograms. Be really careful with memory management, therefore
    - do not create too many gates at the same time;
    - use --only_enabled_histos and comment out all histograms that you do not need. Usually, you do not need the spectra of AGATA alone, so you can set the key `ENABLED_HISTOS` to NO in `AGATA_CONF`. At the same time, usually you either care about the ejectile, or the partner, and not both at the same time. So you can comment out the histograms of the other to save memory that maybe will allow you to create more gates;
    - be careful that if you use --only_enabled_histos and comment out h_E (for example), no histogram with the name h_E will be printed. This includes all histograms not only in the Agata/ folder, but also on the Spider/ folder and relative subfolders. 
    - right now (but it's going to change) in `AGATA_CONF` you find two keywords `BIN_WIDTH` and `GAMMA_E_MAX`, for the energy axis of AGATA histograms. If you are only interested in gamma-ray energies below 2 MeV, there is no reason why you should leave them to the default 4. MeV and 4000, but you should change them to 2. MeV and 2000. This will save you a LOT of memory on histogram sizes;
    - please do not use `ENABLE_TREE` unless absolutely necessary
    - if you do not want a gate, ***COMMENT IT OUT***. ```GAMMA_GATE``` and ```EX_VALUES``` will always Instantiate all their histograms, if found in the configuration file, even if they're at default values. There is no security check implemented that prevents a gate from being Instantiated.


Last updated on 27/06/2025.