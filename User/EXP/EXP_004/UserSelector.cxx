#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User begin
    UserHistograms& h = userHistograms;

    h.dir             = outFile.mkdir("User", "User histos");
    h.dir->cd();

    h.ptrs.push_back(h.h_TSdiff_z17_561keV
                     = new TH1D("h_TSdiff_z17_561keV",
                                "TS difference Prisma-Agata gated on z=17 and "
                                "E around 561 keV;Time[10ns];Counts/10ns",
                                200, -200, 0));

    h.dir->mkdir("z14", "Z=14 histos")->cd();

    h.ptrs.push_back(
        h.h_Beta_Prisma_z14
        = new TH1D("h_Beta_Prisma_z14", "h_Beta_Prisma_z14", 200, 0, 0.2));

    h.ptrs.push_back(h.m_Eg_Theta_Z16_A32
                     = new TH2D("m_Eg_Theta_Z16_A32", "m_Eg_Theta_Z16_A32",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EDC_Theta_Z16_A32
                     = new TH2D("m_EDC_Theta_Z16_A32", "m_EDC_Theta_Z16_A32",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EBP_Theta_Z16_A32
                     = new TH2D("m_EBP_Theta_Z16_A32", "m_EBP_Theta_Z16_A32",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EDCBP_Theta_Z16_A32 = new TH2D(
                         "m_EDCBP_Theta_Z16_A32", "m_EDCBP_Theta_Z16_A32", 360,
                         0, 180, 4000, 1, 4001));

    h.ptrs.push_back(h.m_Theta_p3p4_z17
                     = new TH2D("m_Theta_p3p4_z17", "m_Theta_p3p4_z17", 360, 0,
                                180, 360, 0, 180));
    h.ptrs.push_back(h.m_Theta_Beta_p3_z17
                     = new TH2D("m_Theta_Beta_p3_z17", "m_Theta_Beta_p3_z17",
                                360, 0, 180, 200, 0, 0.20));
    h.ptrs.push_back(h.m_Theta_Beta_p4_z17
                     = new TH2D("m_Theta_Beta_p4_z17", "m_Theta_Beta_p4_z17",
                                360, 0, 180, 200, 0, 0.20));
    h.ptrs.push_back(h.m_TKE_E_Z17_A35
                     = new TH2D("m_TKE_E_Z17_A35", "m_TKE_E_Z17_A35", 500,
                                -100, 400, 4000, 1, 4001));
    h.ptrs.push_back(h.m_TKE_EDC_Z17_A35
                     = new TH2D("m_TKE_EDC_Z17_A35", "m_TKE_EDC_Z17_A35", 500,
                                -100, 400, 4000, 1, 4001));
    h.ptrs.push_back(h.m_TKE_EDCBP_Z17_A35
                     = new TH2D("m_TKE_EDCBP_Z17_A35", "m_TKE_EDCBP_Z17_A35",
                                500, -100, 400, 4000, 1, 4001));
    h.ptrs.push_back(h.m_MCPXY_z17
                     = new TH2D("m_MCPXY_z17", "m_MCPXY_z17", 1000, -100, 100,
                                1000, -100, 100));
    h.ptrs.push_back(
        h.h_EDC_bavg
        = new TH1D("h_EDC_bavg", "EDC Beta avg with prisma", 3000, 0, 3000));
    h.ptrs.push_back(h.m_EDC_EDC_bavg = new TH2D(
                         "m_EDC_EDC_bavg", "EDC EDC Beta avg gg with prisma",
                         3000, 0, 3000, 3000, 0, 3000));

    h.ptrs.push_back(h.m_EDCscan = new TH2D("m_EDCscan", "beta scan", 100,
                                            0.01, 0.04, 3000, 0, 3000));

    h.dir->mkdir("z16", "Z=16 histos")->cd();
    h.ptrs.push_back(h.h_EDC_bavg_z16
                     = new TH1D("h_EDC_bavg_z16", "EDC Beta avg with prisma",
                                3000, 0, 3000));

    h.ptrs.push_back(h.m_EDCscan_z16
                     = new TH2D("m_EDCscan_z16", "beta scan", 100, 0.01, 0.04,
                                3000, 0, 3000));

    // betavg.SetXYZ(0.021941,0.000000,0.020460);
    betavg.SetMagThetaPhi(0.017, 47 * TMath::DegToRad(), 0.);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process
    UserHistograms& h = userHistograms;

    if(conf.detectorsPresent.Agata && conf.detectorsPresent.Prisma
       && prismaOutput.TS != 0 && agataOutput.TS != 0) {
        unsigned long long udiff = prismaOutput.TS > agataOutput.TS
                                     ? prismaOutput.TS - agataOutput.TS
                                     : agataOutput.TS - prismaOutput.TS;
        long long diff = prismaOutput.TS > agataOutput.TS ? udiff : -udiff;

        // for(const auto& it : agataPrismaHistograms.cutHistos) {
        //     // z=14 gate
        //     // //////////////////////////////////////////////////////////////

        //    if(it.cut->IsInside(prismaOutput.E, prismaOutput.dE)
        //       && strcmp(it.cut->GetName(), "down") == 0) {
        //        Fill(h.h_Beta_Prisma_z14, prismaOutput.beta.Mag());
        //        TLorentzVector              p3;
        //        TLorentzVector              p4;
        //        std::vector<TLorentzVector> gammaDCavg;
        //        std::vector<TLorentzVector> gammaDC;
        //        std::vector<TLorentzVector> gammaDCBP;
        //        double                      TKE{0};

        //        bool                        goodEvent{true};
        //        ReactionFragment::Ion       detectedIon{30, 14};
        //        auto& reactionsIt{reactions.find(detectedIon)};
        //        if((reactions.find(detectedIon))
        //           != reactions.end()) {
        //            auto ion = reactionsIt->second.GetFixedFragment();
        //            ion.Set_Beta(prismaOutput.beta);
        //            p3 = ion.Get_P();
        //            try {
        //                // EvalInMidTarget(p3);
        //                TKE = reactionsIt->second.Set_E_Theta_Phi(
        //                    p3.E(), p3.Theta(), p3.Phi());
        //                p4 = reactionsIt->second.GetFreeFragment().Get_P();

        //                // p3 = SlowInTarget(p3, conf->agataPrisma.dcPos);
        //                // p4 = SlowInTarget(p4, conf->agataPrisma.bpDcPos);
        //            } catch(...) { goodEvent = false; };
        //        }

        //        for(unsigned int i{0}; i < agataOutput.E.size(); i++) {
        //            if(agataOutput.E.at(i) > 1230
        //               && agataOutput.E.at(i) < 1260) {
        //                h.h_TSdiff_z17_561keV->Fill(
        //                     static_cast<double>(prismaOutput.TS)
        //                         - static_cast<double>(agataOutput.TS));
        //            }
        //        }

        //        if(goodEvent && diff > conf.agataPrisma.windowLeft
        //           && diff < conf.agataPrisma.windowRight) {
        //            h.m_Theta_p3p4_z17->Fill( p3.Theta() / UNITS::deg,
        //                 p4.Theta() / UNITS::deg);
        //            h.m_Theta_Beta_p3_z17->Fill( p3.Theta() / UNITS::deg,
        //                 p3.Beta());
        //            h.m_Theta_Beta_p4_z17->Fill( p4.Theta() / UNITS::deg,
        //                 p4.Beta());
        //            h.m_MCPXY_z17->Fill( **prismaAnalysis->prismaReader.MCP_X,
        //                 **prismaAnalysis->prismaReader.MCP_Y);
        //            for(unsigned int i{0}; i < agataOutput.E.size(); i++) {
        //                gammaDC.push_back(agataOutput.P.at(i));
        //                gammaDCBP.push_back(agataOutput.P.at(i));
        //                gammaDCavg.push_back(agataOutput.P.at(i));
        //                gammaDC.back().Boost(-p3.BoostVector());
        //                gammaDCBP.back().Boost(-p4.BoostVector());
        //                gammaDCavg.back().Boost(-betavg);

        //                Fill(h.h_EDC_bavg, gammaDCavg.back().E());

        //                for(int j = 0; j < gammaDCavg.size() - 1; ++j) {
        //                    Fill(h.m_EDC_EDC_bavg, gammaDCavg[i].E(),
        //                         gammaDCavg[j].E());
        //                    Fill(h.m_EDC_EDC_bavg, gammaDCavg[j].E(),
        //                         gammaDCavg[i].E());
        //                }

        //                for(int b{0}; b < h.m_EDCscan->GetXaxis()->GetNbins();
        //                    b++) {
        //                    TLorentzVector p    = agataOutput.P[i];
        //                    TVector3       beta = betavg;
        //                    beta.SetMag(
        //                        h.m_EDCscan->GetXaxis()->GetBinCenter(b));
        //                    p.Boost(-beta);
        //                    Fill(h.m_EDCscan, beta.Mag(), p.E());
        //                }

        //                Fill(h.m_EDC_Theta_Z16_A32,
        //                     p3.Vect().Angle(agataOutput.P.at(i).Vect())
        //                         / UNITS::deg,
        //                     gammaDC.back().E());
        //                Fill(h.m_EDCBP_Theta_Z16_A32,
        //                     p4.Vect().Angle(agataOutput.P.at(i).Vect())
        //                         / UNITS::deg,
        //                     gammaDCBP.back().E());

        //                Fill(h.m_Eg_Theta_Z16_A32,
        //                     p3.Vect().Angle(agataOutput.P.at(i).Vect())
        //                         / UNITS::deg,
        //                     agataOutput.E.at(i));
        //                Fill(h.m_EBP_Theta_Z16_A32,
        //                     p4.Vect().Angle(agataOutput.P.at(i).Vect())
        //                         / UNITS::deg,
        //                     agataOutput.E.at(i));

        //                Fill(h.m_TKE_E_Z17_A35, TKE / UNITS::MeV,
        //                     agataOutput.E.at(i));
        //                Fill(h.m_TKE_EDC_Z17_A35, TKE / UNITS::MeV,
        //                     gammaDC.back().E());
        //                Fill(h.m_TKE_EDCBP_Z17_A35, TKE / UNITS::MeV,
        //                     gammaDCBP.back().E());
        //            }
        //        }
        //    }
        //    // z=14 gate
        //    // //////////////////////////////////////////////////////////////

        //    if(it.cut->IsInside(prismaOutput.E, prismaOutput.dE)
        //       && strcmp(it.cut->GetName(), "up") == 0) {
        //        std::vector<TLorentzVector> gammaDCavg;

        //        for(unsigned int i{0}; i < agataOutput.E.size(); i++) {
        //            gammaDCavg.push_back(agataOutput.P.at(i));
        //            gammaDCavg.back().Boost(-betavg);

        //            Fill(h.h_EDC_bavg_z16, gammaDCavg.back().E());

        //            for(int b{0}; b < h.m_EDCscan_z16->GetXaxis()->GetNbins();
        //                b++) {
        //                TLorentzVector p    = agataOutput.P[i];
        //                TVector3       beta = betavg;
        //                beta.SetMag(
        //                    h.m_EDCscan_z16->GetXaxis()->GetBinCenter(b));
        //                p.Boost(-beta);
        //                Fill(h.m_EDCscan_z16, beta.Mag(), p.E());
        //            }
        //        }
        //    }
        //    return true;
        //}
    }
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
