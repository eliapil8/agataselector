#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file
    std::string sRun = fChain->GetDirectory()->GetName();
    // std::cout << sRun << std::endl;
    sRun.erase(sRun.begin(), sRun.begin() + sRun.find("run_") + 4);
    sRun.erase(sRun.begin() + 4, sRun.end());

    // std::cout << sRun << std::endl;
    runNr  = std::stoi(sRun);
    // std::cout << runNr << std::endl;

    initTS = AgataSelector::GetFirstTS();
    oldTS  = initTS;
    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    UserHistograms& hu = userHistograms;

    hu.h_alive_time    = new TH1D("h_alive_time",
                                  "Duration of beam on target"
                                     ";Run;Alive Time [s]",
                                  100, 0, 100);

    hu.h_spiderCR      = new TH1D(Form("h_spiderCR_run-%i", runNr),
                                  Form("SPIDER count rate for run %i"
                                            ";Time [s];CR [Hz]",
                                       runNr),
                                  3200 * 6, 0, 3200 * 6);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry
    UserHistograms& hu = userHistograms;

    if(spiderOutput.TS > 0) {
        ++counter;
        auto diff = unsignedDiff<double, unsigned long long>(spiderOutput.TS,
                                                             initTS);
        hu.h_spiderCR->Fill(diff / 1.e8);
        if((spiderOutput.TS - oldTS) / 1.e8 > 1) {
            if(counter > 500) { hu.h_alive_time->Fill(runNr); }
            counter = 0;
            oldTS   = spiderOutput.TS;
        }
    }
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
