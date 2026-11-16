#!/usr/bin/env python3

import os
import glob
import shutil
import sys
import argparse
import time 
import re
import errno

from threading import Thread
from collections import OrderedDict

class Conf(object):
    AGATAalone = True
    det = ""
    exp = ""
    topo = ""
    replaypattern = ""
    onlinereplaypattern = ""
    Prismafilepattern = ""
    Caenfilepattern_BU = ""
    Caenfilepattern_RU = ""
    Prismafilepattern_RU = ""
    Ancillaryfilepattern_BU = ""
    PrismaFiltersExec = ""
    conflist = ""
    fixscript = ""
    fixOpt = ""
    genconfdict = OrderedDict()
    ancconfdict = OrderedDict()

    def __init__(self, fileName):
        with open(fileName, 'r') as f:
            line = f.readline()
            while line:
                if line.startswith("det"):
                    self.det = line.lstrip("det")
                    self.det = self.det.strip()
                    self.AGATAalone = False
                if line.startswith("exp"):
                    self.exp = line.lstrip("exp")
                    self.exp = self.exp.strip()
                if line.startswith("topo"):
                    self.topo = line.lstrip("topo")
                    self.topo = self.topo.strip()
                if line.startswith("replaypattern"):
                    self.replaypattern = line.lstrip("replaypattern")
                    self.replaypattern = self.replaypattern.strip()
                if line.startswith("onlinereplaypattern"):
                    self.onlinereplaypattern = line.lstrip("onlinereplaypattern")
                    self.onlinereplaypattern = self.onlinereplaypattern.strip()
                if line.startswith("Caenfilepattern_RU"):
                    self.Caenfilepattern_RU = line.lstrip("Caenfilepattern_RU")
                    self.Caenfilepattern_RU = self.Caenfilepattern_RU.strip()
                    self.AGATAalone = False
                if line.startswith("Prismafilepattern_RU"):
                    self.Prismafilepattern_RU = line.lstrip("Prismafilepattern_RU")
                    self.Prismafilepattern_RU = self.Prismafilepattern_RU.strip()
                    self.AGATAalone = False
                if line.startswith("Caenfilepattern_BU"):
                    self.Caenfilepattern_BU = line.lstrip("Caenfilepattern_BU")
                    self.Caenfilepattern_BU = self.Caenfilepattern_BU.strip()
                    self.AGATAalone = False
                if line.startswith("Prismafilepattern "):
                    self.Prismafilepattern = line.lstrip("Prismafilepattern")
                    self.Prismafilepattern = self.Prismafilepattern.strip()
                    self.AGATAalone = False
                if line.startswith("Ancillaryfilepattern_BU"):
                    self.Ancillaryfilepattern_BU = line.lstrip("Ancillaryfilepattern_BU")
                    self.Ancillaryfilepattern_BU = self.Ancillaryfilepattern_BU.strip()
                    self.AGATAalone = False
                if line.startswith("PrismaFiltersExec"):
                    self.PrismaFiltersExec = line.lstrip("PrismaFiltersExec")
                    self.PrismaFiltersExec = self.PrismaFiltersExec.strip()
                    self.AGATAalone = False
                if line.startswith("conflist"):
                    self.conflist = line.lstrip("conflist")
                    self.conflist = self.conflist.strip()
                    self.readconflist()
                if line.startswith("fixscript"):
                    self.fixscript = line.lstrip("fixscript")
                    self.fixscript = self.fixscript.strip()
                if line.startswith("fixOpt"):
                    self.fixOpt = line.lstrip("fixOpt")
                    self.fixOpt = self.fixOpt.strip()
                
                line = f.readline()


    def readconflist(self):
        with open(self.conflist, 'r') as f:
            line = f.readline()
            while line:
                try:
                    runnr = int(line.split()[0])
                    genfileName = line.split()[1]
                    ancfileName = ""
                    if self.Prismafilepattern != "" or self.Ancillaryfilepattern_BU != "" or self.Prismafilepattern_RU != "":
                        ancfileName = line.split()[2]
                    self.genconfdict[runnr] = genfileName
                    if ancfileName != "":
                        self.ancconfdict[runnr] = ancfileName
                    elif self.Prismafilepattern != "" or self.Ancillaryfilepattern_BU != "" or self.Prismafilepattern_RU != "":
                        raise Exception("Prisma is defined in .conf but configuration files are missing in conf_list.")
                except ValueError:
                    pass
                line = f.readline()
        print(self.Ancillaryfilepattern_BU)
        print(self.Prismafilepattern)
        print(self.Prismafilepattern_RU)
        self.genconfdict = OrderedDict(sorted(self.genconfdict.items()))
        if self.Prismafilepattern != "" or self.Ancillaryfilepattern_BU != "" or self.Prismafilepattern_RU != "":
            self.ancconfdict = OrderedDict(sorted(self.ancconfdict.items()))

    def getgenconf(self, run):
        goodgenconf = next(iter(self.genconfdict.values()))
        for key in self.genconfdict:
            if key <= run:
                goodgenconf = self.genconfdict[key]
            else:
                return goodgenconf
        return goodgenconf
        
    def getancconf(self, run):
        goodancconf = next(iter(self.ancconfdict.values()))
        for key in self.ancconfdict:
            print(key)
            if key <= run:
                goodancconf = self.ancconfdict[key]
            else:
                return goodancconf
        return goodancconf

