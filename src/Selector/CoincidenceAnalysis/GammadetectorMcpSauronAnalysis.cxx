#include "GammadetectorMcpSauronAnalysis.h"

GammadetectorMcpSauronAnalysis::GammadetectorMcpSauronAnalysis(
    Inputs& in, GammadetectorSauronConf& gpConf, Gammadetector& g,
    GammadetectorConf& gConf, SauronMcp& sm, SauronConf& sc, Readers& rg,
    SauronAnalysis::SauronReader& rs, std::vector<GammaDC>& gdc,
    GammadetectorMcpSauronHistograms& gsh, Sauron& s, Mcp& mm)
    : CoincidenceAnalysis(in, g, gConf, sm, sc, gpConf, rg, rs, gsh),
      gammadetector(g), sauron(s), sauronMcp(sm), mcp(mm), sauronConf(sc),
      gammadetectorSauronConf(gpConf), gammadetectorConf(gConf), gammaDC(gdc),
      gammadetectorMcpSauronHistograms(gsh), sauronReader(rs) {

        (void)sauronReader;
    }

bool GammadetectorMcpSauronAnalysis::AllocateHistos() {
    GammadetectorMcpSauronHistograms& h = gammadetectorMcpSauronHistograms;
    h.dir->cd();

    if(!CoincidenceAnalysis::AllocateHistos()) return false;

    int   nbins{(gammadetectorSauronConf.windowRight - gammadetectorSauronConf.windowLeft + 600)* 10};
    int   xmin{gammadetectorSauronConf.windowLeft - 300};
    int   xmax{gammadetectorSauronConf.windowRight + 300};

    
    Initialize<TH2D>(
		     h.m_Ex_TSmTS,
		     new TH2D("m_Ex_TSmTS",
			      "SAURON-MCP Ex vs Time difference with AGATA;TS [10 ns]; Excitation Energy [MeV]", nbins,
			      xmin, xmax,
			      static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
					       / sauronConf.sauronExbinWidth),
			      -10, sauronConf.sauronExmax / UNITS::MeV),
		     h.ptrs);
    
    
    Initialize<TH2D>(
		     h.m_E_TSmTS_cfd,
		     new TH2D(
			      "m_E_TSmTS",
			      "E vs Time difference of the two detectors;Time difference "
				   "[10 ns];E_{#gamma} [keV]",  
			      nbins, xmin, xmax,static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
			      0, gammadetectorConf.gammaEmax / UNITS::keV),
		     h.ptrs);

    Initialize<TH2D>(
		     h.m_EDC_TSmTS_cfd,
		     new TH2D(
			      "m_EDC_TSmTS",
			      "E vs Time difference of the two detectors;Time difference "
				   "[10 ns];E_{#gamma} [keV]",  
			      nbins, xmin, xmax,static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
			      0, gammadetectorConf.gammaEmax / UNITS::keV),
		     h.ptrs);

        
    Initialize<TH2D>(
		     h.m_E_posX,
		     new TH2D(
			      "m_E_posX",
			      "E vs position on target X;X (<-AGATA|PRISMA->) [mm] "
				   "[10 ns];E_{#gamma} [keV]",  
			      1000,-100,100,static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
			      0, gammadetectorConf.gammaEmax / UNITS::keV),
		     h.ptrs);

    Initialize<TH2D>(
		     h.m_E_posY,
		     new TH2D(
			      "m_E_posY",
			      "E vs position on target Y;Y [mm] "
				   "[10 ns];E_{#gamma} [keV]",  
			      1000,-100,100,static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
			      0, gammadetectorConf.gammaEmax / UNITS::keV),
		     h.ptrs);

    
    Initialize<TH1D>(h.h_cTime_gated,
                     new TH1D("h_cTime_gated", "h_cTime_gated",
                              (gammadetectorSauronConf.windowRight
                               - gammadetectorSauronConf.windowLeft + 200)
                                  * 10,
                              gammadetectorSauronConf.windowLeft - 100,
                              gammadetectorSauronConf.windowRight + 100),
                     h.ptrs);

    Initialize<TH1D>(
        h.h_E,
        new TH1D("h_E",
                 Form("Gamma energy;Energy [keV];Counts/%d keV",
                      gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_EDC,
        new TH1D(
            "h_EDC",
            Form("Doppler corrected gamma energy;Energy [keV];Counts/%d keV",
                 gammadetectorConf.binWidth),
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                             / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_EDCBP,
        new TH1D("h_EDCBP",
                 Form("Doppler corrected gamma energy for binary "
                      "partner;Energy [keV];Counts/%d keV",
                      gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_Beta,
        new TH2D("m_E_Beta", "Gamma - Beta;Beta [%];Energy [keV]", 4000, 0, 20,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_BetaBP,
        new TH2D("m_E_BetaBP", "Gamma - Beta BP;Beta [%];Energy [keV]", 4000,
                 0, 20,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDC_Beta,
        new TH2D("m_EDC_Beta",
                 "Gamma Doppler corrected - Beta;Beta [%];Energy [keV]", 4000,
                 0, 20,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDCBP_BetaBP,
        new TH2D("m_EDCBP_BetaBP",
                 "Gamma Doppler corrected for binary partner - Beta BP;Beta "
                 "[%];Energy [keV]",
                 4000, 0, 20,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_Theta,
        new TH2D("m_E_Theta", "Gamma - theta energy;Theta [deg];Energy [keV]",
                 1000, 0, 180,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_ThetaBP,
        new TH2D("m_E_ThetaBP",
                 "Gamma - theta for binary partner;Theta [deg];Energy [keV]",
                 1000, 0, 180,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDCBP_ThetaBP,
        new TH2D(
            "m_EDCBP_ThetaBP",
            "Gamma - theta energy for binary partner;Theta [deg];Energy [keV]",
            1000, 0, 180,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                             / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDC_Theta,
        new TH2D("m_EDC_Theta",
                 "Doppler corrected gamma - theta;Angle [deg];Energy [keV]",
                 1000, 0, 180,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDCBP_Theta,
        new TH2D("m_EDCBP_Theta",
                 "Doppler corrected gamma- theta for binary partner;Angle "
                 "[deg];Energy [keV]",
                 1000, 0, 180,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDCBP_TS,
        new TH2D("m_EDCBP_TS", // Added by MR (be careful!)
                 "Doppler corrected gamma-TS for binary partner;TimeStamp "
                 "[s];Energy [keV]",
                 4320, 0, 12 * 3600,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Ex_E,
        new TH2D("m_Ex_E",
                 "Excitation energy - gamma energy;Excitation Energy "
                 "[MeV];Gamma Energy [keV]",
                 static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
                                  / sauronConf.sauronExbinWidth)
                     + static_cast<int>(10 / sauronConf.sauronExbinWidth),
                 -10, sauronConf.sauronExmax / UNITS::MeV,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Ex_EDC,
        new TH2D("m_Ex_EDC",
                 "Excitation energy - Doppler corrected gamma "
                 "energy;Excitation Energy [MeV];Gamma Energy [keV]",
                 static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
                                  / sauronConf.sauronExbinWidth)
                     + static_cast<int>(10 / sauronConf.sauronExbinWidth),
                 -10, sauronConf.sauronExmax / UNITS::MeV,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Ex_EDCBP,
        new TH2D("m_Ex_EDCBP",
                 "Excitation energy - Doppler corrected gamma energy for "
                 "binary partner;Excitation Energy [MeV];Gamma Energy [keV]",
                 static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
                                  / sauronConf.sauronExbinWidth)
                     + static_cast<int>(10 / sauronConf.sauronExbinWidth),
                 -10, sauronConf.sauronExmax / UNITS::MeV,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(
        h.m_SauronID_EDC,
        new TH2D("m_SauronID_EDC",
                 "SAURON ID (strip) - Doppler corrected gamma energy"
                 ";SAURON ID;Gamma Energy [keV]",
                 100, 0, 100,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth)
                     * 2,
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);
    SetupLabelsForChannelID(sauronConf, h.m_SauronID_EDC);
    Initialize<TH2D>(
        h.m_SauronID_EDCBP,
        new TH2D("m_SauronID_EDCBP",
                 "SAURON ID (strip) - Doppler corrected gamma energy for "
                 "binary partner;SAURON ID;Gamma Energy [keV]",
                 100, 0, 100,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth)
                     * 2,
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);
    SetupLabelsForChannelID(sauronConf, h.m_SauronID_EDCBP);

    // Histograms with condition on Excitation energy
    for(unsigned int i{0}; i < gammadetectorSauronConf.exValues.size(); ++i) {
        h.exSelection.emplace_back();
        auto& vals                = gammadetectorSauronConf.exValues.at(i);

        h.exSelection.back().vals = vals;
        h.exSelection.back().dir  = h.dir->mkdir(
            Form("ExGate_Ei%.3f_Ef%.3f", vals.first, vals.second),
            Form("Ex gated histograms, Ei=%.3f Ef=%.3f", vals.first,
                  vals.second));
        h.exSelection.back().dir->cd();

	 Initialize<TH2D>(
			  h.exSelection.back().m_EDC_TSmTS_cfd,
		     new TH2D(
			      "m_EDC_TSmTS",
			      "E vs Time difference of the two detectors;Time difference "
				   "[10 ns];E_{#gamma} [keV]",  
			      nbins, xmin, xmax,static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
			      0, gammadetectorConf.gammaEmax / UNITS::keV),
		     h.exSelection.back().ptrs);

	
        Initialize<TH1D>(
            h.exSelection.back().h_E,
            new TH1D("h_E", "Gamma energy; Energy [keV]; Counts/keV",
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH1D>(
            h.exSelection.back().h_EDC,
            new TH1D("h_EDC",
                     " Doppler-corrected energy; Energy [keV]; Counts/keV",
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH1D>(
            h.exSelection.back().h_EDCBP,
            new TH1D("h_EDCBP",
                     "Doppler-corrected energy for binary partner; Energy "
                     "[keV]; Counts/keV",
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH2D>(
            h.exSelection.back().m_E_Theta,
            new TH2D("m_E_Theta", "Gamma-theta; Theta; Energy [keV]", 1000, 0,
                     180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH2D>(
            h.exSelection.back().m_E_ThetaBP,
            new TH2D("m_E_ThetaBP",
                     "Gamma-theta for binary partner; Theta; Energy [keV]",
                     1000, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH2D>(
            h.exSelection.back().m_EDCBP_Theta,
            new TH2D("m_EDCBP_Theta",
                     " DCBP energy-theta; Theta; Energy [keV]", 1000, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH2D>(
            h.exSelection.back().m_EDCBP_ThetaBP,
            new TH2D(
                "m_EDCBP_ThetaBP",
                "DCBP energy - theta for binary partner; Theta; Energy [keV]",
                1000, 0, 180,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH2D>(
            h.exSelection.back().m_E_Beta,
            new TH2D("m_E_Beta", "Gamma - Beta;Beta [%];Energy [keV]", 4000, 0,
                     20,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        Initialize<TH2D>(
            h.exSelection.back().m_E_BetaBP,
            new TH2D("m_E_BetaBP", "Gamma - Beta;Beta [%];Energy [keV]", 4000,
                     0, 20,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            h.exSelection.back().ptrs);

        // Adding TGraph with kinematic line
        // we should probably check here if this isn't duplicite in theta angle?
        auto reactionsIt = input.reactions.find(sauronConf.ion);
        if(reactionsIt == input.reactions.end())
            throw std::runtime_error("Ion in sauron not found in reactions of "
                                     "interest! Check the conf file\n");

        std::string sTemp
            = input.treeToRead->GetDirectory()
                  ->GetFile()
                  ->GetName(); // to avoid potential memory leak message of hadd
        sTemp.erase(sTemp.begin(), sTemp.begin() + sTemp.find(".root") - 4);
        sTemp.erase(sTemp.end() - 5, sTemp.end());

        if(sTemp == "0000") {
            h.exSelection.back().dir->Add(h.exSelection.back().g_Energy_Theta
                                          = GenerateKinematicLine(vals.first));
            h.exSelection.back().dir->Add(
                h.exSelection.back().g_Energy_Theta2
                = GenerateKinematicLine(vals.second));
        }
    }

    for(const auto& reaction : input.reactions) {
        GammadetectorMcpSauronHistograms::ReactionHistograms& reactionHistos
            = h.reactionHistos[{reaction.first}];
        std::string histoName = "_A" + std::to_string(reaction.first.A) + "Z"
                              + std::to_string(reaction.first.Z);
        std::string histoTitle = " A=" + std::to_string(reaction.first.A)
                               + " Z=" + std::to_string(reaction.first.Z);

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s",
                 histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(
            reactionHistos.h_E,
            new TH1D(Form("h_E_ion%s", histoName.c_str()),
                     Form("E spectrum for %s;Energy [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH1D>(
            reactionHistos.h_EDCBP,
            new TH1D(Form("h_EDCBP_ion%s", histoName.c_str()),
                     Form("DC spectrum for %s;Energy DC [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_E,
            new TH2D(Form("m_Ex_E_ion%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs gamma energy for "
                          "%s;Energy [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_EDCBP,
            new TH2D(Form("m_Ex_EDCBP_ion%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs DC gamma energy for "
                          "%s;Energy DC [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);
        Initialize<TH2D>(
            reactionHistos.m_SauronID_Ex,
            new TH2D(Form("m_SauronID_Ex_ion%s", histoName.c_str()),
                     Form("SAURON ID (strip) - excitation energy for "
                     "%s;SAURON ID;Excitation Energy [MeV]",
                      histoTitle.c_str()),
                     100, 0, 100,
                     static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
                                      / sauronConf.sauronExbinWidth)
                         + static_cast<int>(10 / sauronConf.sauronExbinWidth),
                     -10, sauronConf.sauronExmax / UNITS::MeV ),
            reactionHistos.ptrs);
        SetupLabelsForChannelID(sauronConf, reactionHistos.m_SauronID_Ex);
    }

    for(const auto& reaction : input.nBodyReactions) {
        GammadetectorMcpSauronHistograms::ReactionHistograms& reactionHistos
            = h.reactionHistos[reaction.first];
        std::string histoName;
        std::string histoTitle;

        for(const auto& it : reaction.first) {
            histoName += "_";
            histoName += "A" + std::to_string(it.A);
            histoName += "Z" + std::to_string(it.Z);

            histoTitle += " A=" + std::to_string(it.A);
            histoTitle += " Z=" + std::to_string(it.Z);
        }

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s",
                 histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(
            reactionHistos.h_E,
            new TH1D(Form("h_E_ion%s", histoName.c_str()),
                     Form("E spectrum for %s;Energy [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH1D>(
            reactionHistos.h_EDCBP,
            new TH1D(Form("h_EDCBP_ion%s", histoName.c_str()),
                     Form("DC spectrum for %s;Energy DC [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_E,
            new TH2D(Form("m_Ex_E_ion%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs gamma energy for "
                          "%s;Energy [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_EDCBP,
            new TH2D(Form("m_Ex_DCEBP_ion%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs DC gamma energy for "
                          "%s;Energy DC [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);
    }
    return true;
}

void GammadetectorMcpSauronAnalysis::Clear() { gammaDC.clear(); }

void GammadetectorMcpSauronAnalysis::Analyze() {

    if(settings.verbosity_level>0 && gammadetector.P.size()>0){
      std::cout<<"-------------------------------------------------------------------------------------"<<std::endl;
      std::cout<<"NEW AGATA-MCP-SAURON event"<<std::endl;
      std::cout<<"TS AGATA "<<gammadetector.TS<<std::endl;
      std::cout<<"TS SAURON-MCP "<<sauronMcp.TS<<std::endl;
    }
    
    CoincidenceAnalysis::Analyze();

    sauronMcp.sauronMcpAgataGood = false;
    gammaDC.clear();
	//if(!in_coinc) return;
    gammaDC.resize(coincMatrix.size());

   if(settings.verbosity_level>1)
	    std::cout<<"coinc size "<<coincMatrix.size()<<std::endl;
      
    for(size_t v{0}; v<coincMatrix.size(); v++){
      
      if(settings.verbosity_level>1)
        std::cout<<"sauronMcpGood "<<sauronMcp.sauronMcpGood<<std::endl;
            
      if(!sauronMcp.sauronMcpGood) continue;


        if(settings.verbosity_level>1){
            std::cout<<"Coincidence matrix size "<<coincMatrix.size()<<" index "<<v<<std::endl;  
            for(size_t j = 0; j < coincMatrix.at(v).size(); ++j){
                std::cout<<coincMatrix.at(v).at(j)<<"\t";
            }
            std::cout<<std::endl;
        }

		
	auto &currentIon = gammaDC.at(v);
        currentIon.resize(gammadetector.P.size());
    
        currentIon.p3 = sauronMcp.p3.at(v);

	    //	std::cout<<"size p3 "<<sauron.p3.size()<<" "<<" size p4 "<<sauronMcp.p4.size()<<std::endl;
        currentIon.p4 = sauronMcp.p4.at(v);
	if(gammadetector.E.size()<1){
	  sauronMcp.sauronMcpAgataGood = false;
	} else sauronMcp.sauronMcpAgataGood = true;
	
        if(settings.verbosity_level > 0){
            for(size_t a{0}; a < gammadetector.E.size(); ++a){
                std::cout << "\nGamma " << a << " E X Y Z\t" << gammadetector.E.at(a) << "\t"
                    << gammadetector.Pos.at(a).X()/UNITS::mm << " " 
                    << gammadetector.Pos.at(a).Y()/UNITS::mm << " "
                    << gammadetector.Pos.at(a).Z()/UNITS::mm << "\n"
                    << gammadetector.Pos.at(a).Mag()/UNITS::mm << " " 
                    << gammadetector.Pos.at(a).Theta()/UNITS::deg << " "
                    << gammadetector.Pos.at(a).Phi()/UNITS::deg << "\n";
                std::cout << "BetaBP " << sauronMcp.p4.at(v).Beta() << "\n";
                std::cout << "costheta " << TMath::Cos(gammadetector.Pos.at(a).Angle(sauronMcp.p4.at(v).Vect())) << std::endl;
                std::cout << "p4.Vect():\t" << sauronMcp.p4.at(v).Vect().Mag() << " " << sauronMcp.p4.at(v).Vect().Theta()/UNITS::deg << " " << sauronMcp.p4.at(v).Vect().Phi()/UNITS::deg << " " << std::endl;
            }
        }

        for(size_t s{0}; s<currentIon.E.size(); s++){
            
            if(coincMatrix.at(v).at(s)){
	      //std::cout<<"true"<<std::endl;
	      currentIon.inCoinc.at(s) = true;
            } else {
	      //std::cout<<"false"<<std::endl;
                currentIon.inCoinc.at(s) = false;
            }

            currentIon.E.at(s)     = gammadetector.P.at(s);
            currentIon.EDC.at(s)   = gammadetector.P.at(s);
            currentIon.EDCBP.at(s) = gammadetector.P.at(s);

            currentIon.EDC.at(s).Boost(-currentIon.p3.BoostVector());
            currentIon.EDCBP.at(s).Boost(-currentIon.p4.BoostVector());
	    if(settings.verbosity_level>1){ 
	      std::cout<<"E "<<currentIon.E.at(s).E()<<std::endl;
	      std::cout<<"EDC "<<currentIon.EDC.at(s).E()<<std::endl;
	      std::cout<<"EDCBP "<<currentIon.EDCBP.at(s).E()<<std::endl;
	      std::cout<<"p3.Mag "<<sauronMcp.p3.at(v).Mag()<<std::endl;
	      std::cout<<"p4.Mag "<<sauronMcp.p4.at(v).Mag()<<std::endl;
	      std::cout<<"boost x "<<currentIon.p3.BoostVector().x()<<" boost y "<<currentIon.p3.BoostVector().y()<<" boost z "<<currentIon.p3.BoostVector().z()<<std::endl;
	    }
        if(settings.verbosity_level>7){
            std::cout<<"AGATA TS "<<gammadetector.TS<<std::endl;
            std::cout<<"Sauron TS "<<sauron.TS<<std::endl;
            std::cout<<"Sauron Ex "<<sauron.Ex.at(v)<<std::endl;
            std::cout<<"Sauron E "<<sauron.E.at(v)<<std::endl;
            std::cout << "Mcpp4.Vect():\t" << sauronMcp.p4.at(v).Vect().x() << " " << sauronMcp.p4.at(v).Vect().y() << " " << sauronMcp.p4.at(v).Vect().z()<< " " << std::endl;
            std::cout << "p4.Vect():\t" << sauron.p4.at(v).Vect().x() << " " << sauron.p4.at(v).Vect().y() << " " << sauron.p4.at(v).Vect().z()<< " " << std::endl;
            std::cout << "p3.Vect():\t" << sauron.p3.at(v).Vect().x() << " " << sauron.p3.at(v).Vect().y() << " " << sauron.p3.at(v).Vect().z()<< " " << std::endl;
            std::cout<<"p3.Mag "<<sauronMcp.p3.at(v).Mag()<<std::endl;
	        std::cout<<"p4.Mag "<<sauronMcp.p4.at(v).Mag()<<std::endl;
	      
            
        }
	    
        }
	
    }
    
    if(settings.verbosity_level>3)
      std::cout<<"-------------------------------------------------------------------------------------"<<std::endl;
}

bool GammadetectorMcpSauronAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    GammadetectorMcpSauronHistograms& h = gammadetectorMcpSauronHistograms;
    if(sauronMcp.sauronMcpAgataGood){
    for(int s{0}; s < static_cast<int>(sauron.E.size()); ++s) {
        for (int a1{0}; a1 < static_cast<int>(gammadetector.E.size()); ++a1) {
    
            if(!gammaDC.at(s).inCoinc.at(a1)) continue;

	    Fill(h.m_E_posX,-mcp.postarget.x(),gammadetector.E.at(a1));
	    Fill(h.m_E_posY,mcp.postarget.y(),gammadetector.E.at(a1));
            Fill(h.h_E, gammadetector.E.at(a1));
            Fill(h.h_EDC, gammaDC.at(s).EDC.at(a1).E());
            Fill(h.h_EDCBP, gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_ESauron_Theta_pg, sauronMcp.p3.at(s).Theta() / UNITS::deg,
                 sauron.E.at(s)); // particle-gamma coincidence

            Fill(h.m_E_Beta, gammaDC.at(s).p3.Beta() * 100,
                 gammadetector.E.at(a1));
            Fill(h.m_E_BetaBP, gammaDC.at(s).p4.Beta() * 100,
                 gammadetector.E.at(a1));

            Fill(h.m_EDC_Beta, gammaDC.at(s).p3.Beta() * 100,
                 gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_EDCBP_BetaBP, gammaDC.at(s).p4.Beta() * 100,
                 gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_E_Theta,
                 sauronMcp.p3.at(s).Angle(gammadetector.P.at(a1).Vect())
                     / UNITS::deg,
                 gammadetector.E.at(a1));

            Fill(h.m_E_ThetaBP,
                 sauron.p4.at(s).Angle(gammadetector.P.at(a1).Vect())
                     / UNITS::deg,
                 gammadetector.E.at(a1));

            Fill(h.m_EDC_Theta,
                 sauronMcp.p3.at(s).Angle(gammadetector.P.at(a1).Vect())
                     / UNITS::deg,
                 gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_EDCBP_ThetaBP,
                 sauron.p4.at(s).Angle(gammadetector.P.at(a1).Vect())
                     / UNITS::deg,
                 gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_EDCBP_Theta,
                 sauronMcp.p3.at(s).Angle(gammadetector.P.at(a1).Vect())
                     / UNITS::deg,
                 gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_SauronID_EDC, sauron.IdPside.at(s),
                 gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_SauronID_EDC, sauron.IdNside.at(s),
                 gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_SauronID_EDCBP, sauron.IdPside.at(s),
                 gammaDC.at(s).EDCBP.at(a1).E());
            Fill(h.m_SauronID_EDCBP, sauron.IdNside.at(s),
                 gammaDC.at(s).EDCBP.at(a1).E());

            Fill(h.m_EDCBP_TS,
                 unsignedDiff<double, unsigned long long>(sauron.TS,
                                                          input.firstTS)
                     / 1.E+8,
                 gammaDC.at(s).EDCBP.at(a1).E()); // Added by MR (be careful!)

            Fill(h.m_Ex_E, sauron.Ex.at(s), gammadetector.E.at(a1));
            Fill(h.m_Ex_EDC, sauron.Ex.at(s), gammaDC.at(s).EDC.at(a1).E());
            Fill(h.m_Ex_EDCBP, sauron.Ex.at(s),
                 gammaDC.at(s).EDCBP.at(a1).E());

	    auto diff = unsignedDiff<double, unsigned long long> (sauronMcp.TS,gammadetector.TS)
	      + (sauronMcp.Time.at(s)-gammadetector.Time.at(a1))/ (10* UNITS::ns);

	    if(settings.verbosity_level>3){
	      std::cout<<"SAURON+MC+AGATA coincidence "<<sauronMcp.sauronMcpAgataGood<<std::endl;
	      std::cout<<"iter "<<s<<" "<<a1<<std::endl;
	    }
	    
	    Fill(h.m_E_TSmTS_cfd,diff,gammadetector.E.at(a1) );
	    Fill(h.m_EDC_TSmTS_cfd,diff,gammaDC.at(s).EDC.at(a1).E() );
	    
            for(auto& it : h.exSelection) {
                GammadetectorMcpSauronHistograms::ExSelection& hex
                    = it; // Shadowing other reference!
                if((sauron.Ex.at(s) > hex.vals.first)
                   && (sauron.Ex.at(s) < hex.vals.second)) {
                    Fill(hex.m_E_Beta, gammaDC.at(s).p3.Beta() * 100,
                         gammadetector.E.at(a1));
                    Fill(hex.m_E_BetaBP, gammaDC.at(s).p4.Beta() * 100,
                         gammadetector.E.at(a1));
                    Fill(hex.h_E, gammadetector.E.at(a1));
                    Fill(hex.h_EDC, gammaDC.at(s).EDC.at(a1).E());
                    Fill(hex.h_EDCBP, gammaDC.at(s).EDCBP.at(a1).E());
		    Fill(hex.m_EDC_TSmTS_cfd,diff,gammaDC.at(s).EDC.at(a1).E() );
		    
                    //		    Fill(hex.m_SauronID_Ex, sauron.Id.at(s),
                    // sauron.Ex.at(s));

                    Fill(hex.m_E_Theta,
                         sauronMcp.p3.at(s).Angle(gammadetector.P.at(a1).Vect())
                             / UNITS::deg,
                         gammadetector.E.at(a1));
                    Fill(hex.m_E_ThetaBP,
                         sauron.p4.at(s).Angle(gammadetector.P.at(a1).Vect())
                             / UNITS::deg,
                         gammadetector.E.at(a1));

                    Fill(hex.m_EDCBP_Theta,
                         sauronMcp.p3.at(s).Angle(gammadetector.P.at(a1).Vect())
                             / UNITS::deg,
                         gammaDC.at(s).EDCBP.at(a1).E());
                    Fill(hex.m_EDCBP_ThetaBP,
                         sauron.p4.at(s).Angle(gammadetector.P.at(a1).Vect())
                             / UNITS::deg,
                         gammaDC.at(s).EDCBP.at(a1).E());

                }
            }

        }
	if(settings.verbosity_level>3){
	  std::cout<<"iter "<<s<<std::endl;
	  std::cout<<"sauron size "<<sauron.E.size()<<std::endl;
	}
	auto diff2 = unsignedDiff<double, unsigned long long> (sauronMcp.TS,gammadetector.TS)
	  + (sauronMcp.Time.at(s)-gammadetector.Time.at(0))/ (10* UNITS::ns);
	Fill(h.m_Ex_TSmTS,diff2,sauronMcp.Ex.at(s) );
	
    }

    // Particle identification histograms
    std::vector<ReactionFragment::Ion> reaction;
    for(unsigned int e{0}; e < sauron.E.size(); ++e) {
        if(!sauron.identified.at(e)) { continue; }

        ReactionFragment::Ion idIon{sauron.A.at(e), sauron.Z.at(e)};
        reaction.emplace_back(idIon);
    }
    std::sort(reaction.begin(), reaction.end());

    auto reactionHist = h.reactionHistos.find(reaction);
    if(reactionHist != h.reactionHistos.end()) {
        for(unsigned int s{0}; s < gammaDC.size(); ++s) {
            for(unsigned int a{0}; a < gammaDC.at(s).E.size(); ++a) {
                Fill(reactionHist->second.h_E, gammadetector.E.at(a));
                Fill(reactionHist->second.h_EDCBP,
                     gammaDC.at(s).EDCBP.at(a).E());
                Fill(reactionHist->second.m_Ex_E, gammadetector.E.at(a),
                     sauron.Ex.at(s));
                Fill(reactionHist->second.m_Ex_EDCBP,
                     gammaDC.at(s).EDCBP.at(a).E(), sauron.Ex.at(s));
            }
        }
        
        for(unsigned int e{0}; e < sauron.E.size(); ++e) {
            if(!sauron.identified.at(e)) { continue; }
            Fill(reactionHist->second.m_SauronID_Ex, sauron.IdPside.at(e),
                sauron.Ex.at(e));
            Fill(reactionHist->second.m_SauronID_Ex, sauron.IdNside.at(e),
                sauron.Ex.at(e));
        }
    }
    }
    return true;
}

void GammadetectorMcpSauronAnalysis::Finalize() {
    CoincidenceAnalysis::Finalize();
}

TGraph* GammadetectorMcpSauronAnalysis::GenerateKinematicLine(const double& val) {
    auto reactionsIt = input.reactions.find(sauronConf.ion);
    auto reactionCopy = reactionsIt->second;
    reactionCopy.GetFreeFragment().Set_Ex(val * UNITS::MeV);

    std::vector<double> x, y;
    double              thetaMax
        = static_cast<double>(reactionCopy.Get_ThetaMax()) / UNITS::deg;
    x.reserve(180);
    y.reserve(180);

    for(int p{1}; p < thetaMax; ++p) {
        try {
            reactionCopy.Set_Theta(p * UNITS::deg, true);
        } catch(const std::runtime_error& e) {
            std::cout << e.what();
            // Do not worry, reaction calculations sometimes are not enough
            // precise
        };
        x.push_back(p);
        y.push_back(static_cast<double>(
            reactionCopy.GetFixedFragment().Get_Ek()));
    }
    if(thetaMax < 180) {
        for(int p{static_cast<int>(thetaMax)}; p > 0; --p) {
            try {
                reactionCopy.Set_Theta(p * UNITS::deg, false);
            } catch(const std::runtime_error& e) {
                continue;
                // Do not worry, reaction calculations sometimes are not enough
                // precise
            };
            x.push_back(p);
            y.push_back(static_cast<double>(
                reactionCopy.GetFixedFragment().Get_Ek()));
        }
    }
    auto* gr = new TGraph(static_cast<int>(x.size()), &x[0], &y[0]);
    gr->SetName(Form("g_Energy_Theta_%.3f_%i", val, kineLineCounter++));
    gr->SetTitle(
        Form("Kinematic line with Ex = %.3f;Theta [deg];Energy [MeV]", val));
    return gr;
}
