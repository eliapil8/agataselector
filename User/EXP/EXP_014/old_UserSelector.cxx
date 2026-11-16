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

    hu.h_Pt_production,
        new TH1D("h_Pt_production",
                 "Count Rate in 198Pt 407 keV"
                 "line per minute;Time[min];Counts per min",
                 10080, 0, 10080),

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

    // User process - done for each entry

    // AgataHistograms& ha = agataHistograms;
    // AgataPrismaHistograms& hap = agataPrismaHistograms;
    UserHistograms& hu = userHistograms;

    if(prisma.TS > 0) {
        ++counter;
        if((prisma.TS - oldTS) / 1.e8 > 1) {
            if(counter > 80) { hu.h_alive_time->Fill(runNr); }
            counter = 0;
            oldTS   = prisma.TS;
        }
    }

    for(unsigned int i{0}; i < agata.E.size(); ++i) {
        if(!agata.prisma_coinc[i]) { continue; }
        if(agata.E[i] < 412 && agata.E[i] > 400) {
            hu.h_Pt_production->Fill(
                AgataSelector::unsignedDiff<double, unsigned long long>(
                    agata.TS, initTS)
                / 1.e8 / 60);
        }
        if(agata.E[i] < 350 && agata.E[i] > 338) {
            hu.h_Pt_production->Fill(
                AgataSelector::unsignedDiff<double, unsigned long long>(
                    agata.TS, initTS)
                    / 1.e8 / 60,
                -1);
        }
    }
    /*        for(unsigned int con = 0; con < conf.prisma.aqConfs.size();
       con++) { const PrismaConf::AqConf& currentConf =
       conf.prisma.aqConfs.at(con); for(int j = 0; j <= currentConf.qmax -
       currentConf.qmin; j++) { if(**prismaReader.Z_Nr == currentConf.Z
                       && **prismaReader.q_Nr == j + currentConf.qmin) {
                        if(!hu.m_Aoverq_E.empty()) {
                            TLorentzVector        pDC = agataOutput.P[i];
                            ReactionFragment::Ion ion{prismaOutput.A,
                                                      prismaOutput.Z};
                            TLorentzVector        ps3 = SlowInTarget(
                                prismaOutput.p3, conf.agataPrisma.dcPos, ion);
                            pDC.Boost(-ps3.BoostVector());
                            Fill(hu.m_Aoverq_E.at(con).at(j),
                                 **prismaReader.A_over_q, pDC.E());
                        }
                    }
                }
            }
    */
    /*
            if( prismaOutput.Z == 56 && (
                (prismaOutput.Q == 41 &&
       TMath::Abs(**prismaReader.A_over_q-359.)<1.2) || (prismaOutput.Q ==
       42 && TMath::Abs(**prismaReader.A_over_q-360.)<1.2) ||
       (prismaOutput.Q
       == 43 && TMath::Abs(**prismaReader.A_over_q-348.)<1.2) ||
                (prismaOutput.Q == 44 &&
       TMath::Abs(**prismaReader.A_over_q-344.)<1.2) || (prismaOutput.Q ==
       45 && TMath::Abs(**prismaReader.A_over_q-334.)<1.2) ||
       (prismaOutput.Q
       == 46 && TMath::Abs(**prismaReader.A_over_q-325.)<1.2) )) {

                TLorentzVector pSlow;
                TLorentzVector pBPslow;
                double exE{-1};
                ReactionFragment::Ion forcedIon{138, 56};
                unsigned int A2 = conf.reaction.target.A +
       conf.reaction.beam.A - forcedIon.A; unsigned int Z2 =
       conf.reaction.target.Z + conf.reaction.beam.Z - forcedIon.Z;
                ReactionFragment::Ion ion4{A2,Z2};

                auto reactionsIt = reactions.find(forcedIon);
                if(reactionsIt != reactions.end()) {
                    // Evaluate mid-target energy
                    auto ion3 = reactionsIt->second
                                    .GetFixedFragment();
                    ion3.Set_Beta(prismaOutput.beta);
                    TLorentzVector p = ion3.Get_P();
                    EvalInMidTarget(p, forcedIon);

                    exE = static_cast<double>(
                        reactionsIt->second.Set_E_Theta_Phi(
                            p.E(), p.Theta(), p.Phi()));
                    TLorentzVector pBP
                        = reactionsIt->second.GetFreeFragment()
                              .Get_P();

                    pSlow   = SlowInTarget(p, conf.agataPrisma.dcPos,
                                           forcedIon);
                    pBPslow = SlowInTarget(
                        pBP, conf.agataPrisma.bpDcPos, ion4);
                }
                Fill(hu.h_196Os_noDC, agataOutput.E[i]);
                Fill(hu.m_196Os_noDC_thetaBP,
                     pBPslow.Angle(agataOutput.P.at(i).Vect()) /
       UNITS::deg, agataOutput.E[i]);
            }
     */
    /*for(int j = 0; j < z_count; ++j) {
        unsigned int z = j + conf.prisma.Zrange.first;
        if(**prismaReader.Z_Nr == z ) {
            Fill( hu.m_Aoverq_E.at(j),
              **prismaReader.A_over_q,  agataOutput.E[i]);
        }
    }*/
    //}
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
