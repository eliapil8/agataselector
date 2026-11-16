#define AgaSelector_cxx
// The class definition in AgaSelector.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.


// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// root> T->Process("AgaSelector.C")
// root> T->Process("AgaSelector.C","some options")
// root> T->Process("AgaSelector.C+")
//


#include "AgaSelector.h"
#include <TH2.h>
#include <TStyle.h>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TF2.h>

#include <TH1.h>
#include <TF1.h>
#include <TColor.h>
#include <TFile.h>
#include <TBrowser.h>
#include <TMath.h>
#include <TCutG.h>
#include <cmath>

#include <iostream>
#include <fstream>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

void AgaSelector::Begin(TTree * /*tree*/)
{
	// The Begin() function is called at the start of the query.
	// When running with PROOF Begin() is only called on the client.
	// The tree argument is deprecated (on PROOF 0 is passed).


	cout << endl; 
	cin >> opt;
        cin >> run_nr;
	cout << "Great choice." << endl;
	cout << "You have selected: " << opt << endl;
	cout << "Wait a minute gentilissim@.. Go for a coffe ;)" << endl;
	cout << endl;


	if(opt=="0"){

		//Defining histograms

		//AGATA hits
		hAgata_hits = new TH2F("hAgata_hits","hAgata_hits",400,-400,400,400,-400,400);
		fOutput->Add(hAgata_hits);

		//Energy vs ID
		hEn_vs_Id = new TH2F("hEn_vs_Id","Energy vs ID",40,0,40,2500,0,2500);
		fOutput->Add(hEn_vs_Id);

		//Time difference T0
		hTime_diff_T0 = new TH1F("hTime_diff_T0","Time difference T0",200,-200,200);
		fOutput->Add(hTime_diff_T0);

		//Time difference TS
		hTime_diff_TS = new TH1F("hTime_diff_TS","Time difference TS",200,-200,200);
		fOutput->Add(hTime_diff_TS);

		//PSA hits XY 
		for(UInt_t cryst_id=0; cryst_id<42; cryst_id++) {
			//cout << "Creating histo for channel: " << channel[ch] <<endl;
			hPSA_hits_XY[cryst_id] = new TH2F(Form("PSA_hits_XY_%d",cryst_id),
					Form("PSA_hits_XY_%d",cryst_id),
					50,-50,50,
					50,-50,50);
			hPSA_hits_XY[cryst_id]->SetXTitle("X [mm]");
			hPSA_hits_XY[cryst_id]->SetYTitle("Y [mm]");
			fOutput->Add(hPSA_hits_XY[cryst_id]);
		}

		//PSA hits XZ
		for(UInt_t cryst_id=0; cryst_id<42; cryst_id++) {
			//cout << "Creating histo for channel: " << channel[ch] <<endl;
			hPSA_hits_XZ[cryst_id] = new TH2F(Form("PSA_hits_XZ_%d",cryst_id),
					Form("PSA_hits_XZ_%d",cryst_id),
					50,-50,50,
					55,-10,100);
			hPSA_hits_XZ[cryst_id]->SetXTitle("X [mm]");
			hPSA_hits_XZ[cryst_id]->SetYTitle("Z [mm]");
			fOutput->Add(hPSA_hits_XZ[cryst_id]);
		}


		//GSing= new TH1F("GSing","Gamma spectrum in singles",8000,0,2000);
		//GGMatT0= new TH2F("GGMatT0","Gamma-gamma matrix using T0",8000,0,2000,8000,0,2000);
		//GGMatTS= new TH2F("GGMatTS","Gamma-gamma matrix using TS",8000,0,2000,8000,0,2000);
		//T0diff= new TH1F("T0diff","Timediff T0",100,-200,200);
		//TSdiff= new TH1F("TSdiff","Timediff TS",100,-200,200);

	}else if(opt=="1"){

		for(UInt_t cryst_id=0; cryst_id<42; cryst_id++) {                   
			//cout << "Creating histo for channel: " << channel[ch] <<endl;   
			E_vs_Ts[cryst_id] = new TH2F(Form("E_vs_Ts_%d",cryst_id),
					Form("E_vs_Ts_%d",cryst_id),    
					10000, 0 ,100,                           
					4000,0,4000);                        
			E_vs_Ts[cryst_id]->SetXTitle("Time [hr]");                      
			E_vs_Ts[cryst_id]->SetYTitle("E [keV]");                      
			fOutput->Add(E_vs_Ts[cryst_id]);                             
		}                                                                   



	}else if(opt=="2"){
		for(UInt_t sgm_id=0; sgm_id<36; sgm_id++) {                      
			//cout << "Creating histo for channel: " << channel[ch] <<endl;
			Esgm_vs_Ts[sgm_id] = new TH2F(Form("Esgm_vs_Ts_%d",sgm_id),      
					Form("Esgm_vs_Ts_%d",sgm_id),                   
					10000, 0 ,100,                                  
					4000,0,4000);                                  
			Esgm_vs_Ts[sgm_id]->SetXTitle("Time [hr]");                     
			Esgm_vs_Ts[sgm_id]->SetYTitle("E [keV]");                       
			fOutput->Add(Esgm_vs_Ts[sgm_id]);                               
		}                                                                      

		
		cout << "Choose the crystal ID: " << endl;   
		cout << endl;                                
		cin >> cryID;                                
		cout << endl;                                


	}

	TString option = GetOption();
}

