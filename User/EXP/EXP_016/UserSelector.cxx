#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    std::string sRun = fChain->GetDirectory()->GetName();
    sRun.erase(sRun.begin(), sRun.begin() + sRun.find("run_") + 4);
    sRun.erase(sRun.begin() + 4, sRun.end());

    runNr  = std::stoi(sRun);

    initTS = AgataSelector::GetFirstTS();
    oldTS  = initTS;
    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    UserHistograms& hu = userHistograms;

    hu.h_alive_time    = new TH1D("h_alive_time",
                                  "Duration of beam on target"
                                     ";Run;Alive Time [s]",
                                  100, 0, 100);
    hu.h_avgCR         = new TH1D("h_avgCR",
                                  "Average CR of prisma"
                                          ";Run;AVG CR [s-1]",
                                  100, 0, 100);
    hu.h_LabrID = new TH1D("h_LabrID","LaBr3 ID; ID",10,16,26);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry
    UserHistograms& hu = userHistograms;

    for(int i{0}; i<labrOutput.Id.size();++i){
      hu.h_LabrID->Fill(labrOutput.Id.at(i));
    }
    
    if(prismaOutput.TS > 0) {
      ++counter;
      if((prismaOutput.TS - oldTS) / 1.e8 > 1.) {
	if(counter > 70) {
	  totCR += counter;
	  ++aliveT;
	}
	counter = 0;
	oldTS   = prismaOutput.TS;
      }
    }
    
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
    UserHistograms& hu = userHistograms;

    hu.h_alive_time->Fill(runNr, aliveT);
    hu.h_avgCR->Fill(runNr, totCR / aliveT);

   
}
