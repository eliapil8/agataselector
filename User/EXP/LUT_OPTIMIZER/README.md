# Example of optimization

## Global optimization

Once one is happy with the agata replay, calibrations, ..., it is possible to optimize the data.

### Step 1

It is necessary to reduce the data so that the selector can run with high frequency (something like 1 Hz).
In the case of spider one can ask to always have a spider in an event:

```
RunSelector 0 --reduction_cond nbSPIDER\>0 --nrthr 10
```

This will run on run 0, but it is likely necessary to run it on all runs.
This will create reduced files in the data folder. The new files will be called the same as the old ones the _red prefix.
To apply one more condition one can modify the ```IN_FILE_PATTERN Tree_ ```with ```INFILE_PATTERN red_Tree_```. We can now run another condition such as asking always for agata in an event:

```
RunSelector 0 --reduction_cond nbCores\>0 --nrthr 10
```

Modifying once again the ```IN_FILE_PATTERN``` with ```red_red_Tree``` we can apply more conditions such as the time coincidence:

```
RunSelector 0 --reduction_cond coreTS-TSSPIDER\<10 --nrthr 10
RunSelector 0 --reduction_cond coreTS-TSSPIDER\>-10 --nrthr 10
```

Once all conditions are applied one can add all the files together to have a single reduced file with all runs.

### Step 2
Create a new run with this single file.

### Step 3
Find the histograms to optimize. One can optimize multiple ones such as ```h_EDC``` and ```h_EDCBP``` at the same time. 
Add in the ```OPTIMIZER_CONF``` part of the ```selector.conf``` the transitions of interest (as many as you wish). For example:

```
TRANSITION /AgataSpider h_EDC 1332.54 4 0.1 0.5 keV
TRANSITION /AgataSpider h_EDCBP 2623 4 0.1 0.5 keV
```
The meaning of the parameters is: TRANSITION |folder|spec_name|centroid|sigma|tail|bias

You can also speed up the selector by running it with the ```--only_enabled_histos``` option. 
To enable only the histograms of interest you should find the file in ```Conf/enabled_histos.conf```. Remove all histograms except the ones you want (h_EDC, h_EDCBP).

### Step 4
Chose the parameter(s) to optimize and add it to the selector.conf. Multiple parameters can be optimized at the same time.

```
PARAMETER AGATA_CONF Z_SHIFT 0  10 1 mm #Comment
PARAMETER SPIDER_CONF Z_SHIFT 0 -10 10 1 mm #Comment
```

The meaning of the parameters is: PARAMETER |detector|par_name|initial_value|min|max|step|.

### Step 5 
Change the parameters of the minimizer. Things that you can change are the minimizer (Simplex, Migrad, ..).
You can fit a tail, chose the polynomial background order, the maximum number of calls to the selector that che minimizer can do.
You can choose to optimize only the centroid with ```SIGMA_WEIGHT 0``` or the width of the peaks only with ```SIGMA_WEIGHT 1```. Values in between 0 and 1 are valid.

### Step 6 
Run the selector with 

```
RunSelector 0 --optimize --only_enabled_histos --nrthr 1 --debug_canvas
```

This will open up one canvas for each fit. Check if the fit is ok. If not, you can change the parameters of the fit in the file ```Conf/Optimizer/parameters.dat```.
If there is an error such as "```Fitter found 0 Gaussians, try removing the Conf/Optimizer/parameters.dat file" remove the parameters.dat file and run again```.

### Step 7 
Once the fits are ok, run the command without debug canvas

```
RunSelector 0 --optimize --only_enabled_histos --nrthr 1
```

### Step 8
Monitor the file ```Conf/Optimizer/log.txt``` as the optimizer runs. This will tell you the COST (function that you are minimizing) as well as the current value of each parameter (val: ##).
The value printed is in internal units (checkout ```src/Core/Units.h```).

```
tail -f Conf/Optimizer/log.txt
```

### Step 9
At the end of the optimization it will produce a ```selector.conf_optimized``` file with the optimal parameters.

### Step 10 
If you add in the ```OPTIMIZER_CONF``` a ```SCAN``` (same as PARAMETER)

```
SCAN AGATA_CONF Z_SHIFT 0  10 1 mm #Comment
```

The selector will produce an ```Out/out.root``` file. This contains a scan  of the minimized function for each parameter.
Also, a folder with a TH2D with the histograms of interest at each step is present.


## Strip by strip optimization

As an example one might be interested in optimizing the ```Z_SHIFT``` of spider strip by strip. 

### Step 1 
Set up the optimizer like in the previous instructions.

### Step 2 
Compile the selector with:

```
cmake -DEXP_NAME=SPIDER_ANGCAL .
```

This will compile a special user selector (Check it out in ```User/EXP/SPIDER_ANGCAL```). 

### Step 3
Add in the selector.conf a specific ```USER_CONF``` such as:

```
#-----------------------------------------------------------
USER_CONF
HISTO			m_SpiderID_EDC #			Example of configuration value
DETECTOR			SPIDER #			Example of configuration value
IDRANGE	25 32		 #			Range of channel id to minimize
IDRANGE	41 48		 #			Range of channel id to minimize
EXAMPLE 3 mm #commentn
#-----------------------------------------------------------
```

You can also add more ```HISTO``` (s). They should be 2D matrices with the idx of the channel on the x-axis (typically board*16+channel as in the LUT).
```IDRANGE``` is the range of Ids of interest that correspond to the xBin of the 2D matrix (```m_SpiderID_EDC```). 
Running this will add in the User folder multiple histograms for each channel specified in the ```IDRANGE```. 

### Step 4 
The idea is now to run the optimizer on these produced histograms. You can add them as transitions in the ```OPTIMIZER_CONF```:

```
TRANSITION /User h_EDC_25 1332.54 4 0.1 0.5 keV
TRANSITION /User h_EDC_26 1332.54 4 0.1 0.5 keV
TRANSITION /User h_EDC_27 1332.54 4 0.1 0.5 keV
TRANSITION /User h_EDC_28 1332.54 4 0.1 0.5 keV
TRANSITION /User h_EDC_29 1332.54 4 0.1 0.5 keV
TRANSITION /User h_EDC_30 1332.54 4 0.1 0.5 keV
...
```

### Step 5
In this case you are probably more interested in the ```SCAN``` rather than the full minimization. You can then set the ```MAX_CALLS``` to ```1``` and run the optimizer with the appropriate scan.
Run the minimizer with the usual 

```
RunSelector 0 --optimize --only_enabled_histos --nrthr 1
```

### Step 6 
Remove the old ```Conf/Optimizer/parameters.dat```

### Step 7: 
You should find the results in the ```Out/out.root ```