void AgaSelector::SlaveBegin(TTree * /*tree*/)
{
	// The SlaveBegin() function is called after the Begin() function.
	// When running with PROOF SlaveBegin() is called on each slave server.
	// The tree argument is deprecated (on PROOF 0 is passed).

	TString option = GetOption();

}

Bool_t AgaSelector::Process(Long64_t entry)
{
	// The Process() function is called for each entry in the tree (or possibly
	// keyed object in the case of PROOF) to be processed. The entry argument
	// specifies which entry in the currently loaded tree is to be processed.
	// When processing keyed objects with PROOF, the object is already loaded
	// and is available via the fObject pointer.
	//
	// This function should contain the \"body\" of the analysis. It can contain
	// simple or elaborate selection criteria, run algorithms on the data
	// of the event and typically fill histograms.
	//
	// The processing can be stopped by calling Abort().
	//
	// Use fStatus to set the return value of TTree::Process().
	//
	// The return value is currently not used.

	fReader.SetLocalEntry(entry); 

        //Tree counter
        //tot=fReader.GetEntries();
        //cout << "Total entries: " << tot <<endl;
        //cout << endl;
         
        if(entry==0) {
           tree_counter++;
        }

	if(opt=="0"){    
		for(Int_t i = 0; i < *nbCores; i++) {

			//Agata hits map
			hAgata_hits->Fill(hitGY[i],-1*hitGX[i]);

			//Energy vs ID
			hEn_vs_Id->Fill(coreId[i],coreE0[i]);

			//PSA Hits
			Int_t chan = coreId[i]; 
			//std::cout << "Channel: " << chan <<endl;
			hPSA_hits_XY[chan]->Fill(hitX[i],hitY[i]);
			hPSA_hits_XZ[chan]->Fill(hitX[i],hitZ[i]);

			//Fill the time difference
			for(Int_t j=0; j < *nbCores; j++) {
				if(i!=j) {
					hTime_diff_T0->Fill(coreT0[i]-coreT0[j]);
					hTime_diff_TS->Fill(long(coreTS[i])-long(coreTS[j]));
				}
			}

		}
	}else if(opt=="1"){


		for(Int_t i = 0; i < *nbCores; i++) {  		

			Int_t chan = coreId[i];
			E_vs_Ts[chan]->Fill(coreTS[i]/1e8/3600,coreE0[i]);
		}
	}else if(opt=="2"){

		for(Int_t j = 0; j < *nbHits; j++) {
			if(hitId[j]==cryID){                  
				Int_t chan = hitSg[j];                      
				Esgm_vs_Ts[chan]->Fill(*TSHit/1e8/3600,hitE[j]);

		}
	}

}

		
	std::cout<<'\r'<< "Trees processed: "<< tree_counter << " "<< " Entries processed:"<<entry/1000<<"k"<< std::flush;
	return kTRUE;
}

void AgaSelector::SlaveTerminate()
{
	// The SlaveTerminate() function is called after all entries or objects
	// have been processed. When running with PROOF SlaveTerminate() is called
	// on each slave server.
}

void AgaSelector::Terminate()
{
	// The Terminate() function is the last function to be called during
	// a query. It always runs on the client, it can be used to present
	// the results graphically or save the results to file.

	std::cout<<endl;
	std::cout<<"Writting histograms"<<endl;
	if(opt=="0"){    
		//Drawing results in canvas "PSA HITS XY"
		TCanvas *PSA_hits_XY = new TCanvas("PSA_hits_XY","PSA_hits_XY",700,700);
		fOutput->Add(PSA_hits_XY);
		PSA_hits_XY->Divide(6,6);
		for(Int_t j=0; j<42; j++) {
			PSA_hits_XY->cd(j+1);
			hPSA_hits_XY[j]->Draw("colz");
		}

		//Drawing results in canvas "PSA HITS XZ"
		TCanvas *PSA_hits_XZ = new TCanvas("PSA_hits_XZ","PSA_hits_XZ",700,700);
		fOutput->Add(PSA_hits_XZ);
		PSA_hits_XZ->Divide(6,6);
		for(Int_t j=0; j<42; j++) {
			PSA_hits_XZ->cd(j+1);
			hPSA_hits_XZ[j]->Draw("colz");
		}

//Creating the root file                                           
TFile* outputHists = new TFile(Form("PSA_AGATA_run_%i.root",run_nr), "RECREATE"); 


	}else if(opt=="1"){
	//Creating the root file
	TFile* outputHists = new TFile(Form("EcorevsTS_run_%i.root",run_nr), "RECREATE");
        }else if(opt=="2"){
        //Creating the root file                                           
        TFile* outputHists = new TFile(Form("EsgmvsTS_Id_%i_run_%i.root",cryID,run_nr), "RECREATE"); 
        }
	//Writing the files
	fOutput->Write();

}
