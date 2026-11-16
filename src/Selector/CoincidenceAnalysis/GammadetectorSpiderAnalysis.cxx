#include "GammadetectorSpiderAnalysis.h"

#include "Lut.h"

GammadetectorSpiderAnalysis::GammadetectorSpiderAnalysis(Inputs& in,
		GammadetectorSpiderConf& gpConf, Gammadetector& g,
		GammadetectorConf& gConf, Spider& s, Readers& rg,
		SpiderAnalysis::SpiderReader& rs, std::vector<GammaDC>& gdc,
		GammadetectorSpiderHistograms& gsh)
  : CoincidenceAnalysis(in, g, gConf, s, ConfManager::getInstance().spider,
			gpConf, rg, rs, gsh),
    gammadetector(g), spider(s), gammadetectorSpiderConf(gpConf),
    gammadetectorConf(gConf), gammaDC(gdc),
    gammadetectorSpiderHistograms(gsh), spiderReader(rs) {
  
  (void)spiderReader;
}

bool GammadetectorSpiderAnalysis::AllocateHistos() {  
  GammadetectorSpiderHistograms& h = gammadetectorSpiderHistograms;
  h.dir->cd();
  
  if(!CoincidenceAnalysis::AllocateHistos()) return false;
  
  //////////////////////////////////////////////////
  // General histograms
  /////////////////////////////////////////////////
  
  Initialize<TH2D>(
		   h.histos.m_SpiderID_EDC,
		   new TH2D("m_SpiderID_EDC",
			    "SPIDER ID (strip) - Doppler corrected gamma energy;SPIDER "
			    "ID;Gamma Energy [keV]",
			    200, 0, 200,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth)
			    * 2,
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.histos.ptrs);
  SetupLabelsForChannelID(conf.spider, h.histos.m_SpiderID_EDC);

  Initialize<TH2D>(
		   h.histos.m_SpiderID_EDCBP,
		   new TH2D("m_SpiderID_EDCBP",
			    "SPIDER ID (strip) - Doppler corrected gamma energy for "
			    "binary partner;SPIDER ID;Gamma Energy [keV]",
			    200, 0, 200,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth)
			    * 2,
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.histos.ptrs);
  SetupLabelsForChannelID(conf.spider, h.histos.m_SpiderID_EDCBP);

  Initialize<TH2D>(
		   h.histos.m_Ex_E,
		   new TH2D("m_Ex_E",
			    "Excitation energy -  Gamma "
			    "energy;Excitation Energy [MeV];Gamma Energy [keV]",
			    conf.spider.Exbins, conf.spider.Exmin, conf.spider.Exmax,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.histos.ptrs);


  Initialize<TH2D>(
		   h.histos.m_Ex_EDC,
		   new TH2D("m_Ex_EDC",
			    "Excitation energy - Doppler corrected gamma "
			    "energy;Excitation Energy [MeV];Gamma Energy [keV]",
			    conf.spider.Exbins, conf.spider.Exmin, conf.spider.Exmax,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.histos.ptrs);

  Initialize<TH2D>(
		   h.histos.m_Ex_EDCBP,
		   new TH2D("m_Ex_EDCBP",
			    "Excitation energy - Doppler corrected gamma energy for "
			    "binary partner;Excitation Energy [MeV];Gamma Energy [keV]",
			    conf.spider.Exbins, conf.spider.Exmin, conf.spider.Exmax,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.histos.ptrs);

  // Gamma histograms for SPIDER
  
  GeneralHistograms &hgp = gammadetectorSpiderHistograms.histos;
  Allocate(hgp, "");


  // Adding gamma-gated histograms for general spider
  for(unsigned int gg{0}; gg < gammadetectorSpiderConf.gammaGates.size();
      ++gg) {

    gammadetectorSpiderHistograms.gammagatedHistograms.emplace_back();
    auto& vals  = gammadetectorSpiderConf.gammaGates.at(gg);
    auto& EXgg  = gammadetectorSpiderHistograms.gammagatedHistograms.back();

    EXgg.vals   = vals.gate;
    EXgg.option = vals.option;
    EXgg.dir    = h.dir->mkdir(
        Form("GammaGate_%i_%i_%s", vals.gate.first, vals.gate.second,
	     vals.option.c_str()),
	Form("Gamma spectra gated on gamma line, E=%i-%i, %s",
	     vals.gate.first, vals.gate.second, vals.option.c_str()));
    EXgg.exclude_angle = vals.exclude_angle;
    
    EXgg.dir->cd();

    std::string ss2 = "";
    if(EXgg.option != "none")
      ss2 += EXgg.option;
    ss2 = "_"+ss2;
    ss2 += std::to_string(EXgg.vals.first)+"_"+std::to_string(EXgg.vals.second);
    Allocate(EXgg, ss2);

  }

  /////////////////////////////////////////////////////
  // Adding histograms with cut in Ex energy
  ////////////////////////////////////////////////////
  
  for(unsigned int i{0}; i < gammadetectorSpiderConf.exValues.size(); ++i){

    gammadetectorSpiderHistograms.exSelection.emplace_back();
    auto& vals                = gammadetectorSpiderConf.exValues.at(i);
    
    gammadetectorSpiderHistograms.exSelection.back().vals = vals;
    gammadetectorSpiderHistograms.exSelection.back().dir  = gammadetectorSpiderHistograms.dir->mkdir(
        Form("ExGate_Ei%.3f_Ef%.3f", vals.first, vals.second),
	Form("Ex gated histograms, Ei=%.3f Ef=%.3f", vals.first,
	     vals.second));
    gammadetectorSpiderHistograms.exSelection.back().dir->cd();
    
    Allocate(gammadetectorSpiderHistograms.exSelection.back(), "");

    // Adding TGraph with kinematic line
    // we should probably check here if this isn't duplicite in theta angle?
    auto reactionsIt = input.reactions.find(conf.spider.ion);
    if(reactionsIt == input.reactions.end())
      throw std::runtime_error(
			       "Ion in spiderAgata not found in reactions of "
			       "interest! Check the conf file\n");

    std::string sTemp
      = input.treeToRead->GetDirectory()
      ->GetFile()
      ->GetName(); // to avoid potential memory leak message of hadd
    sTemp.erase(sTemp.begin(), sTemp.begin() + sTemp.find(".root") - 4);
    sTemp.erase(sTemp.end() - 5, sTemp.end());

    if(sTemp == "0000") {
      gammadetectorSpiderHistograms.exSelection.back().dir->Add(gammadetectorSpiderHistograms.exSelection.back().g_Energy_Theta
				    = GenerateKinematicLine(vals.first));
      gammadetectorSpiderHistograms.exSelection.back().dir->Add(
				    gammadetectorSpiderHistograms.exSelection.back().g_Energy_Theta2
				    = GenerateKinematicLine(vals.second));
    }


    //adding gammagated histograms
    for(unsigned int gg{0}; gg < gammadetectorSpiderConf.exgammaGates.size(); ++gg){
      gammadetectorSpiderHistograms.exSelection.back().gammagatedHistograms.emplace_back();
      auto& vals2 = gammadetectorSpiderConf.exgammaGates.at(gg);
      auto& EXgg  = gammadetectorSpiderHistograms.exSelection.back().gammagatedHistograms.back();
      EXgg.vals   = vals2.gate;
      EXgg.option = vals2.option;
      EXgg.dir    = gammadetectorSpiderHistograms.exSelection.back().dir->mkdir(
	        Form("GammaGate_%i_%i_%s", vals2.gate.first, vals2.gate.second,
		     vals2.option.c_str()),
		Form("Gamma spectra gated on gamma line, E=%i-%i, %s",
		     vals2.gate.first, vals2.gate.second,
		     vals2.option.c_str()));
      EXgg.exclude_angle = vals2.exclude_angle;
      EXgg.dir->cd();

      std::string ss3 = "";
      if(EXgg.option != "none")
	ss3 += EXgg.option;
      ss3 = "_"+ss3;
      ss3 += std::to_string(EXgg.vals.first)+"_"+std::to_string(EXgg.vals.second);
      Allocate(EXgg, ss3);


    }
    
  }


  /////////////////////////////////////////////////////
  // Adding histograms with TCutG in E vs Theta
  ////////////////////////////////////////////////////

  conf.AddCuts<GammadetectorSpiderHistograms>(
        conf.spider.ethetacutPath, gammadetectorSpiderHistograms.eTheta);

  
  for(auto& it: gammadetectorSpiderHistograms.eTheta){
    it.dir = gammadetectorSpiderHistograms.dir->mkdir(Form("E_Theta_%s", it.cut->GetName()),
			  "Gamma spectra with cut on Elab - ThetaLab");
    it.dir->cd();
    it.cut->Write();
    Allocate(it, "");

    //adding gammagated histograms
    for(unsigned int gg{0}; gg < gammadetectorSpiderConf.exgammaGates.size(); ++gg){
      gammadetectorSpiderHistograms.exSelection.back().gammagatedHistograms.emplace_back();
      auto& vals2 = gammadetectorSpiderConf.exgammaGates.at(gg);
      auto& EXgg  = it.gammagatedHistograms.back();
      EXgg.vals   = vals2.gate;
      EXgg.option = vals2.option;
      EXgg.dir    = it.dir->mkdir(
	        Form("GammaGate_%i_%i_%s", vals2.gate.first, vals2.gate.second,
		     vals2.option.c_str()),
		Form("Gamma spectra gated on gamma line, E=%i-%i, %s",
		     vals2.gate.first, vals2.gate.second,
		     vals2.option.c_str()));
      EXgg.exclude_angle = vals2.exclude_angle;
      EXgg.dir->cd();

      std::string ss4 = "";
      if(EXgg.option != "none")
	ss4 += EXgg.option;
      ss4 = "_"+ss4;
      ss4 += std::to_string(EXgg.vals.first)+"_"+std::to_string(EXgg.vals.second);
      
      Allocate(EXgg, ss4);

    }
    
  }
  
  
  return true;

}



bool GammadetectorSpiderAnalysis::Allocate(GammaParticleHistograms& h, std::string suffix){

  Initialize<TH1D>(
		   h.h_E,
		   new TH1D(Form("h_E%s", suffix.c_str()),
			    Form("Gamma energy;Energy [keV];Counts/%d keV",
				 gammadetectorConf.binWidth),
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH1D>(
		   h.h_EDC,
		   new TH1D(
			    Form("h_EDC%s", suffix.c_str()),
			    Form("Doppler corrected gamma energy;Energy [keV];Counts/%d keV",
				 gammadetectorConf.binWidth),
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH1D>(
		   h.h_EDCBP,
		   new TH1D(Form("h_EDCBP%s", suffix.c_str()),
			    Form("Doppler corrected gamma energy for binary "
				 "partner;Energy [keV];Counts/%d keV",
				 gammadetectorConf.binWidth),
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_E_E,
		   new TH2D(Form("m_E_E%s", suffix.c_str()), "Gamma-gamma energy;Energy [keV];Energy [keV]",
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_EDC_EDC,
		   new TH2D(
			    Form("m_EDC_EDC%s", suffix.c_str()),
			    "Doppler corrected gamma-gamma energy;Energy [keV];Energy [keV]",
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_EDCBP_EDCBP,
		   new TH2D(Form("m_EDCBP_EDCBP%s", suffix.c_str()),
			    "Doppler corrected gamma-gamma energy for binary "
			    "partner;Energy [keV];Energy [keV]",
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_E_EDC,
		   new TH2D(Form("m_E_EDC%s", suffix.c_str()),
			    "Gamma energy vs Doppler corrected gamma energy;Energy "
			    "[keV];Energy DC[keV]",
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_E_EDCBP,
		   new TH2D(Form("m_E_EDCBP%s", suffix.c_str()),
			    "Gamma energy vs Doppler corrected energy for binary "
			    "partner;Energy [keV];Energy DCBP[keV]",
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_EDC_EDCBP,
		   new TH2D(Form("m_EDC_EDCBP%s", suffix.c_str()),
			    "Doppler corrected energy vs energy doppler corrected for "
			    "binary partner;Energy DC[keV];Energy DCBP[keV]",
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_Beta_E,
		   new TH2D(Form("m_E_Beta%s", suffix.c_str()), "Gamma - Beta;Beta [%];Energy [keV]", 4000, 0, 20,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_BetaBP_E,
		   new TH2D(Form("m_E_BetaBP%s", suffix.c_str()), "Gamma - Beta BP;Beta [%];Energy [keV]", 4000,
			    0, 20,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_Beta_EDC,
		   new TH2D(Form("m_EDC_Beta%s", suffix.c_str()),
			    "Gamma Doppler corrected - Beta;Beta [%];Energy [keV]", 4000,
			    0, 20,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_BetaBP_EDCBP,
		   new TH2D(Form("m_EDCBP_BetaBP%s", suffix.c_str()),
			    "Gamma Doppler corrected for binary partner - Beta BP;Beta "
			    "[%];Energy [keV]",
			    4000, 0, 20,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_Theta_E,
		   new TH2D(Form("m_E_Theta%s", suffix.c_str()), "Gamma - theta energy;Theta [deg];Energy [keV]",
			    gammadetectorSpiderConf.thetaBins, gammadetectorSpiderConf.thetaMin, gammadetectorSpiderConf.thetaMax,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_ThetaBP_E,
		   new TH2D(Form("m_E_ThetaBP%s", suffix.c_str()),
			    "Gamma - theta for binary partner;Theta [deg];Energy [keV]",
			    gammadetectorSpiderConf.thetaBins, gammadetectorSpiderConf.thetaMin, gammadetectorSpiderConf.thetaMax,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_ThetaBP_EDCBP,
		   new TH2D(
			    Form("m_EDCBP_ThetaBP%s", suffix.c_str()),
			    "Gamma - theta energy for binary partner;Theta [deg];Energy [keV]",
			    gammadetectorSpiderConf.thetaBins, gammadetectorSpiderConf.thetaMin, gammadetectorSpiderConf.thetaMax,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  Initialize<TH2D>(
		   h.m_Theta_EDC,
		   new TH2D(Form("m_EDC_Theta%s", suffix.c_str()),
			    "Doppler corrected gamma - theta;Angle [deg];Energy [keV]",
			    gammadetectorSpiderConf.thetaBins, gammadetectorSpiderConf.thetaMin, gammadetectorSpiderConf.thetaMax,
			    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
					     / gammadetectorConf.binWidth),
			    0, gammadetectorConf.gammaEmax / UNITS::keV),
		   h.ptrs);

  
  return true;

}

void GammadetectorSpiderAnalysis::Clear() { gammaDC.clear(); }


void GammadetectorSpiderAnalysis::Analyze() {
  CoincidenceAnalysis::Analyze();

  GeneralHistograms &h = gammadetectorSpiderHistograms.histos;

  //clear the vector of DC corrected gammas, and reserve the correct space
  gammaDC.clear();
  gammaDC.resize(coincMatrix.size());


  //Doppler correct
  for(size_t v{0}; v<coincMatrix.size(); v++){
    
    auto &currentIon = gammaDC.at(v);
    currentIon.resize(gammadetector.P.size());

    
    ReactionFragment::Ion ionBP
      = conf.reaction.target + conf.reaction.beam - conf.spider.ion;
    
    currentIon.p3 = SlowInTarget(
				 spider.p3.at(v), gammadetectorSpiderConf.dcPos, conf.spider.ion);
    
    currentIon.p4 = SlowInTarget(
				 spider.p4.at(v), gammadetectorSpiderConf.bpDcPos, ionBP);
    
    for(size_t s{0}; s<currentIon.E.size(); s++){

      if(coincMatrix.at(v).at(s)){
	currentIon.inCoinc.at(s) = true;
      } else {
	currentIon.inCoinc.at(s) = false;
      }
      
      currentIon.E.at(s)     = gammadetector.P.at(s);
      currentIon.EDC.at(s)   = gammadetector.P.at(s);
      currentIon.EDCBP.at(s) = gammadetector.P.at(s);
      
      currentIon.EDC.at(s).Boost(-currentIon.p3.BoostVector());
      currentIon.EDCBP.at(s).Boost(-currentIon.p4.BoostVector());
      
    }
  }  
	
}


bool GammadetectorSpiderAnalysis::FillHistos() {
  if(!CoincidenceAnalysis::FillHistos()) return false;

  // This is not the most optimal way to do it, however the previous code
  // was amounting to more than 1000 of lines that were mostly CTRL+K CTRL+Y
  // (anyone who doesn't use EMACS is a non-believer)

  //////////////////////////////////
  //Fill general histos of spider
  /////////////////////////////////////
  
  auto& h = gammadetectorSpiderHistograms;

  for(unsigned int s{0}; s < spider.E.size(); ++s){
    for(unsigned int a1{0}; a1 < gammadetector.E.size(); ++a1){

      if(!gammaDC.at(s).inCoinc.at(a1)) continue;

      // we fill these histograms here because we don't want them in all the folders
      Fill(h.histos.m_Ex_E, spider.Ex.at(s), gammadetector.E.at(a1));
      Fill(h.histos.m_Ex_EDC, spider.Ex.at(s), gammaDC.at(s).EDC.at(a1).E());
      Fill(h.histos.m_Ex_EDCBP, spider.Ex.at(s), gammaDC.at(s).EDCBP.at(a1).E());
      Fill(h.histos.m_SpiderID_EDC, spider.Id.at(s), gammaDC.at(s).EDC.at(a1).E());
      Fill(h.histos.m_SpiderID_EDCBP, spider.Id.at(s), gammaDC.at(s).EDCBP.at(a1).E());

      //fill recurring histos
      h.histos.Fill(spider, gammadetector, gammaDC, s, a1);

      
      for(unsigned int a2{0}; a2 < gammadetector.E.size(); ++a2){
	if(!gammaDC.at(s).inCoinc.at(a2)) continue;
	if(a1 == a2) continue;	
	h.histos.Fill2(spider, gammadetector, gammaDC, s, a1, a2);

	for(auto& gg : h.gammagatedHistograms){
	  if(GammadetectorSpiderAnalysis::Check(gg,spider, gammadetector, gammaDC, s, a1)){
	    gg.Fill(spider, gammadetector, gammaDC, s, a2);
	    for(unsigned int a3{0}; a3 < gammadetector.E.size(); ++a3){
	      if(!gammaDC.at(s).inCoinc.at(a3)) continue;
	      if(a3 == a2 || a3 == a1) continue;
	      gg.Fill2(spider, gammadetector, gammaDC, s, a2, a3);
	    } //end loop on a3
	  } // end if in gamma gate
	}// end loop on gamma gates
	
      }//end loop on a2

    } //end loop of a1
  } //end loop on s


  //////////////////////////////////
  //Fill Ex cut histos
  /////////////////////////////////////

  for(auto& sel : h.exSelection){

    for(unsigned int s{0}; s < spider.E.size(); ++s){
      if(spider.Ex[s] < sel.vals.first || spider.Ex[s] > sel.vals.second) continue;

      for(unsigned int a1{0}; a1 < gammadetector.E.size(); ++a1){
	if(!gammaDC.at(s).inCoinc.at(a1)) continue;

	sel.Fill(spider, gammadetector, gammaDC, s, a1);
	
	for(unsigned int a2{0}; a2 < gammadetector.E.size(); ++a2){
	  if(!gammaDC.at(s).inCoinc.at(a2)) continue;
	  if(a1 == a2) continue;

	  sel.Fill2(spider, gammadetector, gammaDC, s, a1, a2);


	  //gamma gated part
	  for(auto& gg : sel.gammagatedHistograms){
	    if(GammadetectorSpiderAnalysis::Check(gg,spider, gammadetector, gammaDC, s, a1)){
	      gg.Fill(spider, gammadetector, gammaDC, s, a2);
	      for(unsigned int a3{0}; a3 < gammadetector.E.size(); ++a3){
		if(!gammaDC.at(s).inCoinc.at(a3)) continue;
		if(a3 == a2 || a3 == a1) continue;
		gg.Fill2(spider, gammadetector, gammaDC, s, a2, a3);
	      } //end loop on a3
	    } // end if in gamma gate
	  }// end loop on gamma gates


	  
	}
       
      }
    }


    
  }
  

  //////////////////////////////////
  //Fill E-Theta histos
  /////////////////////////////////////

  for(auto& sel : h.eTheta){

    for(unsigned int s{0}; s < spider.E.size(); ++s){
      if(!sel.cut->IsInside(spider.p3.at(s).Vect().Theta()/UNITS::deg, spider.ESi.at(s))) continue;
      
      for(unsigned int a1{0}; a1 < gammadetector.E.size(); ++a1){
	if(!gammaDC.at(s).inCoinc.at(a1)) continue;

	sel.Fill(spider, gammadetector, gammaDC, s, a1);
	
	for(unsigned int a2{0}; a2 < gammadetector.E.size(); ++a2){
	  if(!gammaDC.at(s).inCoinc.at(a2)) continue;
	  if(a1 == a2) continue;

	  sel.Fill2(spider, gammadetector, gammaDC, s, a1, a2);	

	  for(auto& gg : h.gammagatedHistograms){
	    if(GammadetectorSpiderAnalysis::Check(gg,spider, gammadetector, gammaDC, s, a1)){
	      gg.Fill(spider, gammadetector, gammaDC, s, a2);
	      for(unsigned int a3{0}; a3 < gammadetector.E.size(); ++a3){
		if(!gammaDC.at(s).inCoinc.at(a3)) continue;
		if(a3 == a2 || a3 == a1) continue;
		gg.Fill2(spider, gammadetector, gammaDC, s, a2, a3);
	      } //end loop on a3
	    } // end if in gamma gate
	  }// end loop on gamma gates
	  
	}
	
      }
    }
   

  }

  
  return true;

}


bool GammadetectorSpiderAnalysis::GammaParticleHistograms::Fill(Spider& spider, Gammadetector& gammadetector, std::vector<GammaDC> &gammaDC, unsigned int s, unsigned int a1){
  
  GammadetectorSpiderAnalysis::Fill(h_E, gammadetector.E.at(a1));
  GammadetectorSpiderAnalysis::Fill(h_EDC, gammaDC.at(s).EDC.at(a1).E());
  GammadetectorSpiderAnalysis::Fill(h_EDCBP, gammaDC.at(s).EDCBP.at(a1).E());

  GammadetectorSpiderAnalysis::Fill(m_Beta_E, gammaDC.at(s).p3.Beta() * 100,
         gammadetector.E.at(a1));
  GammadetectorSpiderAnalysis::Fill(m_BetaBP_E, gammaDC.at(s).p4.Beta() * 100,
	gammadetector.E.at(a1));

  GammadetectorSpiderAnalysis::Fill(m_Beta_EDC, gammaDC.at(s).p3.Beta() * 100,
	gammaDC.at(s).EDC.at(a1).E());
  GammadetectorSpiderAnalysis::Fill(m_BetaBP_EDCBP, gammaDC.at(s).p4.Beta() * 100,
	gammaDC.at(s).EDCBP.at(a1).E());

  GammadetectorSpiderAnalysis::Fill(m_Theta_E,
	spider.p3.at(s).Angle(gammadetector.P.at(a1).Vect())
	/ UNITS::deg,
	gammadetector.E.at(a1));

  GammadetectorSpiderAnalysis::Fill(m_ThetaBP_E,
	spider.p4.at(s).Angle(gammadetector.P.at(a1).Vect())
	/ UNITS::deg,
	gammadetector.E.at(a1));

  GammadetectorSpiderAnalysis::Fill(m_Theta_EDC,
	        spider.p3.at(s).Angle(gammadetector.P.at(a1).Vect())
	        / UNITS::deg,
	        gammaDC.at(s).EDC.at(a1).E());
  
  GammadetectorSpiderAnalysis::Fill(m_ThetaBP_EDCBP,
		spider.p4.at(s).Angle(gammadetector.P.at(a1).Vect())
		/ UNITS::deg,
		gammaDC.at(s).EDCBP.at(a1).E());

  return true;

}



bool GammadetectorSpiderAnalysis::GammaParticleHistograms::Fill2(Spider& spider, Gammadetector& gammadetector, std::vector<GammaDC> &gammaDC, unsigned int s, unsigned int a1, unsigned int a2){

  GammadetectorSpiderAnalysis::Fill(m_E_E, gammadetector.E.at(a1),
       gammadetector.E.at(a2));
  
  GammadetectorSpiderAnalysis::Fill(m_EDC_EDC, gammaDC.at(s).EDC.at(a1).E(),
       gammaDC.at(s).EDC.at(a2).E());
  
  GammadetectorSpiderAnalysis::Fill(m_EDCBP_EDCBP, gammaDC.at(s).EDCBP.at(a1).E(),
       gammaDC.at(s).EDCBP.at(a2).E());

  GammadetectorSpiderAnalysis::Fill(m_E_EDC, gammadetector.E.at(a1),
       gammaDC.at(s).EDC.at(a2).E());

  GammadetectorSpiderAnalysis::Fill(m_E_EDCBP, gammadetector.E.at(a1),
       gammaDC.at(s).EDCBP.at(a2).E());

  GammadetectorSpiderAnalysis::Fill(m_EDC_EDCBP, gammaDC.at(s).EDC.at(a1).E(),
       gammaDC.at(s).EDCBP.at(a2).E());


  return true;
  
}


bool GammadetectorSpiderAnalysis::Check(GammaGatedHistograms & gg, Spider& spider, Gammadetector &gammadetector, std::vector<GammaDC>& gammaDC, unsigned int s,  unsigned int a1){

  double gamma1, angle1;
  if(gg.option.find("DC") != std::string::npos) {
    if(gg.option.find("BP") != std::string::npos) {
      gamma1 = gammaDC.at(s).EDCBP.at(a1).E();
      angle1 = spider.p4.at(s).Angle(
				     gammadetector.P.at(a1).Vect())
	/ UNITS::deg;
    } else {
      gamma1 = gammaDC.at(s).EDC.at(a1).E();
      angle1 = spider.p3.at(s).Angle(
				     gammadetector.P.at(a1).Vect())
	/ UNITS::deg;
    }
  } else {
    gamma1 = gammadetector.E.at(a1);
    if(gg.option.find("BP") != std::string::npos) {
      angle1 = spider.p4.at(s).Angle(
				     gammadetector.Pos.at(a1))
	/ UNITS::deg;
    }else{
      angle1 = spider.p3.at(s).Angle(
				     gammadetector.Pos.at(a1))
	/ UNITS::deg;
    }
  }

  if(gamma1 < gg.vals.first || gamma1 > gg.vals.second) return false;
  if(angle1 > gg.exclude_angle.first && angle1 < gg.exclude_angle.second) return false;
  return true;

}


void GammadetectorSpiderAnalysis::Finalize() {
	CoincidenceAnalysis::Finalize();
}

TGraph* GammadetectorSpiderAnalysis::GenerateKinematicLine(const double& val) {
	auto reactionsIt = input.reactions.find(conf.spider.ion);
	reactionsIt->second.GetFreeFragment().Set_Ex(val * UNITS::MeV);

	std::vector<double> x, y;
	double              thetaMax
		= static_cast<double>(reactionsIt->second.Get_ThetaMax()) / UNITS::deg;
	x.reserve(180);
	y.reserve(180);

	for(int p{1}; p < thetaMax; ++p) {
		try {
			reactionsIt->second.Set_Theta(p * UNITS::deg, true);
		} catch(const std::runtime_error& e) {
			std::cout << e.what();
			// Do not worry, reaction calculations sometimes are not enough
			// precise
		};
		x.push_back(p);
		y.push_back(static_cast<double>(
					reactionsIt->second.GetFixedFragment().Get_Ek()));
	}
	if(thetaMax < 180) {
		for(int p{static_cast<int>(thetaMax)}; p > 0; --p) {
			try {
				reactionsIt->second.Set_Theta(p * UNITS::deg, false);
			} catch(const std::runtime_error& e) {
				continue;
				// Do not worry, reaction calculations sometimes are not enough
				// precise
			};
			x.push_back(p);
			y.push_back(static_cast<double>(
						reactionsIt->second.GetFixedFragment().Get_Ek()));
		}
	}
	auto* gr = new TGraph(static_cast<int>(x.size()), &x[0], &y[0]);
	gr->SetName(Form("g_Energy_Theta_%.3f_%i", val, kineLineCounter++));
	gr->SetTitle(
			Form("Kinematic line with Ex = %.3f;Theta [deg];Energy [MeV]", val));
	return gr;
}
