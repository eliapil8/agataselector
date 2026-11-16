#include "UserSelector.h"

template <class T, class UT>
T unsignedDiff(const UT& val1, const UT& val2) {
  UT res{0};
  if(val1 > val2)
    res = val1 - val2;
  else
    res = val2 - val1;
  
  if(res <= std::numeric_limits<T>::max()) {
    if(val1 > val2)
      return static_cast<T>(res);
    else
      return -static_cast<T>(res);
  }

  throw std::runtime_error("Signed to unsigned conversion problem\n");
}


void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

       UserHistograms& h = userHistograms;
       // User begin
       h.dir = outFile.mkdir("User", "User histos");
       h.dir->cd();

       h.m_spiderID_Theta=
			new TH2D("m_spiderID_Theta", "spiderID; EDCBP; Counts",
				 100, -0.5, 99.5, 50, 120, 170);

       h.m_spiderID_Phi=
			new TH2D("m_spiderID_Phi", "spiderID; EDCBP; Counts",
				 100, -0.5, 99.5, 60, -180, 180);

       int i=0;
       for(auto g : conf.user.ex_gates){
	 h.m_spiderID_EDCBP_gg.push_back(
		new TH2D(Form("m_spiderID_EDCBP_g%d", i),
			 Form("SpiderID vs EDCBP, gated between Ex: %.3f - %.3f", g.first, g.second),
			 100, -0.5, 99.5, 4096, 0, 4095));
	 i++;
       }
       
       h.h_Au =
	 new TH1D("h_Au", "Au vs time; Time; Counts",
		  100, 0, 100);

       h.h_Au_up =
	 new TH1D("h_Au_up", "Au vs time; Time; Counts",
		  100, 0, 100);

       h.h_Au_down =
	 new TH1D("h_Au_down", "Au vs time; Time; Counts",
		  100, 0, 100);
       
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);
        UserHistograms& h = userHistograms;
        // User process

	if(firstTS == 0) firstTS = spiderOutput.TS;
	
	double spiderEx, spiderTheta, spiderPhi;

	for(size_t s{0}; s<spiderOutput.E.size(); ++s){
	  spiderEx = spiderOutput.Ex.at(s);
	  spiderTheta = spiderOutput.p3.at(s).Vect().Theta()*TMath::RadToDeg();
	  spiderPhi = spiderOutput.p3.at(s).Vect().Phi()*TMath::RadToDeg();

	  h.m_spiderID_Theta->Fill(spiderOutput.Id.at(s), spiderTheta);
	  h.m_spiderID_Phi->Fill(spiderOutput.Id.at(s), spiderPhi);
	  
	}


	
	for(unsigned int s{0}; s < spiderOutput.E.size(); ++s){
	  spiderEx = spiderOutput.Ex.at(s);
	  
	  for(unsigned int a1{0}; a1 < agataOutput.E.size(); ++a1){
	    
	    if(!gammaAgataSpider.at(s).inCoinc.at(a1)) continue;

	    //loop on the gates
	    for(int gg=0; gg < conf.user.ex_gates.size(); gg++){
	      if(spiderEx > conf.user.ex_gates[gg].first && spiderEx < conf.user.ex_gates[gg].second)
		h.m_spiderID_EDCBP_gg[gg]->Fill(
			spiderOutput.Id[s], gammaAgataSpider.at(s).EDCBP.at(a1).E());
	    }

	    auto off = unsignedDiff<double, unsigned long long>(conf.user.offTS, firstTS);
	      
	    auto d1time{spiderOutput.Time.at(s)};
	    auto d2time{agataOutput.Time.at(a1)};

	    auto diff = unsignedDiff<double, unsigned long long>(agataOutput.TS, spiderOutput.TS) 
	      + (d2time - d1time)/(10.*UNITS::ns) + off/(10.*UNITS::ns);
	      
	    if(gammaAgataSpider.at(s).E.at(a1).E() > 533 && gammaAgataSpider.at(s).E.at(a1).E() < 558)
	      h.h_Au->Fill(diff);

	    if(gammaAgataSpider.at(s).E.at(a1).E() > 475 && gammaAgataSpider.at(s).E.at(a1).E() < 487)
	      h.h_Au_down->Fill(diff);
		    
	    if(gammaAgataSpider.at(s).E.at(a1).E() > 763 && gammaAgataSpider.at(s).E.at(a1).E() < 783)
	      h.h_Au_up->Fill(diff);
	    	    
	    
	  } // close loop on a1
	} //close loop on s
	

	
	/*
	double spiderEx, spiderTheta, spiderPhi;
	double gammaEDCBP;


	double gate1_1 =  1.75 , gate1_2= 1.97;
	double gate2_1 =  0.2 , gate2_2= 0.85;
	double gate3_1 =  2.2 , gate3_2= 2.6;
	double gate4_1 =  2.9 , gate4_2= 3.45;

	
	//singles analysis

	//if(firstTS == 0) firstTS = spiderOutput.TS;

	
	for(size_t s{0}; s< spiderOutput.E.size(); ++s) {
	  spiderEx = spiderOutput.Ex.at(s);
	  spiderTheta = spiderOutput.p3.at(s).Vect().Theta()*TMath::RadToDeg();//angles[spiderOutput.Id.at(s)];//spiderOutput.p3.at(s).Vect().Theta()*TMath::RadToDeg();
	  spiderPhi = spiderOutput.p3.at(s).Vect().Phi()*TMath::RadToDeg();


	  //	  h.m_Ex_time->Fill( (spiderOutput.TS - firstTS)/1.e8/60., spiderEx );

	  if(spiderEx < gate1_2 && spiderEx > gate1_1){
	    h.h_Thetap_e0->Fill(spiderTheta);
	    h.h_ID_e0->Fill(newID[spiderOutput.Id.at(s)]);
	  }
	  else if(spiderEx < gate2_2 && spiderEx > gate2_1) {
	    h.h_Thetap_e646->Fill(spiderTheta);
	    h.h_ID_e646->Fill(newID[spiderOutput.Id.at(s)]);
	  }
	  else if(spiderEx < gate3_2 && spiderEx > gate3_1) {
	    h.h_Thetap_e2638->Fill(spiderTheta);
	    h.h_ID_e2638->Fill(newID[spiderOutput.Id.at(s)]);
	  }
	  else if(spiderEx < gate4_2 && spiderEx > gate4_1){
	    h.h_Thetap_e3262->Fill(spiderTheta);
	    h.h_ID_e3262->Fill(newID[spiderOutput.Id.at(s)]);
	  }

	  h.m_ID_Ex->Fill(newID[spiderOutput.Id.at(s)], spiderEx);
	  h.m_Theta_Ex->Fill(spiderTheta, spiderEx);
	  h.m_Phi_Ex->Fill(spiderPhi, spiderEx);
	  
	  spiderEx = spiderTheta = spiderPhi = 0.;
	  
	} //close loop on spider



	//coincidence analysis

	bool in_coinc = false;
	double diff = -500.;
	
	if(gammaAgataSpider.size()==0 || agataOutput.E.size() ==0 || spiderOutput.E.size()==0)  return kTRUE;
	
	for(size_t d1{0}; d1 < spiderOutput.Time.size(); ++d1) {
	  auto d1time{spiderOutput.Time.at(d1)};
	  for(size_t d2{0}; d2 < agataOutput.Time.size(); ++d2) {
            auto d2time{agataOutput.Time.at(d2)};

	    //	    std::cout << "TS agata: " << agataOutput.TS << "  TS spider: " << spiderOutput.TS << " d1time: " << d1time << " d2time: " << d2time << std::endl; 
	    
           diff
	      = (double)(1.*agataOutput.TS - 1.*spiderOutput.TS)
	      + ((d2time - d1time) / (10. * UNITS::ns));

	    h.prova->Fill(diff);
	    
	    //std::cout << "diff: " << diff << std::endl;
	    if(diff > conf.agataSpider.windowLeft && diff < conf.agataSpider.windowRight) {

	      //	      std::cout << "in coinc!" << std::endl;
	      in_coinc = true;
	      break;
	    }

	    
	  }
	}

	if(!in_coinc) return kTRUE;


	for(size_t s{0}; s< spiderOutput.E.size(); ++s) {
	  spiderEx = spiderOutput.Ex.at(s);
	  spiderTheta = angles[spiderOutput.Id.at(s)]; //spiderOutput.p3.at(s).Vect().Theta()*TMath::RadToDeg();
	  spiderPhi = spiderOutput.p3.at(s).Vect().Phi()*TMath::RadToDeg();

	  if(spiderOutput.Id.at(s)==38){
	    h.m_Ex_TSdiff->Fill(diff, spiderEx);
	    h.m_TSdiff_time->Fill( (spiderOutput.TS - firstTS)/1.e8/60., diff);
	  }
	  
	  for(size_t a{0}; a < gammaAgataSpider.at(s).EDCBP.size(); ++a) {

	    gammaEDCBP = gammaAgataSpider.at(s).EDCBP.at(a).E();

	    if(spiderEx < gate2_2 && spiderEx > gate2_1){
	    h.m_Thetap_EDCBP_e646->Fill(spiderTheta, gammaEDCBP);
	    h.m_ID_EDCBP_e646->Fill(newID[spiderOutput.Id.at(s)], gammaEDCBP);
	    }
	    else if(spiderEx < gate3_2 && spiderEx > gate3_1){
	    h.m_Thetap_EDCBP_e2638->Fill(spiderTheta, gammaEDCBP);
	    h.m_ID_EDCBP_e2638->Fill(newID[spiderOutput.Id.at(s)], gammaEDCBP);

	    if(gammaAgataSpider.at(s).EDCBP.size()>1){
	      for(int a2=0; a2 < a1; ++a2) {
		if(a2==a) continue;
		h.h_Tdiffgg->Fill(gammaAgataSpider.at(s).EDCBP.at(a).-gammaAgataSpider.at(s).EDCBP.at);
	      }
	    }


	    }
	    else if(spiderEx < gate4_2 && spiderEx > gate4_1){
	    h.m_Thetap_EDCBP_e3262->Fill(spiderTheta, gammaEDCBP);
	    h.m_ID_EDCBP_e3262->Fill(newID[spiderOutput.Id.at(s)], gammaEDCBP);
	    }

	    if(spiderOutput.Id.at(s) == 46)
	      h.m_Ex_EDCBP_ch46->Fill(spiderEx, gammaEDCBP);
	    else if(spiderOutput.Id.at(s) == 38)
	      h.m_Ex_EDCBP_ch38->Fill(spiderEx, gammaEDCBP);
	    else if(spiderOutput.Id.at(s) == 40)
	      h.m_Ex_EDCBP_ch40->Fill(spiderEx, gammaEDCBP);
	    else if(spiderOutput.Id.at(s) == 64)
	      h.m_Ex_EDCBP_ch64->Fill(spiderEx, gammaEDCBP);

	    
	    
	    
	  gammaEDCBP = 0.;

	  } //loop on agata

	  
	  spiderEx = spiderTheta = spiderPhi = 0.;

	} //close loop on spider


	*/
	
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
