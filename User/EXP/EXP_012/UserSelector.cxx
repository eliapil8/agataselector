#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    UserHistograms& hu = userHistograms;
    hu.m_EDC_crystalID
        = new TH2D("m_EDC_crystalID", "EDC vs crystal ID;ID;EDC [keV]", 50,
                   -0.5, 50 - 0.5, 6000, 0, 6000);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry

    // AgataHistograms& ha = agataHistograms;
    // AgataPrismaHistograms& hap = agataPrismaHistograms;
    UserHistograms& hu = userHistograms;
    if(prismaOutput.beta.Mag2() > 0) {
        for(unsigned int i{0}; i < agataOutput.P.size(); ++i) {
            TLorentzVector pDC = agataOutput.P[i];

            pDC.Boost(-prismaOutput.beta);
            hu.m_EDC_crystalID->Fill(agataOutput.CoreID[i], pDC.E());
        }
    }
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
