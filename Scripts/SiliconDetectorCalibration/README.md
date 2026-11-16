# Simple script to calibrate Silicon detectors
## NB. Still to improve

# create_histo()

This function call the hmaker.cxx to create 2D histograms of Channel vs ADC from CoMPASS file
for each board. The number of boards is defined at the beginning as global variable...

# fitAll()

Loop on all the channels of a selected board to fit the peaks of the source. By defauls 3 peaks of alpha source are used. Those can be modified by the users (num_peaks and energy[]). 

# fitSimple

This is the function called by fitAll. It searches the peaks and fit them with gaussian. The output are three text files with ADC centroids (file0), calibration parameters p0 and p1 (file1) and a file with ADC resolution (file2). Moreover it create a ROOT file with all the fitted spectra. 
It outputs on the terminal the status of the fit. Whenever the code find a number of peaks != num_peaks (defined by the user) a warning message in red is outputed. To fit only those channels the peak_finder() can be used

# peak_fined()

To fit a desired channel in a desired board. You can change the sigma and threshold of the TSpectrum and the ranges of the fitted Gaussian to optimize your fit. If you're happy it will append the new calibration parameters (and centroids and resolutions) to the text files. 

## How to run 

$ .L macro_name.C
$ create_histo("tree_file_name.root")
$ fitAll("histofilename.root",board_num,[additional paramters])
