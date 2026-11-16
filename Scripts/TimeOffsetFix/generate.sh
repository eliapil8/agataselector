#ListFrames -f /ancillarydisks/AGATAD_P2_EXP_003/run_0022_23-10-2022_15h46m31s/ancillaryBU_i510_0022_0000.adf -n 1000000 | grep ca020100 | awk '{print $6}' | grep -v \# > anc.txt
ListFrames -f spider_0000.adf -n 1000000 | grep ca020100 | awk '{print $6}' | grep -v \# > anc.txt
ListFrames -f /agatadisks/AGATAD_P2_EXP_003/run_0022_23-10-2022_15h46m31s/Data/01A/psa_0000.adf -n 1000000 | grep fa010102 | awk '{print $6}' | grep -v \# > agata.txt