######Matches the run name with the data found in the data directory
#def GetDataDirectories(run):
#    directories = {}
#    for name in glob.glob(data_dir+'/run_*'):
#        if 'run_'+run in name:
#            directories[run]=name
#            print('Found run :', 'run_'+run, ' in directory : ', name)
#    if len(directories)!=len(runs):
#        print("NOT perfect match between provided runs and data found")
#    return directories``

def makeReplay(runNr, opts):

    conf = Conf(opts.conf)
    agataDataPath = "/agatadisks/AGATAD_P2_{:s}".format(conf.exp) 
    if not conf.AGATAalone:
       ancillaryDataPath = "/ancillarydisks/AGATAD_P2_{:s}".format(conf.exp) 
    else:
       ancillaryDataPath = ""
    
    def runFemul(opts):
        os.system("./gen_conf.py")
        turns = 0
        if opts.nturns:
            turns = opts.nturns
            
        def run():
           os.system("femul {:s} -turns {:d} | tee log_{:s}_run_{:s}.txt".format(conf.topo, turns,conf.exp,str(runNr).zfill(4)))

        runThread = Thread(target=run)
        runThread.setDaemon(True)
        runThread.start()
        
        if opts.online:
            while True:
                if(os.path.exists("./Data/ClosedRun")):
                    os.system("touch StopFile")
                    #kill run thread here
                    return
                else:
                    time.sleep(1)

        runThread.join()


    if not opts.online:
        runFolder = "{:s}{:s}".format(conf.replaypattern, str(runNr).zfill(4))
    else:
        runFolder = "{:s}{:s}".format(conf.onlinereplaypattern, str(runNr).zfill(4))
        
    print("\033[92mI am going to create replay: {:s}".format(runFolder))
    print("Are you happy with this?")
    check = input("(Y/N) to continue/abort...\033[0m")
    print(check)
    if(check.upper() == 'N'):
        return

    if os.path.isdir("./{:s}".format(runFolder)):
        if opts.delete_previous:
            os.system("rm -rf {:s}".format(runFolder))
        else:
            raise Exception("\033[31mReplay is present and I cannot delete it, specify --delete_previous in case\033[0m")
    
    os.mkdir(runFolder)
    os.chdir(runFolder)
    print("Created folder : {:s}".format(runFolder))
    print("Linking agata ...")
 
    #link AGATA data
    if not opts.online:
        agataRunPath = ""
        names = glob.glob("{:s}/run_{:s}*".format(agataDataPath,str(runNr).zfill(4)))
        print("Looking for selected run in: {:s}".format("{:s}/run_{:s}*".format(agataDataPath,str(runNr).zfill(4))))
        if len(names) == 0:
            raise Exception("Agata run not found")
        elif len(names) > 1:
            if opts.use_latest_run:
                agataRunPath = max(names, key=os.path.getctime)
            else:
                raise Exception("Multiple Agata runs with the same number")
        else:
            agataRunPath = names[0]
        os.system("ln -s {:s}/Data".format(agataRunPath))
    else:
        agataRunPath = "{:s}/current_run/Conf".format(agataDataPath)
        os.system("ln -s {:s}/current_run/Data".format(agataDataPath))
    
    print("Coping configurations...")
    #if opts.online:
    #    os.system("cp -rv /opt/data/LNL/EXP_031/Replay/Template/Conf .")
    #else:
    os.system("cp -rv {:s}/Conf .".format(agataRunPath))
    os.system("cp -rv ../Template/Conf/* Conf/.")
    
    if not conf.AGATAalone:
        os.system("cp -rv ../Template/Conf/* Conf/.".format(agataRunPath))
        os.system("cp -rv ../Template/{:s} gen_conf.py".format(conf.getgenconf(runNr)))
        os.system("cp -rv ../Template/{:s} .".format(conf.topo))

        ancillaryConfFolder = "Conf/{:s}/".format(conf.det)
        if not os.path.exists(ancillaryConfFolder):
            os.makedirs(ancillaryConfFolder)

        if conf.Prismafilepattern != "" or conf.Ancillaryfilepattern_BU != "" or conf.Prismafilepattern_RU != "":
            os.system("cp -rv {:s}/* {:s}".format(conf.getancconf(runNr), ancillaryConfFolder))

    else:
        os.system("cp -rv ../Template_noAnc/Conf/* Conf/.".format(agataRunPath))
        os.system("cp -rv ../Template_noAnc/{:s} gen_conf.py".format(conf.getgenconf(runNr)))
        os.system("cp -rv ../Template_noAnc/{:s} .".format(conf.topo))
    #os.system("cp -v Template/gen_conf.py {:i}/".format(runFolder))
    #print("...gen_con.py from Template copied in {:s}".format(runFolder))
    #os.system("cp -rv {:s}/zWorkingDir/Conf {:s}/".format(agataDataPath, runFolder))
    #print("...Conf from zWorkingDir copied in {:s}".format(runFolder))
    #os.system("cp -rv Template/PrismaConf/* {:s}/Conf/prisma/".format(runFolder))        
    #print("...PrismaConf from Templare copied in {:s}/Conf/prisma".format(runFolder))

    
    # run fix script and link ANCILLARY data
    if not conf.AGATAalone:

        #Get the folder with the right run number
        ancillaryRunPath = ""
        names = glob.glob("{:s}/run_{:s}*".format(ancillaryDataPath,str(runNr).zfill(4)))
        if len(names) == 0:
            raise Exception("Ancillary run not found")
        elif len(names) > 1:
            print(names)
            if opts.use_latest_run:
                ancillaryRunPath = max(names, key=os.path.getctime)
            else:
                raise Exception("Multiple Ancillary runs with the same number")
        else:
            ancillaryRunPath = names[0]

        prismaFileNames = ""
        prismaFileRUNames = ""
        caenRuFileNames = ""
        ancillaryCaenRuFileNames = {}
        ancillaryPrismaFileName = {}
        ancillaryPrismaRUFileName = {}
        ancillaryCaenFileName = {}
        ancillaryMergedFileName = {}

       #print(conf.Prismafilepattern)

        # PRISMA RU files load
        if conf.Prismafilepattern_RU != "":
            prismaFiles = glob.glob("{}/Data/prisma/{}*".format(ancillaryRunPath, conf.Prismafilepattern_RU))
            if len(prismaFiles) == 0:
                raise Exception("Prisma RU file not found in {}, the folder might be empty or not present!".format(ancillaryRunPath))
            for pFile in prismaFiles:
                splitString = re.split("_|\.|/", pFile)
                #print(splitString)
                fileIdx = splitString[-2]
                fileRunNr = splitString[-3]
                if(int(fileRunNr) != runNr):
                    continue
                else:
                    print("Found prisma RU file {}".format(pFile))
                    ancillaryPrismaRUFileName[int(fileIdx)] = pFile
            for it in ancillaryPrismaRUFileName:
                prismaFileRUNames += "{:s} ".format(ancillaryPrismaRUFileName[it])

        print(conf.Prismafilepattern_RU != "")
        print(conf.Prismafilepattern)
        print(len(conf.Prismafilepattern))

        #This is if the prismafilters need to be rerun
        if conf.Prismafilepattern_RU != "" and conf.Prismafilepattern == "":
            print("Setting up to run prismafilters")
            if not  os.path.exists(conf.PrismaFiltersExec):
                raise FileNotFoundError( errno.ENOENT, os.strerror(errno.ENOENT), conf.PrismaFiltersExec)
            if not  os.path.exists("../../AncMerging/filter_0000.conf"):
                raise FileNotFoundError( errno.ENOENT, os.strerror(errno.ENOENT), " ../../AncMerging/filter_0000.conf")
            # if file already exists and no option for reruning script is provided just link the file
            if os.path.exists("../../AncMerging/run_{:s}_ancWprisma.adf".format(str(runNr).zfill(4))) and not opts.rerun_fix_script:
                print("Merged file already exists. Skipping fixscript and linking existing file.")
                os.system("ln -s ../../AncMerging/run_{:s}_ancWprisma.adf".format(str(runNr).zfill(4)))
            else:
                print("The PrismaFilters are working on conversion to .adf files for prisma, be patient ...")
                for it in ancillaryPrismaRUFileName:
                    tmpPrismaFileName = "tmp_prisma_run_{:s}_{:s}.adf".format(str(runNr).zfill(4),str(it).zfill(4))
                    print("{:s} -cfg ../../AncMerging/filter_0000.conf -if {:s} -ogf -of {:s}".format(conf.PrismaFiltersExec, ancillaryPrismaRUFileName[it], tmpPrismaFileName))
                    os.system("{:s} -cfg ../../AncMerging/filter_0000.conf -if {:s} -ogf -of {:s}".format(conf.PrismaFiltersExec, ancillaryPrismaRUFileName[it], tmpPrismaFileName))
                conf.Prismafilepattern = "tmp_prisma_"

        # PRISMA BU files load
        if conf.Prismafilepattern != "":
            prismaFiles = {}
            if conf.Prismafilepattern == "tmp_prisma_":
                if opts.do_prisma_ana:
                    prismaFiles = glob.glob("tmp_prisma_*.adf_00")
                else:
                    prismaFiles = glob.glob("tmp_prisma_*.adf_01")
            else:
                prismaFiles = glob.glob("{}/{}*".format(ancillaryRunPath, conf.Prismafilepattern))
            if len(prismaFiles) == 0:
                raise Exception("Prisma BU file not found, the folder might be empty or not present!")
            for pFile in prismaFiles:
                splitString = re.split("_|\.|/", pFile)
                #print(splitString)
                fileIdx = ""
                fileRunNr = ""
                if conf.Prismafilepattern == "tmp_prisma_":
                    fileIdx += splitString[-3]
                    fileRunNr += splitString[-4]
                else:
                    fileIdx += splitString[-2]
                    fileRunNr += splitString[-3]
                if(int(fileRunNr) != runNr):
                    continue
                else:
                    ancillaryPrismaFileName[int(fileIdx)] = pFile
            for it in ancillaryPrismaFileName:
                prismaFileNames += "{:s} ".format(ancillaryPrismaFileName[it])

        # Caen BU files load
        if conf.Caenfilepattern_BU != "":
            caenBUFiles = glob.glob("{}/{}*".format(ancillaryRunPath, conf.Caenfilepattern_BU))
            if len(caenBUFiles) == 0:
                raise Exception("Caen BU file not found, the folder might be empty or not present!")
            for cBUFile in caenBUFiles:
                splitString = re.split("_|\.|/", cBUFile)
                #print(splitString)
                fileIdx = splitString[-2]
                fileRunNr = splitString[-3]
                if(int(fileRunNr) != runNr):
                    continue
                else:
                    ancillaryCaenFileName[int(fileIdx)] = cBUFile
                
        # Caen RU files load for linking
        if conf.Caenfilepattern_RU != "":
            caenRUFiles = glob.glob("{}/{}*".format(ancillaryRunPath, conf.Caenfilepattern_RU))
            if len(caenRUFiles) == 0:
                raise Exception("Caen RU file not found in {}, the folder might be empty or not present!".format(ancillaryRunPath))
            for cRUFile in caenRUFiles:
                splitString = re.split("_|\.|/", cRUFile)
                #print(splitString)
                fileIdx = splitString[-2]
                fileRunNr = splitString[-3]
                if(int(fileRunNr) != runNr):
                    continue
                else:
                    ancillaryCaenRuFileNames[int(fileIdx)] = cRUFile
            for it in ancillaryCaenRuFileNames:
                caenRuFileNames += "{:s} ".format(ancillaryCaenRuFileNames[it])

        # Ancillary DAQ merged BU files load for linking
        if conf.Ancillaryfilepattern_BU != "":
            ancilBUFiles = glob.glob("{}/{}*".format(ancillaryRunPath, conf.Ancillaryfilepattern_BU))
            if len(ancilBUFiles) == 0:
                print("{}/{}".format(ancillaryRunPath, conf.Ancillaryfilepattern_BU))
                raise Exception("Ancillary BU file not found, the folder might be empty or not present!")
            for aBUFile in ancilBUFiles:
                splitString = re.split("_|\.|/", aBUFile)
                #print(splitString)
                fileIdx = splitString[-2]
                fileRunNr = splitString[-3]
                if(int(fileRunNr) != runNr):
                    continue
                else:
                    ancillaryMergedFileName[int(fileIdx)] = aBUFile

        if conf.Ancillaryfilepattern_BU != "":
            print("Ancillary merged by DAQ case: linking BU files.")
            for nr in ancillaryMergedFileName:
                os.system("ln -s {:s} {:s}_{:s}.adf".format(ancillaryMergedFileName[nr], conf.det, str(nr).zfill(4)))
        else:
            # if it is online replay do not run fix script
            if opts.online:
                if conf.Prismafilepattern != "" and conf.Caenfilepattern_BU != "":
                    raise Exception("Online replay is set, but more than one ancillary detectors are provided ... Choose only one!")

                elif conf.Caenfilepattern_BU != "":
                    print("Ancillary only case: linking BU files.")
                    for nr in ancillaryCaenFileName:
                        os.system("ln -s {:s} {:s}_{:s}.adf".format(ancillaryCaenFileName[nr], conf.det, str(nr).zfill(4)))

                elif conf.Prismafilepattern != "":
                    print("Prisma only case: linking BU files.")
                    for nr in ancillaryPrismaFileName:
                        os.system("ln -s {:s} {:s}_{:s}.adf".format(ancillaryPrismaFileName[nr], conf.det, str(nr).zfill(4)))

                else:
                    raise Exception("Online replay is set, but ancillary adf files are missing ... Check conf for file pattern!")

            # offline replay case
            else:

                if conf.Prismafilepattern != "" and conf.Caenfilepattern_RU != "":
                    if conf.fixscript != "":
                        if not  os.path.exists(conf.fixscript):
                           raise FileNotFoundError( errno.ENOENT, os.strerror(errno.ENOENT), conf.fixscript)
                        # if file already exists and no option for reruning script is provided just link the file
                        if os.path.exists("../../AncMerging/run_{:s}_ancWprisma.adf".format(str(runNr).zfill(4))) and not opts.rerun_fix_script:
                            print("Merged file already exists. Skipping fixscript and linking existing file.")
                            os.system("ln -s ../../AncMerging/run_{:s}_ancWprisma.adf {:s}_0000.adf".format(str(runNr).zfill(4), conf.det))
                        else:
                            print("The fix script is merging prisma with other ancillary detectors, be patient ...")
                            print("{:s} {:s} {:s} --prisma {:s} ../../AncMerging/run_{:s}_ancWprisma.adf | tee ancMergingLog.txt".format(conf.fixscript, conf.fixOpt, caenRuFileNames, prismaFileNames, str(runNr).zfill(4)))
                            os.system("cp -v ../Template/ReadCaenRaw.set .")
                            os.system("{:s} {:s} {:s} --prisma {:s} ../../AncMerging/run_{:s}_ancWprisma.adf | tee ancMergingLog.txt".format(conf.fixscript, conf.fixOpt, caenRuFileNames, prismaFileNames, str(runNr).zfill(4)))
                            os.system("ln -s ../../AncMerging/run_{:s}_ancWprisma.adf {:s}_0000.adf".format(str(runNr).zfill(4), conf.det))
                            if conf.Prismafilepattern == "tmp_prisma_":
                                tmpFiles = glob.glob("tmp_prisma_*.adf_*")
                                for it in tmpFiles:
                                    os.system("rm -f {:s}".format(it))
                                tmpFiles = glob.glob("PRISMA_*.spec")
                                for it in tmpFiles:
                                    os.system("rm -f {:s}".format(it))

                    else:
                        raise Exception("Prisma and other ancillary is set, but merging (fix) script is not provided in conf.")

                elif conf.Caenfilepattern_RU != "":

                    # Ancillary alone case
                    if conf.fixscript != "":
                        if not  os.path.exists(conf.fixscript):
                           raise FileNotFoundError( errno.ENOENT, os.strerror(errno.ENOENT), conf.fixscript)
                        # if file already exists and no option for reruning script is provided just link the file
                        if os.path.exists("../../AncMerging/run_{:s}_anc.adf".format(str(runNr).zfill(4))) and not opts.rerun_fix_script:
                            print("Ancillary only case: Merged file already exists. Skipping fixscript and linking existing file.")
                            os.system("ln -s ../../AncMerging/run_{:s}_anc.adf {:s}_0000.adf".format(str(runNr).zfill(4), conf.det))
                        else:
                            print("The fix script is running without prisma, be patient ...")
                            os.system("cp -v ../Template/ReadCaenRaw.set .")
                            os.system("{:s} {:s} {:s} ../../AncMerging/run_{:s}_anc.adf | tee ancMergingLog.txt".format(conf.fixscript, conf.fixOpt, caenRuFileNames, str(runNr).zfill(4)))
                            os.system("ln -s ../../AncMerging/run_{:s}_anc.adf {:s}_0000.adf".format(str(runNr).zfill(4), conf.det))

                elif conf.Prismafilepattern != "":
                    
                    # PRISMA alone case
                    print("Prisma only case: linking BU files.")
                    for nr in ancillaryPrismaFileName:
                        os.system("ln -s {:s} {:s}_{:s}.adf".format(ancillaryPrismaFileName[nr], conf.det, str(nr).zfill(4)))

                # if ancillary RU nor Prisma is defined, try if ancillary BU are defined and link them
                elif conf.Caenfilepattern_BU != "":
                    print("Ancillary only case: linking BU files.")
                    for nr in ancillaryCaenFileName:
                        os.system("ln -s {:s} {:s}_{:s}.adf".format(ancillaryCaenFileName[nr], conf.det, str(nr).zfill(4)))

                else:
                    raise Exception("Nothing to do ... Check conf for file patterns and fix script!")
    print("Config of folder done. Starting femul ... ")
    runFemul(opts)

