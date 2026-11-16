#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    // UserHistograms& hu = userHistograms;
    UserHistograms& hu = userHistograms;
//<<<<<<< HEAD
    hu.m_Ep_Eg = new TH2D("m_Ep_Eg","Particle ene vs gamma en;"
			"Particle Ene [MeV];Gamma Ene [keV]",3000, 0, 30, 3000, 0, 3000);
    hu.m_Ep_Eg_2r = new TH2D("m_Ep_Eg_2r","Particle ene vs gamma en;" 
                        "Particle Ene [MeV];Gamma Ene [keV]",3000, 0, 30, 3000, 0, 3000);
    hu.m_Ep_Eg_1r = new TH2D("m_Ep_Eg_1r","Particle en vs gamma en;"
                         "Particles E (MeV);Gamma E (keV)",3000,0,30,3000,0,3000);
//=======
    hu.m_Ep_Eg         = new TH2D("m_Ep_Eg",
                                  "Particle ene vs gamma en;"
                                          "Particle Ene [MeV];Gamma Ene [keV]",
                                  3000, 0, 30, 3000, 0, 3000);
    hu.m_Ep_Eg_1r      = new TH2D("m_Ep_Eg_1r",
                                  "Particle en vs gamma en;"
                                       "Particles E (MeV);Gamma E (keV)",
                                  3000, 0, 30, 3000, 0, 3000);
//>>>>>>> 5464ddad6d7e7ddd322011f4f140c790fdfd6a2f
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);
    UserHistograms& hu = userHistograms;

    for(int s{0}; s < sauronFwOutput.E.size(); ++s) {
        if(gammaAgataSauronFw.size() == 0) break;
        for(const auto& agaE : gammaAgataSauronFw.at(s).EDCBP) {
            hu.m_Ep_Eg->Fill(sauronFwOutput.ESi.at(s), agaE.E());
        }
        if(sauronFwOutput.Pos.at(s).Theta() > 14 * UNITS::deg) continue;
        for(const auto& agaE : gammaAgataSauronFw.at(s).EDCBP) {
            hu.m_Ep_Eg_1r->Fill(sauronFwOutput.ESi.at(s), agaE.E());
        }
    }
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
