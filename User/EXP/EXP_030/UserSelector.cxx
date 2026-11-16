#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    // outFile.mkdir("User","User's custom analysis");
    // outFile.cd("User");

    // UserHistograms& hu = userHistograms;
    // hu.h_agataCR = new TH1D("h_agataCR","Count Rate per minute of
    // AGATA;Time[min];Counts per min",10080,0,10080);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry

    // AgataHistograms& ha = agataHistograms;
    // AgataPrismaHistograms& hap = agataPrismaHistograms;
    // UserHistograms& hu = userHistograms;

    // double timeDiff = 0;
    // if(agataOutput.TS >= timestampRangeAgata.first){
    //     timeDiff = (double)(agataOutput.TS - timestampRangeAgata.first)
    //     /1.e8/60.; for(unsigned int i{0}; i<agataOutput.E.size(); i++){
    //        Fill(hu.h_agataCR, timeDiff);
    //     }
    // }
    // else {
    //     std::cout << "Something wrong with time stamps; lower time than
    //     reference found " << std::endl;
    // }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
