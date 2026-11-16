#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

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

    hu.h_Pt_production = new TH1D("h_Pt_production",
                                  "Count Rate in 198Pt 407 keV"
                                  "line per minute;Time[min];Counts per min",
                                  10080, 0, 10080);

    hu.m_mat           = new TH2D("mat", "mat", 180, 0, 180, 1000, 0, 2000);

    hu.histo           = new TH1D("histo", "histo", 90, 0, 90);
    hu.dist            = new TH1D("dist", "dist", 2000, 0, 2000);

    /*    Initialize<TH1D>( hu.h_196Os_noDC,
                          new TH1D( "h_196Os_noDC","196Os;E [keV];Counts",
                                    static_cast<int>(conf.agata.gammaEmax /
       UNITS::keV / conf.agata.binWidth), 0, conf.agata.gammaEmax /
       UNITS::keV), hu.ptrs);

        Initialize<TH2D>( hu.m_196Os_noDC_thetaBP,
                          new TH2D( "m_196Os_noDC_thetaBP","196Os;ThetaBP
       [deg];E [keV];Counts", 180,0,180, static_cast<int>(conf.agata.gammaEmax
       / UNITS::keV / conf.agata.binWidth), 0, conf.agata.gammaEmax /
       UNITS::keV), hu.ptrs);
/

    gDirectory->mkdir("A_over_q_E", "A_over_q_E")->cd();

    for(const auto& aq : conf.prisma.aqConfs) {
        gDirectory->mkdir(Form("Z_%i", aq.Z))->cd();
        std::vector<TH2D*> matr_vec;
        for(int i = aq.qmin; i <= aq.qmax; i++) {
            matr_vec.push_back(nullptr);
            Initialize<TH2D>(
                matr_vec.back(),
                new TH2D(Form("m_Aoverq_EDC_%i_%i", aq.Z, i),
                         Form("A/q : EDC (Z = %i, q = %i)", aq.Z, i), 1000,
                         100, 500,
                         static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                          / conf.agata.binWidth),
                         0, conf.agata.gammaEmax / UNITS::keV),
                hu.ptrs);
        }
        hu.m_Aoverq_E.push_back(matr_vec);
        gDirectory->cd("..");
    }
*/
    /*   for( int j = 0; j < z_count; ++j) {
           unsigned int z = j + conf.prisma.Zrange.first;
           hu.m_Aoverq_E.push_back( new TH2D(Form("m_Aoverq_E_%i", z),
                                    Form("A/q (Z = %i) vs E", z),
        }*/
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    UserHistograms& hu = userHistograms;

    if(prismaOutput.TS > 0) {
        ++counter;
        if((prismaOutput.TS - oldTS) / 1.e8 > 1) {
            if(counter > 80) { hu.h_alive_time->Fill(runNr); }
            counter = 0;
            oldTS   = prismaOutput.TS;
        }
    }

    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
        auto diff
            = unsignedDiff<double, unsigned long long>(agataOutput.TS, initTS);

        if(diff < conf.agataPrisma.windowLeft
           && diff > conf.agataPrisma.windowRight) {
            continue;
        }
        if(agataOutput.E[i] < 412 && agataOutput.E[i] > 400) {
            hu.h_Pt_production->Fill(diff / 1.e8 / 60);
        }
        if(agataOutput.E[i] < 350 && agataOutput.E[i] > 338) {
            hu.h_Pt_production->Fill(diff / 1.e8 / 60, -1);
        }

        // The coincidence with BP angle
        // Fill(histoIt->second.m_noDC_thetaBP_ion,
        //             gammaDC.p4.Angle(gammadetector.P.at(i).Vect()) /
        //             UNITS::deg, gammadetector.E.at(i));

        if(prismaOutput.Z == 54 && prismaOutput.A > 135) {
            auto angbp = prismaOutput.p3.Theta() / UNITS::deg;
            if(agataOutput.E.at(i) > 400 && agataOutput.E.at(i) < 412)
                hu.dist->Fill(
                    120.0 / sin(prismaOutput.p4.Theta() - 40 * UNITS::deg));
            if(angbp > 40.0 && angbp < 45.0) {
                auto boh = prismaOutput.p4.Angle(agataOutput.P.at(i).Vect())
                         / UNITS::deg;
                hu.m_mat->Fill(boh, agataOutput.E.at(i));
                hu.histo->Fill(prismaOutput.p4.Theta() / UNITS::deg);
            }
        }
    }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
