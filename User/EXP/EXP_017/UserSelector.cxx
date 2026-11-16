#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

   // std::string type;
   // switch(conf.agata.spectraType) {
   // case GammadetectorConf::SpectraType::SEGMENT: type = "segment"; break;
   // case GammadetectorConf::SpectraType::CORE: type = "core"; break;
   // case GammadetectorConf::SpectraType::CORE1: type = "core1"; break;
   // case GammadetectorConf::SpectraType::ADDBACK: type = "addback"; break;
   // case GammadetectorConf::SpectraType::TRACKED: type = "tracked"; break;
   // default: break;
   // }

   // std::string sRun = fChain->GetDirectory()->GetName();
   // sRun.erase(sRun.begin(), sRun.begin() + sRun.find("run_") + 4);
   // sRun.erase(sRun.begin() + 4, sRun.end());

   // runNr  = std::stoi(sRun);

   // initTS = AgataSelector::GetFirstTS();
   // oldTS  = initTS;
   // outFile.mkdir("User", "User's custom analysis");
   // outFile.cd("User");

   // UserHistograms& hu = userHistograms;

   // hu.h_alive_time,
   //     new TH1D("h_alive_time",
   //              "Duration of beam on target"
   //              ";Run;Alive Time [s]",
   //              100, 0, 100),

   //     hu.h_avgCR,
   //     new TH1D("h_avgCR",
   //              "Average CR of agata"
   //              ";Run;AVG CR [s-1]",
   //              100, 0, 100),

   //     gDirectory->mkdir("z52");
   // gDirectory->cd("z52");

   // hu.h_cTime = new TH1D(
   //     "h_cTime_z52", "h_cTime_z52",
   //     (conf.agataPrisma.windowRight - conf.agataPrisma.windowLeft + 200)
   //         * 10,
   //     conf.agataPrisma.windowLeft - 100, conf.agataPrisma.windowRight + 100);

   // hu.h_noDC_z52 = new TH1D(Form("h_noDC_z52"),
   //                          Form("Non-DC %s spectrum for  ;Energy "
   //                               "[keV];Counts/%d keV",
   //                               type.c_str(), conf.agata.binWidth),
   //                          static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                           / conf.agata.binWidth),
   //                          0, conf.agata.gammaEmax / UNITS::keV);

   // hu.h_DC_z52   = new TH1D(Form("h_DC_z52"),
   //                          Form("DC %s spectrum for  ;Energy "
   //                                 "[keV];Counts/%d keV",
   //                               type.c_str(), conf.agata.binWidth),
   //                          static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                         / conf.agata.binWidth),
   //                          0, conf.agata.gammaEmax / UNITS::keV);

   // hu.h_DCBP_z52 = new TH1D(Form("h_DCBP_z52"),
   //                          Form("DC %s spectrum for binary partner for  "
   //                               ";Energy [keV];Counts/%d keV",
   //                               type.c_str(), conf.agata.binWidth),
   //                          static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                           / conf.agata.binWidth),
   //                          0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_noDC_noDC_z52
   //     = new TH2D(Form("m_noDC_noDC_z52"),
   //                Form("gamma-gamma non-DC %s spectrum for  "
   //                     ";Energy [keV];Energy [keV];",
   //                     type.c_str()),
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_DC_DC_z52
   //     = new TH2D(Form("m_DC_DC_z52"),
   //                Form("gamma-gamma DC %s spectrum for  "
   //                     ";Energy [keV];Energy [keV];",
   //                     type.c_str()),
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_DCBP_DCBP_z52
   //     = new TH2D(Form("m_DCBP_DCBP_z52"),
   //                Form("gamma-gamma DC %s spectrum for binary partner for "
   //                     " ;Energy [keV];Energy [keV];",
   //                     type.c_str()),
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_Theta_ThetaBP_z52 = new TH2D(Form("m_Theta_ThetaBP_z52"),
   //                                   Form("Theta vs Theta BP spectrum for "
   //                                        " ;Theta [deg];ThetaBP [deg];"),
   //                                   180, 0., 180., 180, 0., 180.);

   // hu.m_Beta_BetaBP_z52   = new TH2D(Form("m_Beta_BetaBP_z52"),
   //                                   Form("Beta vs Beta BP spectrum for "
   //                                          " ;Beta [];BetaBP [];"),
   //                                   200, 0., 0.2, 200, 0., 0.2);

   // hu.m_noDC_theta_z52
   //     = new TH2D(Form("m_noDC_theta_z52"),
   //                Form("Non-DC gammma energy vs theta  %s spectrum for "
   //                     " ;Theta [deg];Energy [keV];",
   //                     type.c_str()),
   //                180, 0, 180,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_noDC_thetaBP_z52
   //     = new TH2D(Form("m_noDC_thetaBP_z52"),
   //                Form("Non-DC gammma energy vs theta of binary partner %s "
   //                     "spectrum for  ;Theta BP [deg];Energy [keV];",
   //                     type.c_str()),
   //                180, 0, 180,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_DC_theta_z52
   //     = new TH2D(Form("m_DC_theta_z52"),
   //                Form("DC gammma energy vs theta %s spectrum for  "
   //                     ";Theta [deg];Energy [keV];",
   //                     type.c_str()),
   //                180, 0, 180,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_DCBP_theta_z52
   //     = new TH2D(Form("m_DCBP_theta_z52"),
   //                Form("DC gammma energy vs theta %s spectrum for binary "
   //                     "partner for  ;Theta [deg];Energy [keV];",
   //                     type.c_str()),
   //                180, 0, 180,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_noDC_Qval_z52
   //     = new TH2D(Form("m_noDC_Qval_z52"),
   //                Form("Non-DC gammma energy vs excitation energy (-Q "
   //                     "value) %s spectrum for  ;Total "
   //                     "Excitation Energy"
   //                     "[MeV];#gamma energy [keV];",
   //                     type.c_str()),
   //                600, conf.prisma.minExEne, conf.prisma.maxExEne,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_DC_Qval_z52
   //     = new TH2D(Form("m_DC_Qval_z52"),
   //                Form("DC gammma energy vs excitation energy (-Q value) "
   //                     "%s spectrum for  ;Total Excitation Energy "
   //                     "[MeV];#gamma energy [keV];",
   //                     type.c_str()),
   //                600, conf.prisma.minExEne, conf.prisma.maxExEne,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_DCBP_Qval_z52
   //     = new TH2D(Form("m_DCBP_Qval_z52"),
   //                Form("DC gammma energy vs excitation energy (-Q value) "
   //                     "%s spectrum for binary partner for  "
   //                     ";Total Excitation Energy [MeV];#gamma energy [keV];",
   //                     type.c_str()),
   //                600, conf.prisma.minExEne, conf.prisma.maxExEne,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_dBeta_EDC_z52
   //     = new TH2D(Form("m_dBeta_EDC_z52"),
   //                Form("Delta beta vs DC gammma energy spectrum  "
   //                     ";dBeta [#{percent}];Energy [keV];"),
   //                200, -0.02, 0.02,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);

   // hu.m_dTOF_EDC_z52
   //     = new TH2D(Form("m_dTOF_EDC_z52"),
   //                Form("Delta TOF vs DC gammma energy spectrum  "
   //                     ";TOF [ns];Energy [keV];"),
   //                200, -10, 10,
   //                static_cast<int>(conf.agata.gammaEmax / UNITS::keV
   //                                 / conf.agata.binWidth),
   //                0, conf.agata.gammaEmax / UNITS::keV);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry
    UserHistograms& hu = userHistograms;

    //if(agataOutput.TS > 0) {
    //    ++counter;
    //    if((agataOutput.TS - oldTS) / 1.e8 > 1.) {
    //        if(counter > 170) {
    //            totCR += counter;
    //            ++aliveT;
    //        }
    //        counter = 0;
    //        oldTS   = agataOutput.TS;
    //    }
    //}

    //if(prismaOutput.Z == 52 && prismaOutput.A > 0) {
    //    std::vector<TLorentzVector> gammaDC;
    //    std::vector<TLorentzVector> gammaDCBP;
    //    for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
    //        if(!prismaOutput.mcp_ok && conf.prisma.require_mcpok) { continue; }
    //        if(!prismaOutput.tof_ok && conf.prisma.require_tofok) { continue; }
    //        if(!prismaOutput.side_ok && conf.prisma.require_sideok) {
    //            continue;
    //        }
    //        if(!prismaOutput.traj_ok && conf.prisma.require_trajok) {
    //            continue;
    //        }
    //        if(!prismaOutput.ic_ok && conf.prisma.require_icok) { continue; }

    //        auto diff = unsignedDiff<double, unsigned long long>(
    //                        prismaOutput.TS, agataOutput.TS)
    //                  + (prismaOutput.Time.at(0) - agataOutput.Time.at(i))
    //                        / (10. * UNITS::ns);
    //        hu.h_cTime->Fill(diff);

    //        TLorentzVector p3;
    //        TLorentzVector p4;
    //        TVector3       beta3;
    //        TVector3       beta4;
    //        gammaDC.push_back(agataOutput.P.at(i));
    //        gammaDCBP.push_back(agataOutput.P.at(i));
    //        if(prismaOutput.p3.Mag() != 0) {
    //            // Fragment is identified
    //            ReactionFragment::Ion ion{prismaOutput.A, prismaOutput.Z};
    //            ReactionFragment::Ion ionBP;
    //            ionBP.A
    //                = conf.reaction.target.A + conf.reaction.beam.A - ion.A;
    //            ionBP.Z
    //                = conf.reaction.target.Z + conf.reaction.beam.Z - ion.Z;

    //            // Reaction reconstruction part
    //            if(targetELoss.find(ion) == degraderELoss.end()) {
    //                auto targetELossIt = targetELoss.find(conf.reaction.beam);

    //                double midTargetBeamEnergy = conf.reaction.energy;
    //                midTargetBeamEnergy
    //                    = targetELossIt->second.SlowWithDensity(
    //                        midTargetBeamEnergy, conf.target.thickness / 2.,
    //                        TVector3(0, 0, 1).Angle(conf.target.targetNorm));

    //                reactions.emplace(
    //                    std::piecewise_construct, std::forward_as_tuple(ion),
    //                    std::forward_as_tuple(
    //                        ReactionReconstruction2body<
    //                            long double>::ReactionInput2body{
    //                            ReactionFragment::FragmentSettings(
    //                                conf.reaction.beam.A, conf.reaction.beam.Z,
    //                                0, midTargetBeamEnergy, 0 * UNITS::MeV),
    //                            ReactionFragment::FragmentSettings(
    //                                conf.reaction.target.A,
    //                                conf.reaction.target.Z, 0, 0 * UNITS::MeV,
    //                                0 * UNITS::MeV),
    //                            ReactionFragment::FragmentSettings(
    //                                ionBP.A, ionBP.Z, 0, 0 * UNITS::MeV,
    //                                0 * UNITS::MeV),
    //                            ReactionFragment::FragmentSettings(
    //                                ion.A, ion.Z, 0, 0, 0 * UNITS::MeV)},
    //                        conf.replay.conf_path + "/NuclearData/nutab12"));
    //                reactions.at(ion).ChooseFixed(4);
    //                reactions.at(ion).ChooseExFixed(4);

    //                // Energy loss tables
    //                std::string sTemp = conf.replay.conf_path
    //                                  + "/EnergyLoss/Target-"
    //                                  + conf.target.material + "_Ion-"
    //                                  + std::to_string(ion.A) + "-"
    //                                  + std::to_string(ion.Z) + ".txt";

    //                if(FileIsPresent(sTemp)) {
    //                    targetELoss.emplace(ion,
    //                                        EnergyLoss(sTemp, "SRIM", 100));
    //                }

    //                sTemp = conf.replay.conf_path + "/EnergyLoss/Target-"
    //                      + conf.target.material + "_Ion-"
    //                      + std::to_string(ionBP.A) + "-"
    //                      + std::to_string(ionBP.Z) + ".txt";

    //                if(FileIsPresent(sTemp)) {
    //                    targetELoss.emplace(ionBP,
    //                                        EnergyLoss(sTemp, "SRIM", 100));
    //                }
    //            }

    //            // p3    = SlowInTarget(prismaOutput.p3, conf.agataPrisma.dcPos,
    //            //                      ion);
    //            // p4    = SlowInTarget(prismaOutput.p4,
    //            // conf.agataPrisma.bpDcPos,
    //            //                      ionBP);

    //            beta3 = p3.BoostVector();
    //            beta4 = p4.BoostVector();
    //            gammaDC.back().Boost(-p3.BoostVector());
    //            gammaDCBP.back().Boost(-p4.BoostVector());
    //        } else {
    //            if(prismaOutput.beta.Mag() > 0) {
    //                // Fragment has beta and either is not identified or no
    //                // energy losses are calculated
    //                beta3 = prismaOutput.beta;
    //                gammaDC.back().Boost(-prismaOutput.beta);
    //            } else {
    //                // Fragment has no beta, take the average beta
    //                beta3 = conf.prisma.betaAvg;
    //                gammaDC.back().Boost(-conf.prisma.betaAvg);
    //            }
    //        }

    //        ReactionFragment::Ion detectedIon{prismaOutput.A, prismaOutput.Z};

    //        hu.h_noDC_z52->Fill(agataOutput.E.at(i));
    //        hu.h_DC_z52->Fill(gammaDC.back().E());
    //        hu.h_DCBP_z52->Fill(gammaDCBP.back().E());

    //        // Beta scan
    //        for(int nBeta = -100; nBeta < 100; ++nBeta) {
    //            TLorentzVector ptemp    = agataOutput.P.at(i);
    //            TVector3       betatemp = prismaOutput.beta;
    //            betatemp.SetMag(prismaOutput.beta.Mag() + nBeta * 0.0001);
    //            ptemp.Boost(-betatemp);
    //            hu.m_dBeta_EDC_z52->Fill((nBeta + 0.5) * 0.0002, ptemp.E());

    //            double tof = **prismaAnalysis->prismaReader.TOF * UNITS::ns
    //                           * conf.prisma.tofGain
    //                       + conf.prisma.tofOffset
    //                       + (double(nBeta) / 10.) * UNITS::ns;
    //            double beta
    //                = (**prismaAnalysis->prismaReader.Length * UNITS::mm) / tof
    //                / UNITS::PHYSICS::c_light;
    //            betatemp.SetMag(beta * conf.prisma.betaGain
    //                            + conf.prisma.betaOffset);
    //            ptemp = agataOutput.P.at(i);
    //            ptemp.Boost(-betatemp);
    //            hu.m_dTOF_EDC_z52->Fill((nBeta + 0.5) / 10., ptemp.E());
    //        }

    //        hu.m_Beta_BetaBP_z52->Fill(beta3.Mag(), beta4.Mag());
    //        hu.m_Theta_ThetaBP_z52->Fill(beta3.Theta() / UNITS::deg,
    //                                     beta4.Theta() / UNITS::deg);

    //        // E gamma vs theta
    //        hu.m_noDC_theta_z52->Fill(
    //            prismaOutput.beta.Angle(agataOutput.P.at(i).Vect())
    //                / UNITS::deg,
    //            agataOutput.E.at(i));
    //        hu.m_noDC_thetaBP_z52->Fill(p4.Angle(agataOutput.P.at(i).Vect())
    //                                        / UNITS::deg,
    //                                    agataOutput.E.at(i));
    //        hu.m_DC_theta_z52->Fill(
    //            prismaOutput.beta.Angle(agataOutput.P.at(i).Vect())
    //                / UNITS::deg,
    //            gammaDC.back().E());
    //        hu.m_DCBP_theta_z52->Fill(p4.Angle(agataOutput.P.at(i).Vect())
    //                                      / UNITS::deg,
    //                                  gammaDCBP.back().E());

    //        // E gamma vs excitation energy
    //        hu.m_noDC_Qval_z52->Fill(prismaOutput.TKEL, agataOutput.E.at(i));
    //        hu.m_DC_Qval_z52->Fill(prismaOutput.TKEL, gammaDC.back().E());
    //        hu.m_DCBP_Qval_z52->Fill(prismaOutput.TKEL, gammaDCBP.back().E());

    //        // gamma-gamma no DC filling loop
    //        for(unsigned int j = 0; j < i; j++) {
    //            hu.m_noDC_noDC_z52->Fill(agataOutput.E.at(i),
    //                                     agataOutput.E.at(j));
    //            hu.m_noDC_noDC_z52->Fill(agataOutput.E.at(j),
    //                                     agataOutput.E.at(i));
    //        }

    //        // gamma-gamma with DC filling loop
    //        for(unsigned int j = 0; j < gammaDC.size() - 1; j++) {
    //            hu.m_DC_DC_z52->Fill(gammaDC.back().E(), gammaDC.at(j).E());
    //            hu.m_DC_DC_z52->Fill(gammaDC.at(j).E(), gammaDC.back().E());

    //            hu.m_DCBP_DCBP_z52->Fill(gammaDCBP.back().E(),
    //                                     gammaDCBP.at(j).E());
    //            hu.m_DCBP_DCBP_z52->Fill(gammaDCBP.at(j).E(),
    //                                     gammaDCBP.back().E());
    //        }
    //    }
    //}
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
    //UserHistograms& hu = userHistograms;

    //hu.h_alive_time->Fill(runNr, aliveT);
    //if(aliveT > 0) hu.h_avgCR->Fill(runNr, totCR / aliveT);
}
