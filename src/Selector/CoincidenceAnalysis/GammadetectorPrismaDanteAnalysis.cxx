#include "GammadetectorPrismaDanteAnalysis.h"

GammadetectorPrismaDanteAnalysis::GammadetectorPrismaDanteAnalysis(
    DetectorAnalysis::Inputs& in, GammadetectorPrismaConf& cp,
    GammadetectorDanteConf& cd, Gammadetector& g, GammadetectorConf& gc,
    Prisma& p, Dante& d, Readers& rg, PrismaAnalysis::PrismaReader& rp,
    DanteAnalysis::DanteReader& rd, GammaDC& gdc,
    GammadetectorPrismaDanteHistograms& gpdh)
    : GammadetectorPrismaAnalysis(in, cp, g, gc, p, rg, rp, gdc, gpdh),
      dante(d), gammadetectorDanteConf(cd), gammaDC(gdc),
      gammadetectorPrismaDanteHistograms(gpdh), prismaReader(rp),
      danteReader(rd),
        TOF_QVAL_Gate(conf.replay.conf_path+"/CUT/PRISMA_DANTE/TOF_QVAL/"){
        (void)prismaReader;
        (void)danteReader;
    }

bool GammadetectorPrismaDanteAnalysis::AllocateHistos() {
    GammadetectorPrismaDanteHistograms& h = gammadetectorPrismaDanteHistograms;
    h.dir->cd();

    if(!GammadetectorPrismaAnalysis::AllocateHistos()) return false;

    h.dir->cd();

    std::string type;
    switch(gammadetectorConf.spectraType) {
    case GammadetectorConf::SpectraType::SEGMENT: type = "segment"; break;
    case GammadetectorConf::SpectraType::CORE: type = "core"; break;
    case GammadetectorConf::SpectraType::CORE1: type = "core1"; break;
    case GammadetectorConf::SpectraType::ADDBACK: type = "addback"; break;
    case GammadetectorConf::SpectraType::TRACKED: type = "tracked"; break;
    default: break;
    }

    Initialize<TH2D>(
        h.m_TSprismamTSgamma_TSdantemTSgamma,
        new TH2D(
            "m_TSprismamTSgamma_TSdantemTSgamma",
            "TS dante-gamma vs TS prisma-gamma;Time difference dante-gamma "
            "[10 ns];Time difference prisma-gamma [10 ns];Counts/10 ns",
            (gammadetectorDanteConf.windowRight
             - gammadetectorDanteConf.windowLeft + 200)
                * 10,
            gammadetectorDanteConf.windowLeft - 100,
            gammadetectorDanteConf.windowRight + 100,
            (gammadetectorPrismaConf.windowRight
             - gammadetectorPrismaConf.windowLeft + 200)
                * 10,
            gammadetectorPrismaConf.windowLeft - 100,
            gammadetectorPrismaConf.windowRight + 100),
        h.ptrs);

    // Look for TCuts
    //if(settings.verbosity_level > 2)
    //    std::cout << "Looking for cuts in /CUT/PRISMA_DANTE_TOF_TKEL/"
    //              << std::endl;
    //conf.AddCuts<GammadetectorPrismaDanteHistograms::tofTKELHistograms>(
    //    "/CUT/PRISMA_DANTE/TOF_TKEL/", h.tofTKEL);

    //std::vector<GammadetectorPrismaDanteHistograms::tofTKELHistograms*>
    //    histoRefs;
    for(const auto& id : conf.dante.getAllDanteId()) {
        if(conf.dante.hasTof(id) && conf.dante.tofType(id) == Dante::TofType::PRISMA){
            auto cutIds = TOF_QVAL_Gate.getListOfIds(std::array<int, 1>{static_cast<int>(id)});

            for(const auto& cutId : cutIds) {
                for(auto& ion : conf.reaction.fragmentsOfInterest) {
                    auto& ref = h.tofTKELIon[cutId[0]][ion.first];
                         ref.dir = h.dir->mkdir(Form("tof_tkel_cut_id%i_ion_Z%i_A%i", cutId[0], ion.first.Z, ion.first.A),
                                      Form("Gamma spectra with cut in tof vs tkel %i and ion Z=%d, A=%d", cutId[0], ion.first.Z, ion.first.A));
                         ref.dir->cd();

                    Initialize<TH1D>(
                        ref.h_noDC_ion,
                        new TH1D(Form("h_noDC_ion"),
                                 Form("Non-DC %s spectrum;Energy [keV];Counts/%d keV",
                                      type.c_str(), gammadetectorConf.binWidth),
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH1D>(
                        ref.h_DC_ion,
                        new TH1D(Form("h_DC_ion"),
                                 Form("DC %s spectrum;Energy [keV];Counts/%d keV",
                                      type.c_str(), gammadetectorConf.binWidth),
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH1D>(
                        ref.h_DCBP_ion,
                        new TH1D(Form("h_DCBP_ion"),
                                 Form("DC %s spectrum for binary partner;Energy "
                                      "[keV];Counts/%d keV",
                                      type.c_str(), gammadetectorConf.binWidth),
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);

                    Initialize<TH2D>(
                        ref.m_noDC_noDC_ion,
                        new TH2D(Form("m_noDC_noDC_ion"),
                                 Form("gamma-gamma non-DC %s spectrum;Energy [keV];Energy "
                                      "[keV];",
                                      type.c_str()),
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV,
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH2D>(
                        ref.m_DC_DC_ion,
                        new TH2D(
                            Form("m_DC_DC_ion"),
                            Form("gamma-gamma DC %s spectrum;Energy [keV];Energy [keV];",
                                 type.c_str()),
                            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                             / gammadetectorConf.binWidth),
                            0, gammadetectorConf.gammaEmax / UNITS::keV,
                            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                             / gammadetectorConf.binWidth),
                            0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH2D>(
                        ref.m_DCBP_DCBP_ion,
                        new TH2D(Form("m_DCBP_DCBP_ion"),
                                 Form("gamma-gamma DC %s spectrum for binary "
                                      "partner;Energy [keV];Energy [keV];",
                                      type.c_str()),
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV,
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);

                    Initialize<TH2D>(
                        ref.m_noDC_theta_ion,
                        new TH2D(Form("m_noDC_theta_ion"),
                                 Form("Non-DC gammma energy vs theta  %s spectrum;Theta "
                                      "[deg];Energy [keV];",
                                      type.c_str()),
                                 180, 0, 180,
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH2D>(
                        ref.m_noDC_thetaBP_ion,
                        new TH2D(Form("m_noDC_thetaBP_ion"),
                                 Form("Non-DC gammma energy vs theta of binary partner %s "
                                      "spectrum;Theta BP [deg];Energy [keV];",
                                      type.c_str()),
                                 180, 0, 180,
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH2D>(
                        ref.m_DC_theta_ion,
                        new TH2D(Form("m_DC_theta_ion"),
                                 Form("DC gammma energy vs theta %s spectrum;Theta "
                                      "[deg];Energy [keV];",
                                      type.c_str()),
                                 180, 0, 180,
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH2D>(
                        ref.m_DCBP_theta_ion,
                        new TH2D(Form("m_DCBP_theta_ion"),
                                 Form("DC gammma energy vs theta %s spectrum for binary "
                                      "partner;Theta [deg];Energy [keV];",
                                      type.c_str()),
                                 180, 0, 180,
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);

                    Initialize<TH2D>(
                        ref.m_noDC_Qval_ion,
                        new TH2D(
                            Form("m_noDC_Qval_ion"),
                            Form("Non-DC gammma energy vs excitation energy (-Q value) %s "
                                 "spectrum;-Q value [MeV];#gamma energy [keV];",
                                 type.c_str()),
                            300, -50, 100,
                            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                             / gammadetectorConf.binWidth),
                            0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH2D>(
                        ref.m_DC_Qval_ion,
                        new TH2D(Form("m_DC_Qval_ion"),
                                 Form("DC gammma energy vs excitation energy (-Q value) "
                                      "%s spectrum;-Q value [MeV];#gamma energy [keV];",
                                      type.c_str()),
                                 300, -50, 100,
                                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                                  / gammadetectorConf.binWidth),
                                 0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);
                    Initialize<TH2D>(
                        ref.m_DCBP_Qval_ion,
                        new TH2D(
                            Form("m_DCBP_Qval_ion"),
                            Form("DC gammma energy vs excitation energy (-Q value) "
                                 "%s spectrum for binary partner;Total Excitation Energy "
                                 "[MeV];#gamma energy [keV];",
                                 type.c_str()),
                            300, conf.prisma.minExEne, conf.prisma.maxExEne,
                            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                             / gammadetectorConf.binWidth),
                            0, gammadetectorConf.gammaEmax / UNITS::keV),
                        ref.ptrs);

                }
            }
        }
    }
    return true;
}

void GammadetectorPrismaDanteAnalysis::Analyze() {
    GammadetectorPrismaAnalysis::Analyze();
    gammaDC.clear();
    //if(!in_coinc) return;

    if(!prisma.mcp_ok && conf.prisma.require_mcpok) return;
    if(!prisma.tof_ok && conf.prisma.require_tofok) return;
    if(!prisma.side_ok && conf.prisma.require_sideok) return;
    if(!prisma.traj_ok && conf.prisma.require_trajok) return;
    if(!prisma.ic_ok && conf.prisma.require_icok) return;

    if(coincMatrix.size()==1)  gammaDC.resize(coincMatrix.at(0).size());
    else return;

    int danteIdxMatch{-1};
    for(unsigned int d{0}; d < dante.Time.size(); d++) {
        //Prisma-Dante Timing condition
        auto diff = unsignedDiff<double, unsigned long long>(prisma.TS,
                                                             dante.TS)
                  + (prisma.Time.at(0) - dante.Time.at(d)) / (10. * UNITS::ns);
        if(!(diff > conf.prismaDante.windowLeft && diff < conf.prismaDante.windowRight))
            continue;

        //Angle ob binary reaction condition
        if(abs(dante.Pos.at(d).Theta() - prisma.p4.Theta())
               > conf.prismaDante.max_theta_discr
           || abs(dante.Pos.at(d).Phi() - prisma.p4.Phi())
                  > conf.prismaDante.max_phi_discr) {
            continue;
        }
	
	danteIdxMatch = d;
        dante.p4.at(d) = prisma.p4;
        dante.p4.at(d).SetTheta(dante.Pos.at(d).Theta());
        dante.p4.at(d).SetPhi(dante.Pos.at(d).Phi());
    }
    
    for(unsigned int a{0}; a < gammadetector.E.size(); a++) {
        if(!(gammaDC.inCoinc.at(a) = (danteIdxMatch >= 0))) continue;
        if(coincMatrix.at(0).at(a)){
            gammaDC.inCoinc.at(a) = true;
        } else {
            gammaDC.inCoinc.at(a) = false;
        }

        gammaDC.E.at(a)     = gammadetector.P.at(a);
        gammaDC.EDC.at(a)   = gammadetector.P.at(a);
        gammaDC.EDCBP.at(a) = gammadetector.P.at(a);

        ReactionFragment::Ion ion{prisma.A, prisma.Z};
        ReactionFragment::Ion ionBP
            = conf.reaction.target + conf.reaction.beam - ion;

        gammaDC.p3
            = SlowInTarget(prisma.p3, gammadetectorPrismaConf.dcPos, ion);
        gammaDC.p4 = SlowInTarget(dante.p4.at(danteIdxMatch),
                                  gammadetectorPrismaConf.bpDcPos, ionBP);

        gammaDC.EDC.at(a).Boost(-gammaDC.p3.BoostVector());
        gammaDC.EDCBP.at(a).Boost(-gammaDC.p4.BoostVector());

    }
}

bool GammadetectorPrismaDanteAnalysis::FillHistos() {
    if(!GammadetectorPrismaAnalysis::FillHistos()) return false;
    //if(!in_coinc) return true;

    GammadetectorPrismaDanteHistograms& h = gammadetectorPrismaDanteHistograms;

    // Get the histograms to fill
    // TCutG gated histograms
    for(size_t d{0}; d < dante.Time.size(); ++d) {
        if(dante.TOF.at(d).second != Dante::TofType::PRISMA) continue;
        for(auto &gateId: h.tofTKELIon){
            auto gated = TOF_QVAL_Gate.Id({static_cast<int>(dante.Id.at(d))},
                                          dante.TOF.at(d).first/UNITS::ns,
                                          prisma.TKEL);
            if(!gated.first || gated.second.at(0) != gateId.first) continue;

            ReactionFragment::Ion detectedIon{prisma.A, prisma.Z};
            auto histoIt = gateId.second.find(detectedIon);
            if(histoIt == gateId.second.end()) continue;

            for(size_t a{0}; a < gammaDC.EDC.size(); ++a) {
                if(!gammaDC.inCoinc.at(a)) continue;
                Fill(histoIt->second.h_noDC_ion, gammaDC.E.at(a).E());
                Fill(histoIt->second.h_DC_ion, gammaDC.EDC.at(a).E());
                Fill(histoIt->second.h_DCBP_ion, gammaDC.EDCBP.at(a).E());

                Fill(histoIt->second.m_noDC_theta_ion,
                     prisma.p3.Angle(gammaDC.E.at(a).Vect())
                     / UNITS::deg,
                     gammadetector.E.at(a));
                Fill(histoIt->second.m_noDC_thetaBP_ion,
                     dante.p4.at(d).Angle(gammaDC.EDCBP.at(a).Vect())
                     / UNITS::deg,
                     gammadetector.E.at(a));
                Fill(histoIt->second.m_DC_theta_ion,
                     prisma.p3.Angle(gammaDC.EDC.at(a).Vect())
                     / UNITS::deg,
                     gammaDC.EDC.at(a).E());
                Fill(histoIt->second.m_DCBP_theta_ion,
                     dante.p4.at(d).Angle(gammaDC.EDCBP.at(a).Vect())
                     / UNITS::deg,
                     gammaDC.EDCBP.at(a).E());

                // E gamma vs excitation energy
                Fill(histoIt->second.m_noDC_Qval_ion, prisma.TKEL, gammaDC.E.at(a).E());
                Fill(histoIt->second.m_DC_Qval_ion, prisma.TKEL, gammaDC.EDC.at(a).E());
                Fill(histoIt->second.m_DCBP_Qval_ion, prisma.TKEL,
                     gammaDC.EDCBP.at(a).E());

                for(size_t a2{0}; a2 < a; ++a2) {
                    if(!gammaDC.inCoinc.at(a2)) continue;
                    Fill(histoIt->second.m_noDC_noDC_ion, gammaDC.E.at(a).E(),
                         gammadetector.E.at(a2));
                    Fill(histoIt->second.m_noDC_noDC_ion, gammaDC.E.at(a2).E(),
                         gammadetector.E.at(a));

                    Fill(histoIt->second.m_DC_DC_ion, gammaDC.EDC.at(a).E(),
                         gammaDC.EDC.at(a2).E());
                    Fill(histoIt->second.m_DC_DC_ion, gammaDC.EDC.at(a2).E(),
                         gammaDC.EDC.at(a).E());

                    Fill(histoIt->second.m_DCBP_DCBP_ion, gammaDC.EDCBP.at(a).E(),
                         gammaDC.EDCBP.at(a2).E());
                    Fill(histoIt->second.m_DCBP_DCBP_ion, gammaDC.EDCBP.at(a2).E(),
                         gammaDC.EDCBP.at(a).E());
                }
            }
        }
    }
    return true;
}

void GammadetectorPrismaDanteAnalysis::Finalize() {
    GammadetectorPrismaAnalysis::Finalize();
}
void GammadetectorPrismaDanteAnalysis::Clear() { gammaDC.clear(); }
