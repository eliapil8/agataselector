#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User begin

    UserHistograms& h = userHistograms;
    h.dir             = outFile->mkdir("User", "User histos");
    h.dir->cd();

    h.coincs.push_back(
        UserHistograms::Coincidence("60Co", 1168, 1178, 1327, 1337));
    h.coincs.push_back(
        UserHistograms::Coincidence("152Eu", 1401, 1412, 118, 125));
    h.coincs.push_back(
        UserHistograms::Coincidence("152Eu", 241, 248, 118, 125));
    h.coincs.push_back(
        UserHistograms::Coincidence("152Eu", 775, 782, 341, 347));

    /*
        h.coincs.push_back(UserHistograms::Coincidence("60Co",1165,1180,1320,1340));
        h.coincs.push_back(UserHistograms::Coincidence("152Eu",1398,1413,117,125));
        h.coincs.push_back(UserHistograms::Coincidence("152Eu",240,250,117,125));
        h.coincs.push_back(UserHistograms::Coincidence("152Eu",773,783,339,349));
      */

    for(unsigned int k = 0; k < h.coincs.size(); ++k) {
        h.coincs[k].dir = h.dir->mkdir(
            Form("Coinc_%s_%3.0f_%3.0f", h.coincs[k].Name.c_str(),
                 h.coincs[k].Egamma1, h.coincs[k].Egamma2));

        h.coincs[k].dir->cd();

        h.coincs[k].Initialize();
    }
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process

    UserHistograms& h = userHistograms;

    for(unsigned int k = 0; k < h.coincs.size(); ++k) {
        h.coincs[k].Compute(agataReader, conf);
    }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate

    UserHistograms& h = userHistograms;

    for(auto it : h.coincs) { it.UncorrProcess(); }
}
