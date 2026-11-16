#
# Author: Elia Pilotto
#

Read this for a list of the programs and their usage:


 - SimToTreeMaster: convert data from simulations to TreeMaster format (with artificial timestamps)
 
 		+ SimToTreeMaster.cxx: source code
 		+ SimToTreeMaster.sh: example of usage
 
 
 - CaenToTreeMaster: perform Event Building and convert data from Caen format to TreeMaster format
 
 		+ CaenToTreeMaster.cxx: main source code, perform event building and time sorting using 
 			channel-by-channel buffer, then write in TreeMaster format. Works well with low counting rate
 			and if input file is already mostly ordered. 
 			
 		+ MergeSortCaen.cxx: source code of merge-sort algorithm. Use merge-sort algorithm to perform efficient 
 			time sorting, even for completely non-ordered data. It works by dividing the data into blocks and sub-blocks. 
 			To make it fast it is very important to choose the correct size of the blocks (maximum that fits in memory).
 			
 		+ TimeSortedCaenToTreeMaster.cxx: analogous to "CaenToTreeMaster.cxx" but skipping time sorting. 
 			Convert from Caen format to TreeMaster format and perform event building on already time sorted data. 
 			To be used only after MergeSortCaen.cxx.
 			
 - CheckSorting: simple selector to check data in Caen format are time sorted.
 
 		+ CheckSorting.h
 		+ CheckSorting.C

