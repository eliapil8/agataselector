#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    std::string sRun = fChain->GetDirectory()->GetName();
    sRun.erase(sRun.begin(), sRun.begin() + sRun.find("run_") + 4);
    sRun.erase(sRun.begin() + 4, sRun.end());

    runNr = std::stoi(sRun);

    // initTS = AgataSelector::GetFirstTS();
    // oldTS  = initTS;
    thr   = conf.user.threshold_rate * timeBin;
    outFile.mkdir("User", "User's custom analysis");
    outFile.cd("User");

    UserHistograms& hu = userHistograms;

    hu.h_noDC_beam
        = new TH1D("h_noDC_beam",
                   "Non-DC activation spectrum"
                   ";Energy [keV];Counts;",
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV);

    hu.h_noDC_decay
        = new TH1D("h_noDC_decay",
                   "Non-DC decay spectrum"
                   ";Energy [keV];Counts;",
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV);

    hu.m_noDC_noDC_beam
        = new TH2D("h_noDC_noDC_beam",
                   "gamma-gamma activation spectrum"
                   ";Energy [keV];Energy [keV] ;Counts;",
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV,
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV);

    hu.m_noDC_noDC_decay
        = new TH2D("h_noDC_noDC_decay",
                   "gamma-gamma decay spectrum"
                   ";Energy [keV];Energy [keV] ;Counts;",
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV,
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV);

    hu.m_noDC_decay_time
        = new TH2D("h_noDC_decay_time",
                   "Non-DC decay spectrum;"
                   "Time [s];Energy [keV];Counts;",
                   4000, 0, 3 * 60,
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV);

    hu.m_noDC_beam_time
        = new TH2D("h_noDC_beam_time",
                   "Non-DC beam spectrum;"
                   "Time [s];Energy [keV];Counts;",
                   4000, 0, 3 * 60,
                   static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                    / conf.agata.binWidth),
                   0, conf.agata.gammaEmax / UNITS::keV);

    hu.h_CR = new TH1D("h_CD",
                       "Counts in bin"
                       ";Counts in bin;Counts;",
                       1000, 0, 40000);

    //     hu.h_tmp = new TH1D("h_tmp","h_tmp"=
    //              static_cast<int>(conf.agata.gammaEmax / UNITS::keV
    //                               / conf.agata.binWidth),
    //              0, conf.agata.gammaEmax / UNITS::keV);
    //
    //    hu.m_noDC_noDC_beam=
    //    new TH2D("m_noDC_noDC_beam",
    //             "gamma-gamma non-DC activation spectrum"
    //             ";Energy [keV];Energy [keV];",
    //             static_cast<int>(conf.agata.gammaEmax / UNITS::keV
    //                              / conf.agata.binWidth),
    //             0, conf.agata.gammaEmax / UNITS::keV,
    //             static_cast<int>(conf.agata.gammaEmax / UNITS::keV
    //                              / conf.agata.binWidth),
    //             0, conf.agata.gammaEmax / UNITS::keV),
    //
    //
    //    hu.m_noDC_noDC_decay=
    //    new TH2D("m_noDC_noDC_decay",
    //             "gamma-gamma non-DC decay spectrum"
    //             ";Energy [keV];Energy [keV];",
    //             static_cast<int>(conf.agata.gammaEmax / UNITS::keV
    //                              / conf.agata.binWidth),
    //             0, conf.agata.gammaEmax / UNITS::keV,
    //             static_cast<int>(conf.agata.gammaEmax / UNITS::keV
    //                              / conf.agata.binWidth),
    //             0, conf.agata.gammaEmax / UNITS::keV),
    //
    // hu.m_tmp = new TH2D("m_tmp","m_tmp"=
    //             static_cast<int>(conf.agata.gammaEmax / UNITS::keV
    //                              / conf.agata.binWidth),
    //             0, conf.agata.gammaEmax / UNITS::keV,
    //             static_cast<int>(conf.agata.gammaEmax / UNITS::keV
    //                              / conf.agata.binWidth),
    //             0, conf.agata.gammaEmax / UNITS::keV);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry
    UserHistograms& hu = userHistograms;
    if(agataOutput.TS == 0) return kTRUE;
    if(binStartTime == 0) binStartTime = agataOutput.TS;
    if(startIrradTime == 0) startIrradTime = agataOutput.TS;

    double timeSinceBinStart   = (agataOutput.TS - binStartTime) / 1.e8;
    double timeSinceIrradStart = (agataOutput.TS - startIrradTime) / 1.e8;
    double timeSinceDecayStart = (agataOutput.TS - startDecayTime) / 1.e8;

    if(timeSinceBinStart > timeBin) {
        if(counter > thr) {
            if(decay == true && timeSinceDecayStart > minDecTime) {
                startIrradTime = agataOutput.TS;
                decay          = false;
            }
        } else {
            if(decay == false && timeSinceIrradStart > minIrradTime) {
                startDecayTime = agataOutput.TS;
                decay          = true;
            }
        }

        hu.h_CR->Fill(counter / timeBin);
        counter      = 0;
        binStartTime = agataOutput.TS;
    }

    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        ++counter;

        if(decay) {
            hu.h_noDC_decay->Fill(agataOutput.E.at(i));
            hu.m_noDC_decay_time->Fill(timeSinceDecayStart,
                                       agataOutput.E.at(i));
            for(unsigned int j{0}; j < i; ++j) {
                hu.m_noDC_noDC_decay->Fill(agataOutput.E.at(i),
                                           agataOutput.E.at(j));
                hu.m_noDC_noDC_decay->Fill(agataOutput.E.at(j),
                                           agataOutput.E.at(i));
            }
        } else {
            hu.h_noDC_beam->Fill(agataOutput.E.at(i));
            hu.m_noDC_beam_time->Fill(timeSinceIrradStart,
                                      agataOutput.E.at(i));
            for(unsigned int j{0}; j < i; ++j) {
                hu.m_noDC_noDC_beam->Fill(agataOutput.E.at(i),
                                          agataOutput.E.at(j));
                hu.m_noDC_noDC_beam->Fill(agataOutput.E.at(j),
                                          agataOutput.E.at(i));
            }
        }
    }
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
    UserHistograms& hu = userHistograms;
}
