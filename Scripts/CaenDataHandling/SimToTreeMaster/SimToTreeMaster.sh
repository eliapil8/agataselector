#!/bin/bash

./exec/SimToTreeMaster ../SimuData/tracked_energies_0000 TreeMaster_Simu_0000.root
./exec/SimToTreeMaster ../SimuData/tracked_energies_0001 TreeMaster_Simu_0001.root
./exec/SimToTreeMaster ../SimuData/tracked_energies_0002 TreeMaster_Simu_0002.root
./exec/SimToTreeMaster ../SimuData/tracked_energies_0003 TreeMaster_Simu_0003.root
./exec/SimToTreeMaster ../SimuData/tracked_energies_0004 TreeMaster_Simu_0004.root

hadd -f ../Runs/TreeMaster_Simu_0000.root TreeMaster_Simu_*.root
rm -v TreeMaster_Simu_*.root
