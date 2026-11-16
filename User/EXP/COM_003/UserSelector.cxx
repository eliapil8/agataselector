#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    if(bla = true) continue;
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

    h.dir->mkdir("z17", "Z=17 histos")->cd();

    h.ptrs.push_back(
        h.h_Beta_Prisma_z17
        = new TH1D("h_Beta_Prisma_z17", "h_Beta_Prisma_z17", 200, 0, 0.2));

    h.ptrs.push_back(h.m_Eg_Theta_Z17_A35
                     = new TH2D("m_Eg_Theta_Z17_A35", "m_Eg_Theta_Z17_A35",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EDC_Theta_Z17_A35
                     = new TH2D("m_EDC_Theta_Z17_A35", "m_EDC_Theta_Z17_A35",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EBP_Theta_Z17_A35
                     = new TH2D("m_EBP_Theta_Z17_A35", "m_EBP_Theta_Z17_A35",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EDCBP_Theta_Z17_A35 = new TH2D(
                         "m_EDCBP_Theta_Z17_A35", "m_EDCBP_Theta_Z17_A35", 360,
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

    h.dir->mkdir("z15", "Z=15 histos")->cd();

    h.ptrs.push_back(h.m_EDC_Theta_Z15_A34
                     = new TH2D("m_EDC_Theta_Z15_A34", "m_EDC_Theta_Z15_A34",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EDCBP_Theta_Z15_A34 = new TH2D(
                         "m_EDCBP_Theta_Z15_A34", "m_EDCBP_Theta_Z15_A34", 360,
                         0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_Eg_Theta_Z15_A34
                     = new TH2D("m_Eg_Theta_Z15_A34", "m_Eg_Theta_Z15_A34",
                                360, 0, 180, 4000, 1, 4001));
    h.ptrs.push_back(h.m_EBP_Theta_Z15_A34
                     = new TH2D("m_EBP_Theta_Z15_A34", "m_EBP_Theta_Z15_A34",
                                360, 0, 180, 4000, 1, 4001));

    h.ptrs.push_back(h.m_Theta_p3p4_z15
                     = new TH2D("m_Theta_p3p4_z15", "m_Theta_p3p4_z15", 360, 0,
                                180, 360, 0, 180));
    h.ptrs.push_back(h.m_Theta_Beta_p3_z15
                     = new TH2D("m_Theta_Beta_p3_z15", "m_Theta_Beta_p3_z15",
                                360, 0, 180, 200, 0, 0.20));
    h.ptrs.push_back(h.m_Theta_Beta_p4_z15
                     = new TH2D("m_Theta_Beta_p4_z15", "m_Theta_Beta_p4_z15",
                                360, 0, 180, 200, 0, 0.20));
    h.ptrs.push_back(h.m_TKE_E_Z15_A34
                     = new TH2D("m_TKE_E_Z15_A34", "m_TKE_E_Z15_A34", 500,
                                -100, 400, 4000, 1, 4001));
    h.ptrs.push_back(h.m_TKE_EDC_Z15_A34
                     = new TH2D("m_TKE_EDC_Z15_A34", "m_TKE_EDC_Z15_A34", 500,
                                -100, 400, 4000, 1, 4001));
    h.ptrs.push_back(h.m_TKE_EDCBP_Z15_A34
                     = new TH2D("m_TKE_EDCBP_Z15_A34", "m_TKE_EDCBP_Z15_A34",
                                500, -100, 400, 4000, 1, 4001));
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process
    UserHistograms& h = userHistograms;

    if(conf->detectorsPresent.Agata && conf->detectorsPresent.Prisma
       && prismaOutput.TS != 0 && agataOutput.TS != 0
       && **prismaReader.MCP_X > -53 && **prismaReader.MCP_X < 38
       && **prismaReader.MCP_Y > -60 && **prismaReader.MCP_Y < 62) {
        unsigned long long udiff = prismaOutput.TS > agataOutput.TS
                                     ? prismaOutput.TS - agataOutput.TS
                                     : agataOutput.TS - prismaOutput.TS;
        long long diff = prismaOutput.TS > agataOutput.TS ? udiff : -udiff;

        for(const auto& it : agataPrismaHistograms.cutHistos) {
            // z=17 gate
            // //////////////////////////////////////////////////////////////

            if(it.cut->IsInside(prismaOutput.E, prismaOutput.dE)
               && strcmp(it.cut->GetName(), "z17") == 0) {
                Fill(h.h_Beta_Prisma_z17, prismaOutput.beta.Mag());
                TLorentzVector              p3;
                TLorentzVector              p4;
                std::vector<TLorentzVector> gammaDC;
                std::vector<TLorentzVector> gammaDCBP;
                double                      TKE{0};

                bool                        goodEvent{true};
                ReactionFragment::Ion       detectedIon{35, 17};
                if((reactionsIt = reactions.find(detectedIon))
                   != reactions.end()) {
                    auto ion = reactionsIt->second.GetFixedFragment();
                    ion.Set_Beta(prismaOutput.beta);
                    p3 = ion.Get_P();
                    try {
                        EvalInMidTarget(p3);
                        TKE = reactionsIt->second.Set_E_Theta_Phi(
                            p3.E(), p3.Theta(), p3.Phi());
                        p4 = reactionsIt->second.GetFreeFragment().Get_P();

                        p3 = SlowInTarget(p3, conf->agataPrisma.dcPos);
                        p4 = SlowInTarget(p4, conf->agataPrisma.bpDcPos);
                    } catch(...) { goodEvent = false; };
                }

                for(unsigned int i{0}; i < agataOutput.E.size(); i++) {
                    if(agataOutput.E.at(i) > 550
                       && agataOutput.E.at(i) < 570) {
                        Fill(h.h_TSdiff_z17_561keV,
                             static_cast<double>(prismaOutput.TS)
                                 - static_cast<double>(agataOutput.TS));
                    }
                }

                if(goodEvent && diff > conf->prisma.windowLeft
                   && diff < conf->prisma.windowRight) {
                    Fill(h.m_Theta_p3p4_z17, p3.Theta() / UNITS::deg,
                         p4.Theta() / UNITS::deg);
                    Fill(h.m_Theta_Beta_p3_z17, p3.Theta() / UNITS::deg,
                         p3.Beta());
                    Fill(h.m_Theta_Beta_p4_z17, p4.Theta() / UNITS::deg,
                         p4.Beta());
                    Fill(h.m_MCPXY_z17, **prismaReader.MCP_X,
                         **prismaReader.MCP_Y);
                    for(unsigned int i{0}; i < agataOutput.E.size(); i++) {
                        gammaDC.push_back(agataOutput.P.at(i));
                        gammaDCBP.push_back(agataOutput.P.at(i));
                        gammaDC.back().Boost(-p3.BoostVector());
                        gammaDCBP.back().Boost(-p4.BoostVector());

                        Fill(h.m_EDC_Theta_Z17_A35,
                             p3.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             gammaDC.back().E());
                        Fill(h.m_EDCBP_Theta_Z17_A35,
                             p4.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             gammaDCBP.back().E());

                        Fill(h.m_Eg_Theta_Z17_A35,
                             p3.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             agataOutput.E.at(i));
                        Fill(h.m_EBP_Theta_Z17_A35,
                             p4.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             agataOutput.E.at(i));

                        Fill(h.m_TKE_E_Z17_A35, TKE / UNITS::MeV,
                             agataOutput.E.at(i));
                        Fill(h.m_TKE_EDC_Z17_A35, TKE / UNITS::MeV,
                             gammaDC.back().E());
                        Fill(h.m_TKE_EDCBP_Z17_A35, TKE / UNITS::MeV,
                             gammaDCBP.back().E());
                    }
                }
            }
            // z=15 gate
            // //////////////////////////////////////////////////////////////

            if(it.cut->IsInside(prismaOutput.E, prismaOutput.dE)
               && strcmp(it.cut->GetName(), "z15") == 0) {
                TLorentzVector              p3;
                TLorentzVector              p4;
                std::vector<TLorentzVector> gammaDC;
                std::vector<TLorentzVector> gammaDCBP;
                double                      TKE{0};

                bool                        goodEvent{true};
                ReactionFragment::Ion       detectedIon{34, 15};
                if((reactionsIt = reactions.find(detectedIon))
                   != reactions.end()) {
                    auto ion = reactionsIt->second.GetFixedFragment();
                    ion.Set_Beta(prismaOutput.beta);
                    p3 = ion.Get_P();
                    // EvalInMidTarget(p3);
                    try {
                        reactionsIt->second.Set_E_Theta_Phi(p3.E(), p3.Theta(),
                                                            p3.Phi());
                        p4 = reactionsIt->second.GetFreeFragment().Get_P();
                    } catch(...) { goodEvent = false; };

                    //                 p3 = SlowInTarget(p3,
                    //                 conf->agataPrisma.dcPos); p4 =
                    //                 SlowInTarget(p4,
                    //                 conf->agataPrisma.bpDcPos);
                }
                if(goodEvent && diff > conf->prisma.windowLeft
                   && diff < conf->prisma.windowRight) {
                    Fill(h.m_Theta_p3p4_z15, p3.Theta() / UNITS::deg,
                         p4.Theta() / UNITS::deg);
                    Fill(h.m_Theta_Beta_p3_z15, p3.Theta() / UNITS::deg,
                         p3.Beta());
                    Fill(h.m_Theta_Beta_p4_z15, p4.Theta() / UNITS::deg,
                         p4.Beta());

                    for(unsigned int i{0}; i < agataOutput.E.size(); i++) {
                        gammaDC.push_back(agataOutput.P.at(i));
                        gammaDCBP.push_back(agataOutput.P.at(i));
                        gammaDC.back().Boost(-p3.BoostVector());
                        gammaDCBP.back().Boost(-p4.BoostVector());

                        Fill(h.m_EDC_Theta_Z15_A34,
                             p3.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             gammaDC.back().E());
                        Fill(h.m_EDCBP_Theta_Z15_A34,
                             p4.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             gammaDCBP.back().E());

                        Fill(h.m_Eg_Theta_Z15_A34,
                             p3.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             agataOutput.E.at(i));
                        Fill(h.m_EBP_Theta_Z15_A34,
                             p4.Vect().Angle(agataOutput.P.at(i).Vect())
                                 / UNITS::deg,
                             agataOutput.E.at(i));

                        Fill(h.m_TKE_E_Z15_A34, TKE / UNITS::MeV,
                             agataOutput.E.at(i));
                        Fill(h.m_TKE_EDC_Z15_A34, TKE / UNITS::MeV,
                             gammaDC.back().E());
                        Fill(h.m_TKE_EDCBP_Z15_A34, TKE / UNITS::MeV,
                             gammaDCBP.back().E());
                    }
                }
            }
        }
    }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
