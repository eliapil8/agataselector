#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User begin
    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    std::ifstream tsFile("TSreference.dat");
    int           runNr = 0;
    std::string   sTemp = outFile.GetName();
    sTemp.erase(sTemp.begin(), sTemp.begin() + sTemp.find(".root") - 9);
    sTemp.erase(sTemp.end() - 10, sTemp.end());
    int fileRun = std::stoi(sTemp);

    while(tsFile >> runNr >> tsRef >> tsOff) {
        if(runNr == fileRun) break;
    }
    std::cout << "initial time stamp: " << tsRef << " -- offset " << tsOff
              << std::endl;
    tsFile.close();

    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    UserHistograms& hu = userHistograms;
    hu.h_cr_z16        = new TH1D(
        "h_cr_z16",
        "Count Rate per minute in Z=16 banana;Time[min];Counts per min", 10080,
        0, 10080);
    hu.h_cr_z15 = new TH1D(
        "h_cr_z15",
        "Count Rate per minute in Z=15 banana;Time[min];Counts per min", 10080,
        0, 10080);
    hu.h_cr_z14 = new TH1D(
        "h_cr_z14",
        "Count Rate per minute in Z=14 banana;Time[min];Counts per min", 10080,
        0, 10080);
    hu.h_cr_zRatio14over16 = new TH1D("h_cr_zRatio14over16",
                                      "Count Rate per minute in Z=14 divided "
                                      "by Z=16;Time[min];Counts per min",
                                      10080, 0, 10080);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process
    // AgataPrismaHistograms& h  = agataPrismaHistograms;
    UserHistograms& hu = userHistograms;

    // if(prismaOutput.is_agata_coincidence) {
    if(1) {
        double timeDiff = 0;
        if(agataOutput.TS >= tsRef)
            timeDiff = (double)(agataOutput.TS - tsRef + tsOff) / 1.e8 / 60.;
        else {
            std::cout
                << "Something wrong with TS; lower time than reference found "
                << agataOutput.TS << " ... using new value ..." << std::endl;
            //         tsOff = tsOld-tsRef;
            //         tsRef = tsCorr;
        }
        for(unsigned int i{0}; i < agataOutput.E.size(); i++) {
            // for(const auto& it : h.cutHistos) {
            //     if(it.cut->IsInside(prismaOutput.E, prismaOutput.dE)) {
            //         if(strcmp(it.cut->GetName(), "z16") == 0)
            //            hu.h_cr_z16->Fill( timeDiff);
            //         else if(strcmp(it.cut->GetName(), "z15") == 0)
            //            hu.h_cr_z15->Fill( timeDiff);
            //         else if(strcmp(it.cut->GetName(), "z14") == 0)
            //            hu.h_cr_z14->Fill( timeDiff);
            //     }
            // }
        }
    }

    if(agataOutput.TS > 0) tsOld = agataOutput.TS;

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
    UserHistograms& hu = userHistograms;
    for(int i{1}; i <= hu.h_cr_z14->GetNbinsX(); ++i) {
        if(hu.h_cr_z16->GetBinContent(i) == 0)
            hu.h_cr_zRatio14over16->SetBinContent(i, 0);
        else
            hu.h_cr_zRatio14over16->SetBinContent(
                i,
                hu.h_cr_z14->GetBinContent(i) / hu.h_cr_z16->GetBinContent(i));
    }
}
