#!/usr/bin/python

"""
Replace one column of a multi-column ascii file (file1) with one column from another file (file2).
Result listed to the terminal and optionally written to file3, given with -o file3 or -i (inplace).
Comments (lines starting with #) in file1 are passed unchaged to the output; are ignored in file2.
Comments starting later in the lines of both files are not handled and are likely to spoil things.
Normally used to update the energy or time calibration columns of PreprocessingFilterPSA.conf, e.g.

calupdate.py -i -c 4 5 PreprocessingFilterPSA.conf outputFromRecal.txt

 column 4 in PreprocessingFilterPSA.conf has the energy gain coefficients; it should be replaced by
 column 5 in outputFromRecal.txt that contains the scaled gains calculated by Recal
 -c 4 5 is the default and can be omitted from the command line
 -i (inplace) tells to overwrite file1
 
 Dino Bazzacco   ~November 2012
"""

import sys
import numpy as np
import array

# parameters to be obtained from command line
fileConf = '' # file to be modified
fileEcal = '' # file to take the data from
fileOut  = '' # output file, same as fileConf if -i
colConf = 4   # default 4
colEcal = 5   # default 5

def getParameters():
    """
    decode command line arguments
    """

    global fileConf, fileEcal, fileOut
    global colConf, colEcal

    try:
        import argparse
    
        parser = argparse.ArgumentParser()
        
        parser.add_argument("fileConf", help="file with the column to be replaced", metavar="file1")
        parser.add_argument("fileEcal", help="file with the replacement column   ", metavar="file2")
        parser.add_argument("-c", dest="columns", type=int, nargs=2, default=(4,5), help="replaced and replacement columns", metavar="n")
        parser.add_argument("-i", dest="inplace", action="store_true", default=False, help="output overwrites original file")
        parser.add_argument("-o", dest="fileOut", help="output written to this file", metavar="file3")
        
        options = parser.parse_args()
        #print "\n", options, "\n"
        
        fileConf = options.fileConf
        fileEcal = options.fileEcal
        colConf, colEcal = options.columns
        if options.inplace:
           fileOut = options.fileConf
        elif options.fileOut is not None:
           fileOut = options.fileOut
    
    except ImportError:
        # use getopt, which should always be present
        print "\nCould not import 'argparse' to decode command line arguments ==> will do it with getopt:"
        print "options first, 2 positional arguments at the end; use ',' to separate the 2 values of -c" 
        import getopt
        opts,args = getopt.getopt(sys.argv[1:],"hic:o:")
        #print "\n", opts, "\n", args
        if "-h" in opts or len(args) != 2:
            print "\n[-h] [-i] [-o fileOutput] [-c c1,c2] fileToModify fileToTakeColumnFrom"
            exit()
        fileConf = args[0]
        fileEcal = args[1]
        for o,v in opts:
            if o == "-i":
               fileOut = fileConf
            elif o == "-o":
               fileOut = v
            elif o == "-c":
               colConf,colEcal = int(v.split(",")[0]), int(v.split(",")[1])

    print
    print "column %2i of file %s  replaced with" % (colConf, fileConf)
    print "column %2i of file %s"                % (colEcal, fileEcal)
    if len(fileOut) > 1:
       print "result written to", fileOut
    print

def doIt():
    global dConf  # for inspection if run interactively
    # read file to be modified
    f = open(fileConf)
    if f.closed:
        print "Error opening ", fileConf
        exit()
    dConf = f.readlines()
    f.close()

    # read file wherefrom to take the new column
    f = open(fileEcal)
    if f.closed:
        print "Error opening ", fileConf
        exit()
    dEcal = f.readlines()
    f.close()

    # determine largest number of columns in file to be modified, excluding comment lines
    colNum = 0
    for nn in range(len(dConf)):
        if dConf[nn].split()[0][0] != "#":
            colNum = max(colNum, len(dConf[nn].split()))
    #determine the largest width for each of the columns, excluding comment lines
    colWidth = array.array('i')
    for nn in range(colNum): colWidth.append(0)
    for nn in range(len(dConf)):
        if dConf[nn].split()[0][0] != "#":
            ss1 = dConf[nn].split()
            for ii in range(len(ss1)):
                colWidth[ii] = max(colWidth[ii], len(ss1[ii]))

    # whether to write the results on the output file
    writeResults = False
    if len(fileOut) > 1 :
        fOut = open(fileOut, "w")
        if fOut.closed:
           print "Error opening ", fileOut
           exit()
        writeResults = True
    
    # replace colConf in fileConf with colEcal from fileEcal and form result
    changed = 0
    indEcal = 0
    found = False
    valid = False
    for nn in range(len(dConf)):
        ss1 = dConf[nn].split()
        if ss1[0][0] == '#':                             # comment lines passed unchanged
           if dConf[nn][-1]=='\n': mm = dConf[nn][:-1]   # removing the \n at the end
           else: mm = dConf[nn]
        else:
            found = False
            valid = False
            while indEcal < len(dEcal):
                ss2 = dEcal[indEcal].split()
                indEcal += 1
                if ss2[0][0] != "#":
                     found = True
                     break
            if found and (len(ss1) > colConf) and (len(ss2) > colEcal) :
                ss1[colConf] = ss2[colEcal]
                valid = True
                changed += 1
            for ii in range(len(ss1)):                   # prepend blanks to have same width     
                if len(ss1[ii]) < colWidth[ii]:
                    ess1 = ss1[ii]
                    for jj in range(colWidth[ii]-len(ss1[ii])): ess1 = ' '+ess1
                    ss1[ii] = ess1
            mm = '   '.join(map(str,ss1))
        if writeResults : fOut.write(mm+'\n')            # write to result file
        if valid : print mm+ "    *"                     # mark modified columns     
        else:      print mm 
    
    if writeResults: fOut.close()
    print "\n", changed , "lines changed"

###############################################################################################
###############################################################################################

if __name__ == "__main__":
    getParameters()
    doIt()
  
    