def main():
    #####Setting up the parser
    parser = argparse.ArgumentParser(description='Script to run replays in Runs.txt')
    parser.add_argument('--run',
            type=int,
            nargs='+',
            dest='runs',
            required=True,
            help='run number(s) to replay')

    parser.add_argument('--conf',
            type=str,
            dest='conf',
            required=False,
            default="replay.conf",
            help='Specify configuration file')

    parser.add_argument('--delete_previous',
            action='store_true',
            dest='delete_previous',
            help='if a replay is already present, remove it and create a new one')

    parser.add_argument('--use_previous',
            action='store_true',
            dest='use_previous',
            help='Use previous replay folder')

    parser.add_argument('--use_latest_run',
            action='store_true',
            dest='use_latest_run',
            help='When multiple runs with the same nr are present replay the latest')

    parser.add_argument('--online',
            action='store_true',
            dest='online',
            help='Replay the online data while the acquisition is running')

    parser.add_argument('--rerun_fix_script',
            action='store_true',
            dest='rerun_fix_script',
            help='Deletes previous fix script output if present, otherwise it is kept')

    parser.add_argument('--do_prisma_ana',
            action='store_true',
            dest='do_prisma_ana',
            help='Redo prisma ana if you are running the prisma filters')

    #parser.add_argument('--AGATAalone',
    #        action='store_true',
    #        dest='AGATAalone',
    #        help='Replay using AGATA without ancillary')


    parser.add_argument('--nturns',
            type=int,
            dest='nturns',
            help='Number of femul turns')


    args = parser.parse_args()


    for run in args.runs:
        try: 
            makeReplay(run, args)
        except Exception as exception:
            print(exception)

if __name__ == '__main__':
    main()
