#!/bin/bash

root -l << EOF
	TChain* ch = new TChain("TreeMaster");
	ch->Add("Data/run_$1/Out/Analysis/Tree_*.root");
	TFile file("redTree_run$1.root","recreate");
	TTree* tree = ch->CopyTree("nbTrack==1 && nbSAURON==2 && trackE[0]>4000 && SAURONEnergy[0] && SAURONEnergy[1]");
	tree->Write();
	file.Close();
EOF
