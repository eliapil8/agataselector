#include "PrismaDanteAnalysis.h"

PrismaDanteAnalysis::PrismaDanteAnalysis(Inputs& in, Prisma& p, Dante& d,
                                         PrismaAnalysis::PrismaReader& rp,
                                         DanteAnalysis::DanteReader&   rd)
    : CoincidenceAnalysis(in, p, ConfManager::getInstance().prisma, d,
                          ConfManager::getInstance().dante,
                          ConfManager::getInstance().prismaDante, rp, rd,
                          prismaDanteHistograms),
      prisma(p), dante(d), prismaReader(rp), danteReader(rd) {
    (void)prismaReader;
    (void)danteReader;
}

bool PrismaDanteAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    PrismaDanteHistograms& h = prismaDanteHistograms;
    h.dir = input.outFile.mkdir("PrismaDante", "Prisma-Dante coincidences");
    h.dir->cd();
    CoincidenceAnalysis::AllocateHistos();

    Initialize<TH2D>(
        h.m_TSdiff_TS,
        new TH2D(
            "m_TSdiff_TS", "m_TSdiff_TS", 12 * 360, 0, 12 * 3600,
            (conf.prismaDante.windowRight - conf.prismaDante.windowLeft + 200),
            conf.prismaDante.windowLeft - 100,
            conf.prismaDante.windowRight + 100),
        h.ptrs);

    Initialize<TH1D>(h.h_cTime_gated,
                     new TH1D("h_cTime_gated", "h_cTime_gated",
                              (conf.prismaDante.windowRight
                               - conf.prismaDante.windowLeft + 200)
                              * 10,
                              conf.prismaDante.windowLeft - 100,
                              conf.prismaDante.windowRight + 100),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_danteId_TSmTS,
        new TH2D(
            "m_danteId_TSmTS",
            "DanteTS - PrismaTS vs danteID; danteID; TS difference [10 ns]",
            64, 0, 64,
            (conf.prismaDante.windowRight - conf.prismaDante.windowLeft + 200)
            * 10,
            conf.prismaDante.windowLeft - 100,
            conf.prismaDante.windowRight + 100),
        h.ptrs);

    // Angle checks
    Initialize<TH2D>(
        h.m_ThetaPrisma_ThetaDante,
        new TH2D("m_ThetaPrisma_ThetaDante",
                 "Comparison of theta reconstructed from prisma and measured "
                 "by dante; Dante Theta [deg]; Prisma Theta [deg]",
                 180 * 4, 0, 180, // range for Energy of Prisma
                 180 * 4, 0, 180),
        h.ptrs); // the range of tof need to be checked

    Initialize<TH2D>(
        h.m_PhiPrisma_PhiDante,
        new TH2D("m_PhiPrisma_PhiDante",
                 "Comparison of phi reconstructed from prisma and measured by "
                 "dante; Dante Phi [deg]; Prisma Phi - 180 [deg]",
                 360 * 4, -180, 180, // range for Energy of Prisma
                 360 * 4, -180, 180),
        h.ptrs); // the range of tof need to be checked

    // TOF histograms
    h.tof.dir = h.dir->mkdir(Form("TOF"),
                             Form("TOT histograms between DANTE and PRISMA"));
    h.tof.dir->cd();
    for(const auto& id : conf.dante.getAllDanteId()) {
        Initialize<TH2D>(
            h.tof.m_TSdiff_Z[id],
            new TH2D(Form("m_TSdiff_Z_%i", id),
                     Form("Prisma-Dante tsdiff vs Z for Dante detector %i;TS "
                          "diff [10 ns];Z [];Counts",
                          id),
                     2000, -500, 500,
                     conf.prisma.Zrange.second - conf.prisma.Zrange.first,
                     conf.prisma.Zrange.first, conf.prisma.Zrange.second),
            h.tof.ptrs);
        Initialize<TH2D>(
            h.tof.m_TSdiff_E[id],
            new TH2D(Form("m_TSdiff_E_%i", id),
                     Form("Prisma-Dante tsdiff vs Ion energy for Dante "
                          "detector %i;TS diff [10 ns];IC Energy [MeV];Counts",
                          id),
                     2000, -500, 500, 1000, 0,
                     conf.prisma.maxICEne), // range for Energy of Prisma
            h.tof.ptrs);
        Initialize<TH2D>(
            h.tof.m_TSdiff_Qval[id],
            new TH2D(Form("m_TSdiff_Qval_%i", id),
                     Form("Prisma-Dante tsdiff vs Q value %i;TS diff [10 "
                          "ns];Total excitation energy [MeV];Counts",
                          id),
                     2000, -500, 500, 1000, -100,
                     100), // range for Energy of Prisma
            h.tof.ptrs);

        if(conf.dante.hasTof(id)) {
            auto brdchanTOF = conf.dante.getBrdChanFromId(id, DanteLut::TOF);
            auto tofThr
                = conf.dante.getThr(brdchanTOF.first, brdchanTOF.second);
            Initialize<TH1D>(h.tof.h_TOF[id],
                             new TH1D(Form("h_TOF_%i", id),
                                      Form("Prisma-Dante TOF"
                                           "%i;TOF [];Counts",
                                           id),
                                      2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id)),
                             h.tof.ptrs);
            Initialize<TH2D>(
                h.tof.m_TOF_Z[id],
                new TH2D(Form("m_TOF_Z_%i", id),
                         Form("Prisma-Dante TOF vs Z for Dante detector "
                              "%i;TOF [];Z [];Counts",
                              id),
                         2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id),
                         conf.prisma.Zrange.second - conf.prisma.Zrange.first,
                         conf.prisma.Zrange.first, conf.prisma.Zrange.second),
                h.tof.ptrs);
            Initialize<TH2D>(
                h.tof.m_TOF_E[id],
                new TH2D(Form("m_TOF_E_%i", id),
                         Form("Prisma-Dante TOF vs Ion energy for Dante "
                              "detector %i;TOF [];IC Energy [MeV];Counts",
                              id),
                         2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id), 1000, 0,
                         conf.prisma.maxICEne), // range for Energy of Prisma
                h.tof.ptrs);
            Initialize<TH2D>(
                h.tof.m_TOF_Qval[id],
                new TH2D(Form("m_TOF_Qval_%i", id),
                         Form("Prisma-Dante TOF vs Q value %i;TOF [];Total "
                              "excitation energy [MeV];Counts",
                              id),
                         2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id), 1000, -100,
                         100), // range for Energy of Prisma
                h.tof.ptrs);
            Initialize<TH2D>(
                h.tof.m_TOF_beta[id],
                new TH2D(Form("m_TOF_beta_%i", id),
                         Form("Prisma-Dante TOF vs beta of DANTE %i;TOF [];"
                              "Beta [];Counts",
                              id),
                         2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id), 1000, 0,
                         0.20), // range for Energy of Prisma
                h.tof.ptrs);
        }
    }
    for(auto& ion : conf.reaction.fragmentsOfInterest) {
        PrismaDanteHistograms::IonHistograms ion_histos;
        ion_histos.dir = h.dir->mkdir(
            Form("Ion_Cuts_Z%i_A%i", ion.first.Z, ion.first.A),
            Form("Gamma spectra with cuts on the identified ion: Z=%d, "
                 "A=%d",
                 ion.first.Z, ion.first.A));
        ion_histos.dir->cd();

        // Angle checks
        Initialize<TH2D>(
            ion_histos.m_ThetaPrisma_ThetaDante,
            new TH2D("m_ThetaPrisma_ThetaDante",
                     "Comparison of theta reconstructed from prisma and "
                     "measured "
                     "by dante; Dante Theta [deg]; Prisma Theta [deg]",
                     180 * 4, 0, 180, // range for Energy of Prisma
                     180 * 4, 0, 180),
            ion_histos.ptrs); // the range of tof need to be checked

        Initialize<TH2D>(
            ion_histos.m_PhiPrisma_PhiDante,
            new TH2D("m_PhiPrisma_PhiDante",
                     "Comparison of phi reconstructed from prisma and "
                     "measured by "
                     "dante; Dante Phi [deg]; Prisma Phi - 180 [deg]",
                     360 * 4, -180, 180, // range for Energy of Prisma
                     360 * 4, -180, 180),
            ion_histos.ptrs); // the range of tof need to be checked

        for(const auto& id : conf.dante.getAllDanteId()) {
            if(conf.dante.hasTof(id)) {
                auto brdchanTOF
                    = conf.dante.getBrdChanFromId(id, DanteLut::TOF);
                auto tofThr
                    = conf.dante.getThr(brdchanTOF.first, brdchanTOF.second);
                Initialize<TH1D>(ion_histos.tof.h_TOF[id],
                                 new TH1D(Form("h_TOF_%i", id),
                                          Form("Prisma-Dante TOF"
                                               "%i;TOF [];Counts",
                                               id),
                                          2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id)),
                                 ion_histos.tof.ptrs);
                Initialize<TH2D>(
                    ion_histos.tof.m_TOF_Z[id],
                    new TH2D(
                        Form("m_TOF_Z_%i", id),
                        Form("Prisma-Dante TOF vs Z for Dante detector "
                             "%i;TOF [];Z [];Counts",
                             id),
                        2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id),
                        conf.prisma.Zrange.second - conf.prisma.Zrange.first,
                        conf.prisma.Zrange.first, conf.prisma.Zrange.second),
                    ion_histos.tof.ptrs);
                Initialize<TH2D>(
                    ion_histos.tof.m_TOF_E[id],
                    new TH2D(
                        Form("m_TOF_E_%i", id),
                        Form("Prisma-Dante TOF vs Ion energy for Dante "
                             "detector %i;TOF [];IC Energy [MeV];Counts",
                             id),
                        2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id), 1000, 0,
                        conf.prisma.maxICEne), // range for Energy of Prisma
                    ion_histos.tof.ptrs);
                Initialize<TH2D>(
                    ion_histos.tof.m_TOF_Qval[id],
                    new TH2D(
                        Form("m_TOF_Qval_%i", id),
                        Form("Prisma-Dante TOF vs Q value %i;TOF [];Total "
                             "excitation energy [MeV];Counts",
                             id),
                        2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id), 1000, -100,
                        100), // range for Energy of Prisma
                    ion_histos.tof.ptrs);
                Initialize<TH2D>(
                    ion_histos.tof.m_TOF_beta[id],
                    new TH2D(
                        Form("m_TOF_beta_%i", id),
                        Form("Prisma-Dante TOF vs beta of DANTE %i;TOF [];"
                             "Beta [];Counts",
                             id),
                        2000, conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id), 1000, 0,
                        0.20), // range for Energy of Prisma
                    ion_histos.tof.ptrs);
            }
        }

        h.ionHistos[ion.first] = ion_histos;
    }
    return true;
}

void PrismaDanteAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();

    if(!prisma.mcp_ok && conf.prisma.require_mcpok) return;
    if(!prisma.tof_ok && conf.prisma.require_tofok) return;
    if(!prisma.side_ok && conf.prisma.require_sideok) return;
    if(!prisma.traj_ok && conf.prisma.require_trajok) return;
    if(!prisma.ic_ok && conf.prisma.require_icok) return;
    PrismaDanteHistograms& h = prismaDanteHistograms;

    for(unsigned int d{0}; d < dante.Pos.size(); ++d) {
        if(!coincMatrix.at(d).at(0)) continue;
        // TS diff histos
        auto dtof
            = unsignedDiff<double, unsigned long long>(dante.TS, prisma.TS)
              + (dante.Time.at(d) - prisma.Time.at(0))
                / (10. * UNITS::nanosecond);
        auto diff
            = unsignedDiff<double, unsigned long long>(dante.TS, prisma.TS)
              + (dante.Time.at(d) - prisma.Time.at(0)) / (10. * UNITS::ns);
        Fill(h.h_cTime_gated, diff);

        // TSdiff histos
        auto danteId = dante.Id.at(d);
        Fill(h.tof.m_TSdiff_Z.at(danteId), dtof, prisma.Z);
        Fill(h.tof.m_TSdiff_E.at(danteId), dtof, prisma.E);
        Fill(h.tof.m_TSdiff_Qval.at(danteId), dtof, prisma.TKEL);

        if(conf.dante.hasTof(danteId)
           || dante.TOF.at(d).second == dante.PRISMA) {
            Fill(h.tof.h_TOF.at(danteId), dante.TOF.at(d).first/UNITS::ns);
            Fill(h.tof.m_TOF_Z.at(danteId), dante.TOF.at(d).first/UNITS::ns, prisma.Z);
            Fill(h.tof.m_TOF_E.at(danteId), dante.TOF.at(d).first/UNITS::ns, prisma.E);
            Fill(h.tof.m_TOF_Qval.at(danteId), dante.TOF.at(d).first/UNITS::ns,
                 prisma.TKEL);
            Fill(h.tof.m_TOF_beta.at(danteId), dante.TOF.at(d).first/UNITS::ns,
                 prisma.beta.Mag());
        }

        Fill(h.m_ThetaPrisma_ThetaDante, dante.Pos.at(d).Theta() / UNITS::deg,
             prisma.beta.Theta()
             / UNITS::deg); // using time information for each hit
        Fill(h.m_PhiPrisma_PhiDante, dante.Pos.at(d).Phi() / UNITS::deg,
             prisma.beta.Phi() / UNITS::deg
             - 180.); // using time information for each hit

        ReactionFragment::Ion detectedIon{prisma.A, prisma.Z};
        auto                  histoIt = h.ionHistos.find(detectedIon);
        if(histoIt != h.ionHistos.end()) {
            Fill(histoIt->second.m_ThetaPrisma_ThetaDante,
                 dante.Pos.at(d).Theta() / UNITS::deg,
                 prisma.beta.Theta()
                 / UNITS::deg); // using time information for each hit
            Fill(histoIt->second.m_PhiPrisma_PhiDante,
                 dante.Pos.at(d).Phi() / UNITS::deg,
                 prisma.beta.Phi() / UNITS::deg
                 - 180.); // using time information for each hit
            if(conf.dante.hasTof(danteId)
               || dante.TOF.at(d).second == dante.PRISMA) {
                Fill(histoIt->second.tof.h_TOF.at(danteId),
                     dante.TOF.at(d).first/UNITS::ns);
                Fill(histoIt->second.tof.m_TOF_Z.at(danteId),
                     dante.TOF.at(d).first/UNITS::ns, prisma.Z);
                Fill(histoIt->second.tof.m_TOF_E.at(danteId),
                     dante.TOF.at(d).first/UNITS::ns, prisma.E);
                Fill(histoIt->second.tof.m_TOF_Qval.at(danteId),
                     dante.TOF.at(d).first/UNITS::ns, prisma.TKEL);
                Fill(histoIt->second.tof.m_TOF_beta.at(danteId),
                     dante.TOF.at(d).first/UNITS::ns, prisma.beta.Mag());
            }
        }
    }
}

void PrismaDanteAnalysis::Finalize() { CoincidenceAnalysis::Finalize(); }
