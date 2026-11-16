#include "GammadetectorPrismaAnalysis.h"

GammadetectorPrismaAnalysis::GammadetectorPrismaAnalysis(
    Inputs& in, GammadetectorPrismaConf& gpConf, Gammadetector& g,
    GammadetectorConf& gConf, Prisma& p, Readers& rg,
    PrismaAnalysis::PrismaReader& rp, GammaDC& gdc,
    GammadetectorPrismaHistograms& gph)
    : CoincidenceAnalysis(in, g, gConf, p, ConfManager::getInstance().prisma,
                          gpConf, rg, rp, gph),
      gammadetector(g), prisma(p), gammadetectorPrismaConf(gpConf),
      gammadetectorConf(gConf), gammaDC(gdc),
      gammadetectorPrismaHistograms(gph), prismaReader(rp) {}

bool GammadetectorPrismaAnalysis::AllocateHistos() {
    if(!CoincidenceAnalysis::AllocateHistos()) return false;

    std::string type;
    switch(gammadetectorConf.spectraType) {
    case GammadetectorConf::SpectraType::SEGMENT: type = "segment"; break;
    case GammadetectorConf::SpectraType::CORE: type = "core"; break;
    case GammadetectorConf::SpectraType::CORE1: type = "core1"; break;
    case GammadetectorConf::SpectraType::ADDBACK: type = "addback"; break;
    case GammadetectorConf::SpectraType::TRACKED: type = "tracked"; break;
    default: break;
    }

    GammadetectorPrismaHistograms& h = gammadetectorPrismaHistograms;
    h.dir->cd();

    Initialize<TH2D>(h.m_TSdiff_TS,
                     new TH2D("m_TSdiff_TS", "m_TSdiff_TS", 12 * 360, 0,
                              12 * 3600,
                              (gammadetectorPrismaConf.windowRight
                               - gammadetectorPrismaConf.windowLeft + 200),
                              gammadetectorPrismaConf.windowLeft - 100,
                              gammadetectorPrismaConf.windowRight + 100),
                     h.ptrs);

    Initialize<TH1D>(h.h_cTime,
                     new TH1D("h_cTime", "h_cTime",
                              (gammadetectorPrismaConf.windowRight
                               - gammadetectorPrismaConf.windowLeft + 200)
                                  * 10,
                              gammadetectorPrismaConf.windowLeft - 100,
                              gammadetectorPrismaConf.windowRight + 100),
                     h.ptrs);
    Initialize<TH1D>(h.h_cTime_gated,
                     new TH1D("h_cTime_gated", "h_cTime_gated",
                              (gammadetectorPrismaConf.windowRight
                               - gammadetectorPrismaConf.windowLeft + 200)
                                  * 10,
                              gammadetectorPrismaConf.windowLeft - 100,
                              gammadetectorPrismaConf.windowRight + 100),
                     h.ptrs);

    Initialize<TH2D>(
            h.m_cTime_coreId,
            new TH2D("m_cTime_coreId", "m_cTime_coreId",
                    (gammadetectorPrismaConf.windowRight
                    - gammadetectorPrismaConf.windowLeft + 200),
                    gammadetectorPrismaConf.windowLeft - 100,
                    gammadetectorPrismaConf.windowRight + 100,
                    45,-0.5,44.5),
            h.ptrs);

    Initialize<TH2D>(
        h.m_cTime_E,
        new TH2D("m_cTime_E", "m_cTime_E",
                 (gammadetectorPrismaConf.windowRight
                  - gammadetectorPrismaConf.windowLeft + 200),
                 gammadetectorPrismaConf.windowLeft - 100,
                 gammadetectorPrismaConf.windowRight + 100,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_cTime_EDC,
        new TH2D("m_cTime_EDC", "m_cTime_EDC",
                 (gammadetectorPrismaConf.windowRight
                  - gammadetectorPrismaConf.windowLeft + 200),
                 gammadetectorPrismaConf.windowLeft - 100,
                 gammadetectorPrismaConf.windowRight + 100,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_cTime_EDCBP,
        new TH2D("m_cTime_EDCBP", "m_cTime_EDCBP",
                 (gammadetectorPrismaConf.windowRight
                  - gammadetectorPrismaConf.windowLeft + 200),
                 gammadetectorPrismaConf.windowLeft - 100,
                 gammadetectorPrismaConf.windowRight + 100,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(h.m_ICDEAB_ICE,
                     new TH2D("m_ICDEAB_ICE",
                              "total DE_AB : E, with Prisma-Gamma time gate",
                              2000, 0, conf.prisma.maxICEne, 2000, 0,
                              conf.prisma.maxICdEne),
                     h.ptrs);

    Initialize<TH1D>(
        h.h_E,
        new TH1D("h_E",
                 Form("Total E %s spectrum;Energy [keV];Counts/%d keV",
                      type.c_str(), gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_DC,
        new TH1D("h_DC",
                 Form("Total DC %s spectrum;Energy [keV];Counts/%d keV",
                      type.c_str(), gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_DCBP,
        new TH1D("h_DCBP",
                 Form("Total DC %s spectrum for binary partner;Energy "
                      "[keV];Counts/%d keV",
                      type.c_str(), gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_E,
        new TH2D("m_E_E",
                 Form("Total %s gamma-gamma;Energy [keV];Energy [keV];",
                      type.c_str()),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_DC_DC,
        new TH2D("m_DC_DC",
                 Form("Total %s gamma-gamma DC;Energy [keV];Energy [keV];",
                      type.c_str()),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_DCBP_DCBP,
        new TH2D("m_DCBP_DCBP",
                 Form("Total %s gamma-gamma DC for binary partner;Energy "
                      "[keV];Energy [keV];",
                      type.c_str()),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_Theta,
        new TH2D(
            "m_E_Theta",
            Form("Total %s gamma vs ion-gamma angle;Angle [deg];Energy [keV];",
                 type.c_str()),
            360, 0, 180,
            static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                             / gammadetectorConf.binWidth),
            0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EBP_Theta,
        new TH2D("m_EBP_Theta",
                 Form("Total %s gamma vs BP ion-gamma angle;Angle "
                      "[deg];Energy [keV];",
                      type.c_str()),
                 360, 0, 180,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDC_Theta,
        new TH2D("m_EDC_Theta",
                 Form("Total %s DC gamma vs ion-gamma angle;Angle "
                      "[deg];Energy [keV];",
                      type.c_str()),
                 360, 0, 180,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_EDCBP_Theta,
        new TH2D("m_EDCBP_Theta",
                 Form("Total %s DCBP gamma vs BP ion-gamma angle;Angle "
                      "[deg];Energy [keV];",
                      type.c_str()),
                 360, 0, 180,
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    for(auto& ion : conf.reaction.fragmentsOfInterest) {
        GammadetectorPrismaHistograms::IonHistograms ion_histos;
        ion_histos.dir = h.dir->GetDirectory(Form("Z%d", ion.first.Z));
        if(ion_histos.dir == nullptr) {
            ion_histos.dir = h.dir->mkdir(
                Form("Z%d", ion.first.Z),
                Form("Gamma spectra with cuts on the identified ion: Z=%d",
                     ion.first.Z));
        }
        TDirectory* tmpDir
            = ion_histos.dir->GetDirectory(Form("A%d", ion.first.A));
        if(tmpDir) {
            throw std::runtime_error(
                Form("A%d directory in Z%d already exists. Duplicate ion?",
                     ion.first.A, ion.first.Z));
        }
        ion_histos.dir = ion_histos.dir->mkdir(
            Form("A%d", ion.first.A),
            Form("Gamma spectra with cuts on the identified ion: A=%d",
                 ion.first.A));
        ion_histos.dir->cd();

        Initialize<TH1D>(
            ion_histos.h_noDC_ion,
            new TH1D(Form("h_noDC_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("Non-DC %s spectrum for A=%d and Z=%d;Energy "
                          "[keV];Counts/%d keV",
                          type.c_str(), ion.first.A, ion.first.Z,
                          gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH1D>(
            ion_histos.h_DC_ion,
            new TH1D(Form("h_DC_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("DC %s spectrum for A=%d and Z=%d;Energy "
                          "[keV];Counts/%d keV",
                          type.c_str(), ion.first.A, ion.first.Z,
                          gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH1D>(
            ion_histos.h_DCBP_ion,
            new TH1D(Form("h_DCBP_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("DC %s spectrum for binary partner for A=%d and "
                          "Z=%d;Energy [keV];Counts/%d keV",
                          type.c_str(),
                          conf.reaction.beam.A + conf.reaction.target.A
                              - ion.first.A,
                          conf.reaction.beam.Z + conf.reaction.target.Z
                              - ion.first.Z,
                          gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);

        Initialize<TH2D>(
            ion_histos.m_noDC_noDC_ion,
            new TH2D(Form("m_noDC_noDC_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("gamma-gamma non-DC %s spectrum for A=%d and "
                          "Z=%d;Energy [keV];Energy [keV];",
                          type.c_str(), ion.first.A, ion.first.Z),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_DC_DC_ion,
            new TH2D(Form("m_DC_DC_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("gamma-gamma DC %s spectrum for A=%d and "
                          "Z=%d;Energy [keV];Energy [keV];",
                          type.c_str(), ion.first.A, ion.first.Z),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_DCBP_DCBP_ion,
            new TH2D(Form("m_DCBP_DCBP_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("gamma-gamma DC %s spectrum for binary partner for "
                          "A=%d and Z=%d;Energy [keV];Energy [keV];",
                          type.c_str(),
                          conf.reaction.beam.A + conf.reaction.target.A
                              - ion.first.A,
                          conf.reaction.beam.Z + conf.reaction.target.Z
                              - ion.first.Z),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);

        Initialize<TH2D>(
            ion_histos.m_Theta_ThetaBP_ion,
            new TH2D(
                Form("m_Theta_ThetaBP_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("Theta vs Theta BP spectrum for "
                     "A=%d and Z=%d;Theta [deg];ThetaBP [deg];",
                     ion.first.A, ion.first.Z),
                180, 0., 180., 180, 0., 180.),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_Beta_BetaBP_ion,
            new TH2D(Form("m_Beta_BetaBP_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("Beta vs Beta BP spectrum for "
                          "A=%d and Z=%d;Beta [];BetaBP [];",
                          ion.first.A, ion.first.Z),
                     200, 0., 0.2, 200, 0., 0.2),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_noDC_theta_ion,
            new TH2D(Form("m_noDC_theta_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("Non-DC gammma energy vs theta  %s spectrum for "
                          "A=%d and Z=%d;Theta [deg];Energy [keV];",
                          type.c_str(), ion.first.A, ion.first.Z),
                     180, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_noDC_thetaBP_ion,
            new TH2D(
                Form("m_noDC_thetaBP_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("Non-DC gammma energy vs theta of binary partner %s "
                     "spectrum for A=%d and Z=%d;Theta BP [deg];Energy [keV];",
                     type.c_str(),
                     conf.reaction.beam.A + conf.reaction.target.A
                         - ion.first.A,
                     conf.reaction.beam.Z + conf.reaction.target.Z
                         - ion.first.Z),
                180, 0, 180,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_DC_theta_ion,
            new TH2D(Form("m_DC_theta_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("DC gammma energy vs theta %s spectrum for A=%d and "
                          "Z=%d;Theta [deg];Energy [keV];",
                          type.c_str(), ion.first.A, ion.first.Z),
                     180, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_DCBP_theta_ion,
            new TH2D(
                Form("m_DCBP_theta_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("DC gammma energy vs theta %s spectrum for binary "
                     "partner for A=%d and Z=%d;Theta [deg];Energy [keV];",
                     type.c_str(),
                     conf.reaction.beam.A + conf.reaction.target.A
                         - ion.first.A,
                     conf.reaction.beam.Z + conf.reaction.target.Z
                         - ion.first.Z),
                180, 0, 180,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);

        Initialize<TH2D>(
            ion_histos.m_noDC_Qval_ion,
            new TH2D(Form("m_noDC_Qval_ion_%d_%d", ion.first.A, ion.first.Z),
                     Form("Non-DC gammma energy vs excitation energy (-Q "
                          "value) %s spectrum for A=%d and Z=%d;Total "
                          "Excitation Energy"
                          "[MeV];#gamma energy [keV];",
                          type.c_str(), ion.first.A, ion.first.Z),
                     600, conf.prisma.minExEne, conf.prisma.maxExEne,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_DC_Qval_ion,
            new TH2D(
                Form("m_DC_Qval_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("DC gammma energy vs excitation energy (-Q value) "
                     "%s spectrum for A=%d and Z=%d;Total Excitation Energy "
                     "[MeV];#gamma energy [keV];",
                     type.c_str(), ion.first.A, ion.first.Z),
                600, conf.prisma.minExEne, conf.prisma.maxExEne,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_DCBP_Qval_ion,
            new TH2D(
                Form("m_DCBP_Qval_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("DC gammma energy vs excitation energy (-Q value) "
                     "%s spectrum for binary partner for A=%d and "
                     "Z=%d;Total Excitation Energy [MeV];#gamma energy [keV];",
                     type.c_str(),
                     conf.reaction.beam.A + conf.reaction.target.A
                         - ion.first.A,
                     conf.reaction.beam.Z + conf.reaction.target.Z
                         - ion.first.Z),
                600, conf.prisma.minExEne, conf.prisma.maxExEne,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_dTOF_EDC_ion,
            new TH2D(
                Form("m_dTOF_EDC_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("Delta TOF vs DC gammma energy spectrum for ion A=%d and "
                     "Z=%d;TOF [ns];Energy [keV];",
                     ion.first.A, ion.first.Z),
                400, -20, 20,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);
        Initialize<TH2D>(
            ion_histos.m_colNr_EDC_ion,
            new TH2D(
                Form("m_colNr_EDC_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("Column number vs DC gammma energy spectrum for "
                     "ion A=%d and Z=%d;TOF [ns];Energy [keV];",
                     ion.first.A, ion.first.Z),
                10,-0.5,9.5,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);   
        Initialize<TH2D>(
            ion_histos.m_Xfp_EDC_ion,
            new TH2D(
                Form("m_Xfp_EDC_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("X_FP vs DC gammma energy spectrum for "
                     "ion A=%d and Z=%d;TOF [ns];Energy [keV];",
                     ion.first.A, ion.first.Z),
                1000,0,1,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);   
        Initialize<TH2D>(
            ion_histos.m_mcpX_EDC_ion,
            new TH2D(
                Form("m_mcpX_EDC_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("MCP X vs DC gammma energy spectrum for "
                     "ion A=%d and Z=%d;TOF [ns];Energy [keV];",
                     ion.first.A, ion.first.Z),
                1000,-0.05,0.05,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);   
        Initialize<TH2D>(
            ion_histos.m_mcpY_EDC_ion,
            new TH2D(
                Form("m_mcpY_EDC_ion_%d_%d", ion.first.A, ion.first.Z),
                Form("MCP Y vs DC gammma energy spectrum for "
                     "ion A=%d and Z=%d;TOF [ns];Energy [keV];",
                     ion.first.A, ion.first.Z),
                1000,-0.05,0.05,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);            
        h.ionHistos[ion.first] = ion_histos;

        // Histograms with condition on Excitation energy
        for(unsigned int i{0}; i < gammadetectorPrismaConf.exValues.size();
            ++i) {
            h.ionHistos[ion.first].exSelection.emplace_back();
            auto& vals = gammadetectorPrismaConf.exValues.at(i);

            h.ionHistos[ion.first].exSelection.back().vals = vals;
            h.ionHistos[ion.first].exSelection.back().dir
                = h.ionHistos[ion.first].dir->mkdir(
                    Form("ExGate_Ei%.3f_Ef%.3f", vals.first, vals.second),
                    Form("Ex gated histograms, Ei=%.3f Ef=%.3f", vals.first,
                         vals.second));
            h.ionHistos[ion.first].exSelection.back().dir->cd();

            Initialize<TH1D>(
                h.ionHistos[ion.first].exSelection.back().h_E,
                new TH1D(
                    "h_E", "Gamma energy; Energy [keV]; Counts/keV",
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH1D>(
                h.ionHistos[ion.first].exSelection.back().h_EDC,
                new TH1D(
                    "h_EDC",
                    " Doppler-corrected energy; Energy [keV]; Counts/keV",
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH1D>(
                h.ionHistos[ion.first].exSelection.back().h_EDCBP,
                new TH1D(
                    "h_EDCBP",
                    "Doppler-corrected energy for binary partner; Energy "
                    "[keV]; Counts/keV",
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH2D>(
                h.ionHistos[ion.first].exSelection.back().m_E_Theta,
                new TH2D(
                    "m_E_Theta", "Gamma-theta; Theta; Energy [keV]", 1000, 0,
                    180,
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH2D>(
                h.ionHistos[ion.first].exSelection.back().m_E_ThetaBP,
                new TH2D(
                    "m_E_ThetaBP",
                    "Gamma-theta for binary partner; Theta; Energy [keV]",
                    1000, 0, 180,
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH2D>(
                h.ionHistos[ion.first].exSelection.back().m_EDCBP_Theta,
                new TH2D(
                    "m_EDCBP_Theta", " DCBP energy-theta; Theta; Energy [keV]",
                    1000, 0, 180,
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH2D>(
                h.ionHistos[ion.first].exSelection.back().m_EDCBP_ThetaBP,
                new TH2D(
                    "m_EDCBP_ThetaBP",
                    "DCBP energy - theta for binary partner; Theta; "
                    "Energy [keV]",
                    1000, 0, 180,
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH2D>(
                h.ionHistos[ion.first].exSelection.back().m_E_E,
                new TH2D(
                    "m_E_E", "Gamma-gamma energy;Energy [keV];Energy [keV]",
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV,
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH2D>(
                h.ionHistos[ion.first].exSelection.back().m_EDC_EDC,
                new TH2D(
                    "m_EDC_EDC",
                    "Doppler corrected gamma-gamma energy;Energy "
                    "[keV];Energy [keV]",
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV,
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);

            Initialize<TH2D>(
                h.ionHistos[ion.first].exSelection.back().m_EDCBP_EDCBP,
                new TH2D(
                    "m_EDCBP_EDCBP",
                    "Doppler corrected gamma-gamma energy for binary "
                    "partner;Energy [keV];Energy [keV]",
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV,
                    static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                     / gammadetectorConf.binWidth),
                    0, gammadetectorConf.gammaEmax / UNITS::keV),
                h.ionHistos[ion.first].exSelection.back().ptrs);
        }
    }

    // Look for TCuts
    conf.AddCuts<GammadetectorPrismaHistograms::DE_E>("/CUT/PRISMA/IC_GAMMAGATE/",
                                                      h.cutHistos);
    for(auto& it : h.cutHistos) {
        it.dir = h.dir->mkdir(Form("DE_E_Cuts_%s", it.cut->GetName()),
                              "Gamma spectra with ionization chamber cuts");
        it.dir->cd();
        it.cut->Write();

        Initialize<TH1D>(
            it.h_noDC_cut,
            new TH1D(Form("h_noDC_cut_%s", it.cut->GetName()),
                     Form("Non-DC %s spectrum with cut %s;Energy "
                          "[keV];Counts/%d keV",
                          type.c_str(), it.cut->GetName(),
                          gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH1D>(
            it.h_DC_cut,
            new TH1D(
                Form("h_DC_cut_%s", it.cut->GetName()),
                Form("DC %s spectrum with cut %s;Energy [keV];Counts/%d keV",
                     type.c_str(), it.cut->GetName(),
                     gammadetectorConf.binWidth),
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH1D>(
            it.h_DCBP_cut,
            new TH1D(Form("h_DCBP_cut_%s", it.cut->GetName()),
                     Form("DC %s spectrum for binary partner with cut "
                          "%s;Energy [keV];Counts/%d keV",
                          type.c_str(), it.cut->GetName(),
                          gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);

        Initialize<TH2D>(
            it.m_Theta_ThetaBP_cut,
            new TH2D(Form("m_Theta_ThetaBP_cut_%s", it.cut->GetName()),
                     Form("Theta vs Theta BP spectrum for "
                          "cut %s;Theta [deg];ThetaBP [deg];",
                          it.cut->GetName()),
                     180, 0, 180, 180, 0, 180),
            it.ptrs);
        Initialize<TH2D>(
            it.m_Beta_BetaBP_cut,
            new TH2D(Form("m_Beta_BetaBP_cut_%s", it.cut->GetName()),
                     Form("Beta vs Beta BP spectrum for "
                          "cut %s;Beta [];BetaBP [];",
                          it.cut->GetName()),
                     200, 0, 0.2, 200, 0, 0.2),
            it.ptrs);

        Initialize<TH2D>(
            it.m_noDC_noDC_cut,
            new TH2D(Form("m_noDC_noDC_cut_%s", it.cut->GetName()),
                     Form("gamma-gamma non-DC %s spectrum with cut %s;Energy "
                          "[keV];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_DC_DC_cut,
            new TH2D(Form("m_DC_DC_cut_%s", it.cut->GetName()),
                     Form("gamma-gamma DC %s spectrum with cut %s;Energy "
                          "[keV];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_DCBP_DCBP_cut,
            new TH2D(Form("m_DCBP_DCBP_cut_%s", it.cut->GetName()),
                     Form("gamma-gamma DC %s spectrum for binary partner with "
                          "cut %s;Energy [keV];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);

        Initialize<TH2D>(
            it.m_noDC_theta_cut,
            new TH2D(Form("m_noDC_theta_cut_%s", it.cut->GetName()),
                     Form("Non-DC gammma energy vs theta  %s spectrum with "
                          "cut %s;Theta [deg];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     180, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_noDC_thetaBP_cut,
            new TH2D(Form("m_noDC_thetaBP_cut_%s", it.cut->GetName()),
                     Form("Non-DC gammma energy vs theta of binary partner %s "
                          "spectrum with cut %s;Theta BP [deg];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     180, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_DC_theta_cut,
            new TH2D(Form("m_DC_theta_cut_%s", it.cut->GetName()),
                     Form("DC gammma energy vs theta %s spectrum with cut "
                          "%s;Theta [deg];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     180, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_DCBP_theta_cut,
            new TH2D(Form("m_DCBP_theta_cut_%s", it.cut->GetName()),
                     Form("DC gammma energy vs theta %s spectrum for binary "
                          "partner with cut %s;Theta [deg];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     180, 0, 180,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);

        Initialize<TH2D>(
            it.m_noDC_Qval_cut,
            new TH2D(
                Form("m_noDC_Qval_cut_%s", it.cut->GetName()),
                Form("Non-DC gammma energy vs excitation energy (-Q value) %s "
                     "spectrum with cut %s;Total Excitation Energy "
                     "[MeV];#gamma energy [keV];",
                     type.c_str(), it.cut->GetName()),
                600, conf.prisma.minExEne, conf.prisma.maxExEne,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_DC_Qval_cut,
            new TH2D(
                Form("m_DC_Qval_cut_%s", it.cut->GetName()),
                Form("DC gammma energy vs excitation energy (-Q value) %s "
                     "spectrum with cut %s;Total Excitation Energy "
                     "[MeV];#gamma energy [keV];",
                     type.c_str(), it.cut->GetName()),
                600, conf.prisma.minExEne, conf.prisma.maxExEne,
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_DCBP_Qval_cut,
            new TH2D(Form("m_DCBP_Qval_cut_%s", it.cut->GetName()),
                     Form("DC gammma energy vs excitation energy (-Q value) "
                          "%s spectrum for binary partner with cut %s;Total "
                          "Excitation Energy "
                          "[MeV];#gamma energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     600, conf.prisma.minExEne, conf.prisma.maxExEne,

                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
        Initialize<TH2D>(
            it.m_dTOF_EDC_cut,
            new TH2D(Form("m_dTOF_EDC_cut_%s", it.cut->GetName()),
                     Form("Delta TOF vs E DC %s spectrum with cut %s;dTOF "
                          "[ns];Energy [keV];",
                          type.c_str(), it.cut->GetName()),
                     200, -20, 20,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            it.ptrs);
    }
    return true;
}

void GammadetectorPrismaAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();
    gammaDC.clear();
    //if(!in_coinc) return;

    if(!prisma.mcp_ok && conf.prisma.require_mcpok) { return; }
    if(!prisma.tof_ok && conf.prisma.require_tofok) { return; }
    if(!prisma.side_ok && conf.prisma.require_sideok) { return; }
    if(!prisma.traj_ok && conf.prisma.require_trajok) { return; }
    if(!prisma.ic_ok && conf.prisma.require_icok) { return; }
    //std::cout << "coinc size" << coincMatrix.size() << std::endl;

    if(coincMatrix.size()==1)  gammaDC.resize(coincMatrix.at(0).size());
    else return;


    for(size_t v{0}; v<coincMatrix.size(); v++){

        auto &currentIon = gammaDC;

        ReactionFragment::Ion ion{prisma.A, prisma.Z};
        ReactionFragment::Ion ionBP  = conf.reaction.target + conf.reaction.beam - ion;

        TVector3 Beta3, Beta4;

        if(prisma.p3.Mag() != 0) {
        // Fragment is identified

            currentIon.p3
                = SlowInTarget(prisma.p3, gammadetectorPrismaConf.dcPos, ion);
            currentIon.p4 = SlowInTarget(prisma.p4,
                                      gammadetectorPrismaConf.bpDcPos, ionBP);

            Beta3 = currentIon.p3.BoostVector();
            Beta4 = currentIon.p4.BoostVector();

        } else {
            if(prisma.beta.Mag() > 0) {
                // Fragment has beta and either is not identified or no energy
                // losses are calculated
                Beta3 = prisma.beta;
                Beta4 = TVector3{0,0,0};
            } else {
                // Fragment has no beta, take the average beta
                Beta3 = conf.prisma.betaAvg;
                Beta4 = TVector3{0,0,0};
            }
        }
        
        currentIon.redgammaDC.rE.clear();
        currentIon.redgammaDC.rEDC.clear();
        currentIon.redgammaDC.rEDCBP.clear();

        currentIon.redgammaDC.rE.reserve(gammadetector.P.size());
        currentIon.redgammaDC.rEDC.reserve(gammadetector.P.size());
        currentIon.redgammaDC.rEDCBP.reserve(gammadetector.P.size());

        TVector3 decayPos = {0,0,0};
        TVector3 decayPosBP = {0,0,0};

        for(size_t s{0}; s<gammadetector.P.size(); s++){
            if(coincMatrix.at(v).at(s)){
                currentIon.inCoinc.at(s) = true;
            } else {
                currentIon.inCoinc.at(s) = false;
            }

            if(gammadetectorPrismaConf.lifetime_ion>0){
                TVector3 decayPos = Beta3*gammadetectorPrismaConf.lifetime_ion*UNITS::PHYSICS::c_light;
                TVector3 relGammaPos = gammadetector.Pos.at(s) - decayPos;
                gammadetector.P.at(s) = TLorentzVector(0, 0, gammadetector.P.at(s).E(), gammadetector.P.at(s).E());
                gammadetector.P.at(s).SetTheta(relGammaPos.Theta());
                gammadetector.P.at(s).SetPhi(relGammaPos.Phi());
            }
            if(gammadetectorPrismaConf.lifetime_ionBP>0){
                TVector3 decayPosBP = Beta4*gammadetectorPrismaConf.lifetime_ionBP*UNITS::PHYSICS::c_light;
                TVector3 relGammaPos = gammadetector.Pos.at(s) - decayPosBP;
                gammadetector.P.at(s) = TLorentzVector(0, 0, gammadetector.P.at(s).E(), gammadetector.P.at(s).E());
                gammadetector.P.at(s).SetTheta(relGammaPos.Theta());
                gammadetector.P.at(s).SetPhi(relGammaPos.Phi());
            }

            currentIon.E.at(s)     = gammadetector.P.at(s);
            currentIon.EDC.at(s)   = gammadetector.P.at(s);
            currentIon.EDCBP.at(s) = gammadetector.P.at(s);

            currentIon.EDC.at(s).Boost(-Beta3);
            currentIon.EDCBP.at(s).Boost(-Beta4);

            if(coincMatrix.at(v).at(s)){
                currentIon.redgammaDC.rE.push_back(currentIon.E.at(s).E());
                currentIon.redgammaDC.rEDC.push_back(currentIon.EDC.at(s).E());
                currentIon.redgammaDC.rEDCBP.push_back(currentIon.EDCBP.at(s).E());
            }

        }

    }





//    unsigned int nCoinc = 0;
//    auto prismatime{prisma.Time.front()};
//
//    for(unsigned int i{0}; i < gammadetector.E.size(); ++i) {
//
//        auto gammatime{gammadetector.Time.at(i)};
//
//        auto diff = unsignedDiff<double, unsigned long long>(prisma.TS, gammadetector.TS)
//                          + (prismatime - gammatime) / (10. * UNITS::ns);
//
//        Fill(gammadetectorPrismaHistograms.h_cTime, diff);
//        Fill(gammadetectorPrismaHistograms.m_cTime_coreId, diff, gammadetector.CoreID.at(i));
//
//        if(diff>gammadetectorPrismaConf.windowLeft&&diff<gammadetectorPrismaConf.windowRight)  nCoinc++;
//    }
//    //std::cout<< "Ngammas "<<  gammadetector.E.size() << "    Ncoinc "<<nCoinc<<std::endl;
//    gammaDC.resize(nCoinc);
//
//    unsigned int idCoinc = 0;
//
//    for(unsigned int i{0}; i < gammadetector.E.size(); ++i) {
//
//        auto gammatime{gammadetector.Time.at(i)};
//
//        auto diff = unsignedDiff<double, unsigned long long>(prisma.TS, gammadetector.TS)
//                          + (prismatime - gammatime) / (10. * UNITS::ns);
//
//        if(diff>gammadetectorPrismaConf.windowLeft&&diff<gammadetectorPrismaConf.windowRight){
//            gammaDC.E.at(idCoinc)     = gammadetector.P.at(i);
//            gammaDC.EDC.at(idCoinc)   = gammadetector.P.at(i);
//            gammaDC.EDCBP.at(idCoinc) = gammadetector.P.at(i);
//            //std::cout<< "1 gammaDC E "<<  gammaDC.E.at(idCoinc).E() << "    gammaDC EDC "<<  gammaDC.EDC.at(idCoinc).E() << "    gammaDC EDCBP "<<  gammaDC.EDCBP.at(idCoinc).E() << std::endl;
//
//            idCoinc++;
//        }
//
//
//
//    }
//
//    for(unsigned int j{0}; j < gammaDC.E.size(); ++j) {
//            if(prisma.p3.Mag() != 0) {
//            // Fragment is identified
//            ReactionFragment::Ion ion{prisma.A, prisma.Z};
//            ReactionFragment::Ion ionBP
//                = conf.reaction.target + conf.reaction.beam - ion;
//
//            gammaDC.p3
//                = SlowInTarget(prisma.p3, gammadetectorPrismaConf.dcPos, ion);
//            gammaDC.p4 = SlowInTarget(prisma.p4,
//                                      gammadetectorPrismaConf.bpDcPos, ionBP);
//
//            gammaDC.EDC.at(j).Boost(-gammaDC.p3.BoostVector());
//            gammaDC.EDCBP.at(j).Boost(-gammaDC.p4.BoostVector());
//        } else {
//            if(prisma.beta.Mag() > 0) {
//                // Fragment has beta and either is not identified or no energy
//                // losses are calculated
//                gammaDC.EDC.at(j).Boost(-prisma.beta);
//            } else {
//                // Fragment has no beta, take the average beta
//                gammaDC.EDC.at(j).Boost(-conf.prisma.betaAvg);
//            }
//        }
//       // std::cout<< "2 gammaDC E "<<  gammaDC.E.at(j).E() << "    gammaDC EDC "<<  gammaDC.EDC.at(j).E() << "    gammaDC EDCBP "<<  gammaDC.EDCBP.at(j).E() << std::endl;
//
//    }
}

bool GammadetectorPrismaAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    if(!prisma.mcp_ok && conf.prisma.require_mcpok) { return true; }
    if(!prisma.tof_ok && conf.prisma.require_tofok) { return true; }
    if(!prisma.side_ok && conf.prisma.require_sideok) { return true; }
    if(!prisma.traj_ok && conf.prisma.require_trajok) { return true; }
    if(!prisma.ic_ok && conf.prisma.require_icok) { return true; }

    GammadetectorPrismaHistograms& h = gammadetectorPrismaHistograms;
    Fill(h.m_ICDEAB_ICE, prisma.E, prisma.dE);

    for(unsigned int i{0}; i < gammaDC.E.size(); ++i) {
        auto diff = unsignedDiff<double, unsigned long long>(prisma.TS,
                                                             gammadetector.TS)
                  + (prisma.Time.at(0) - gammadetector.Time.at(i))
                        / (10. * UNITS::ns);

        Fill(h.h_cTime, diff);
    }

    for(unsigned int i{0}; i < gammaDC.E.size(); ++i) {
        if(!gammaDC.inCoinc.at(i)) continue;
        auto diff = unsignedDiff<double, unsigned long long>(prisma.TS,
                                                             gammadetector.TS)
                  + (prisma.Time.at(0) - gammadetector.Time.at(i))
                        / (10. * UNITS::ns);

        Fill(h.m_cTime_E, diff, gammadetector.E.at(i));

        // if(!prisma.all_ok) continue;
        // if(prisma.beta.Mag2() <= 0) continue;
        Fill(h.h_cTime_gated, diff);

        // fill 1-D histograms
        Fill(h.h_E, gammaDC.E.at(i).E());
        Fill(h.h_DC, gammaDC.EDC.at(i).E());
        Fill(h.m_cTime_EDC, diff, gammaDC.EDC.at(i).E());
        Fill(h.h_DCBP, gammaDC.EDCBP.at(i).E());
        Fill(h.m_cTime_EDCBP, diff, gammaDC.EDCBP.at(i).E());

        // E gamma vs theta
        Fill(h.m_E_Theta,
             prisma.beta.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
             gammadetector.E.at(i));
        Fill(h.m_EBP_Theta,
             gammaDC.p4.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
             gammadetector.E.at(i));
        Fill(h.m_EDC_Theta,
             prisma.beta.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
             gammaDC.EDC.at(i).E());
        Fill(h.m_EDCBP_Theta,
             gammaDC.p4.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
             gammaDC.EDC.at(i).E());

        // TGcutG gated histogrmas
        for(const auto& it : h.cutHistos) {
            if(it.cut->IsInside(prisma.E, prisma.dE)) {
                Fill(it.h_noDC_cut, gammaDC.E.at(i).E());
                double         EDC   = 0;
                double         EDCBP = 0;
                double         exE   = 0;
                TLorentzVector pSlow;
                TLorentzVector pBPslow;

                /*  try {*/
                std::string    tmp = it.cut->GetName();
                if(tmp.find('z') != std::string::npos
                   && tmp.find('a') != std::string::npos) {
                    tmp.replace(tmp.find('_'), 1, " ");
                    tmp.insert(tmp.find('z') + 1, " ");
                    tmp.insert(tmp.find('a') + 1, " ");

                    std::stringstream iss;
                    iss.str(tmp);
                    unsigned int num1, num2;
                    std::string  tag1, tag2;
                    if(iss >> tag1 >> num1 >> tag2 >> num2) {
                        unsigned int zed = 0, mass = 0;
                        if(tag1 == "z")
                            zed = num1;
                        else
                            mass = num1;
                        if(tag2 == "a")
                            mass = num2;
                        else
                            zed = num2;

                        ReactionFragment::Ion forcedIon{mass, zed};
                        unsigned int          A2 = conf.reaction.target.A
                                        + conf.reaction.beam.A - forcedIon.A;
                        unsigned int Z2 = conf.reaction.target.Z
                                        + conf.reaction.beam.Z - forcedIon.Z;
                        ReactionFragment::Ion ion4{A2, Z2};

                        auto reactionsIt = input.reactions.find(forcedIon);
                        if(reactionsIt != input.reactions.end()) {
                            // Evaluate mid-target energy
                            auto& ion3
                                = reactionsIt->second.GetFixedFragment();
                            ion3.Set_Beta(prisma.beta);
                            TLorentzVector p = ion3.Get_P();
                            EvalInTarget(p, forcedIon);

                            exE = static_cast<double>(
                                reactionsIt->second.Set_E_Theta_Phi(
                                    p.E(), p.Theta(), p.Phi()));
                            TLorentzVector pBP
                                = reactionsIt->second.GetFreeFragment().Get_P();

                            pSlow = SlowInTarget(
                                p, gammadetectorPrismaConf.dcPos, forcedIon);
                            pBPslow = SlowInTarget(
                                pBP, gammadetectorPrismaConf.bpDcPos, ion4);
                            TLorentzVector gP   = gammadetector.P.at(i);
                            TLorentzVector gPBP = gammadetector.P.at(i);
                            Fill(it.m_Beta_BetaBP_cut,
                                 pSlow.BoostVector().Mag(),
                                 pBPslow.BoostVector().Mag());
                            Fill(it.m_Theta_ThetaBP_cut,
                                 pSlow.Theta() / UNITS::deg,
                                 pBPslow.Theta() / UNITS::deg);
                            gP.Boost(-pSlow.BoostVector());
                            gPBP.Boost(-pBPslow.BoostVector());
                            EDC   = gP.E();
                            EDCBP = gPBP.E();
                        }
                    }
                } else {
                    Fill(it.m_Beta_BetaBP_cut, 0., 0.);
                    Fill(it.m_Theta_ThetaBP_cut, 0., 0.);
                    EDC   = gammaDC.EDC.at(i).E();
                    EDCBP = gammaDC.EDCBP.at(i).E();
                    exE   = prisma.TKEL;
                }
                Fill(it.h_DC_cut, EDC);
                Fill(it.h_DCBP_cut, EDCBP);

                for(double dTOF = -20; dTOF < 20; dTOF+=0.1) {
                    double tof  = prisma.TOF + static_cast<double>(dTOF) * UNITS::ns;
                    double beta = (prisma.Length) / tof
                                / UNITS::PHYSICS::c_light;

                    TVector3       betatemp = prisma.beta;
		    betatemp.SetMag(beta);

                    TLorentzVector ptemp    = gammadetector.P.at(i);
                    ptemp = gammadetector.P.at(i);
                    ptemp.Boost(-betatemp);
                    Fill(it.m_dTOF_EDC_cut, dTOF, ptemp.E());
                }

                Fill(it.m_noDC_theta_cut,
                     prisma.beta.Angle(gammadetector.P.at(i).Vect())
                         / UNITS::deg,
                     gammadetector.E.at(i));
                Fill(it.m_noDC_thetaBP_cut,
                     pBPslow.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
                     gammadetector.E.at(i));
                Fill(it.m_DC_theta_cut,
                     prisma.beta.Angle(gammadetector.P.at(i).Vect())
                         / UNITS::deg,
                     EDC);
                Fill(it.m_DCBP_theta_cut,
                     pBPslow.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
                     EDCBP);

                // E gamma vs excitation energy
                Fill(it.m_noDC_Qval_cut, exE, gammadetector.E.at(i));
                Fill(it.m_DC_Qval_cut, exE, EDC);
                Fill(it.m_DCBP_Qval_cut, exE, EDCBP);
            }
        }

        // Detected ion specific gated histogrmas
        ReactionFragment::Ion detectedIon{prisma.A, prisma.Z};
        auto                  histoIt = h.ionHistos.find(detectedIon);
        if(histoIt != h.ionHistos.end()) {
            Fill(histoIt->second.h_noDC_ion, gammadetector.E.at(i));
            Fill(histoIt->second.h_DC_ion, gammaDC.EDC.at(i).E());
            Fill(histoIt->second.h_DCBP_ion, gammaDC.EDCBP.at(i).E());

            // Beta scan
            for(double dTOF = -20; dTOF < 20; dTOF+=0.1) {
                double tof  = prisma.TOF + static_cast<double>(dTOF) * UNITS::ns;
                double beta = (prisma.Length) / tof
		  / UNITS::PHYSICS::c_light; //(prisma.Length * UNITS::mm)

                TVector3       betatemp = prisma.beta;
		betatemp.SetMag(beta);

                TLorentzVector ptemp    = gammadetector.P.at(i);
                ptemp = gammadetector.P.at(i);
                ptemp.Boost(-betatemp);
                Fill(histoIt->second.m_dTOF_EDC_ion, dTOF, ptemp.E());
            }

            Fill(histoIt->second.m_Beta_BetaBP_ion,
                 gammaDC.p3.BoostVector().Mag(),
                 gammaDC.p4.BoostVector().Mag());
            Fill(histoIt->second.m_Theta_ThetaBP_ion,
                 gammaDC.p3.Theta() / UNITS::deg,
                 gammaDC.p4.Theta() / UNITS::deg);

            // E gamma vs theta
            Fill(histoIt->second.m_noDC_theta_ion,
                 prisma.beta.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
                 gammadetector.E.at(i));
            Fill(histoIt->second.m_noDC_thetaBP_ion,
                 gammaDC.p4.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
                 gammadetector.E.at(i));
            Fill(histoIt->second.m_DC_theta_ion,
                 prisma.beta.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
                 gammaDC.EDC.at(i).E());
            Fill(histoIt->second.m_DCBP_theta_ion,
                 gammaDC.p4.Angle(gammadetector.P.at(i).Vect()) / UNITS::deg,
                 gammaDC.EDCBP.at(i).E());

            //E gamma vs Prisma positions
            Fill(histoIt->second.m_colNr_EDC_ion, prisma.colNr, gammaDC.EDC.at(i).E());
            Fill(histoIt->second.m_Xfp_EDC_ion, prisma.X_FP, gammaDC.EDC.at(i).E());
            Fill(histoIt->second.m_mcpX_EDC_ion, prisma.MCP_X, gammaDC.EDC.at(i).E());
            Fill(histoIt->second.m_mcpY_EDC_ion, prisma.MCP_Y, gammaDC.EDC.at(i).E());


            // E gamma vs excitation energy
            Fill(histoIt->second.m_noDC_Qval_ion, prisma.TKEL,
                 gammadetector.E.at(i));
            Fill(histoIt->second.m_DC_Qval_ion, prisma.TKEL,
                 gammaDC.EDC.at(i).E());
            Fill(histoIt->second.m_DCBP_Qval_ion, prisma.TKEL,
                 gammaDC.EDCBP.at(i).E());

            for(auto& it : histoIt->second.exSelection) {
                GammadetectorPrismaHistograms::IonHistograms::ExSelection& hex
                    = it; // Shadowing other reference!
                if((prisma.TKEL > hex.vals.first)
                   && (prisma.TKEL < hex.vals.second)) {
                    Fill(hex.h_E, gammadetector.E.at(i));
                    Fill(hex.h_EDC, gammaDC.EDC.at(i).E());
                    Fill(hex.h_EDCBP, gammaDC.EDCBP.at(i).E());

                    Fill(hex.m_E_Theta,
                         prisma.p3.Angle(gammadetector.P.at(i).Vect())
                             / UNITS::deg,
                         gammadetector.E.at(i));
                    Fill(hex.m_E_ThetaBP,
                         prisma.p4.Angle(gammadetector.P.at(i).Vect())
                             / UNITS::deg,
                         gammadetector.E.at(i));

                    Fill(hex.m_EDCBP_Theta,
                         prisma.p3.Angle(gammadetector.P.at(i).Vect())
                             / UNITS::deg,
                         gammaDC.EDCBP.at(i).E());
                    Fill(hex.m_EDCBP_ThetaBP,
                         prisma.p4.Angle(gammadetector.P.at(i).Vect())
                             / UNITS::deg,
                         gammaDC.EDCBP.at(i).E());
                }
            }
        }

        // gamma-gamma no DC filling loop
        for(unsigned int j = 0; j < i; j++) {
            if(!gammaDC.inCoinc.at(j)) continue;
            Fill(h.m_E_E, gammadetector.E.at(i), gammadetector.E.at(j));
            Fill(h.m_E_E, gammadetector.E.at(j), gammadetector.E.at(i));
            for(const auto& it : h.cutHistos) {
                if(it.cut->IsInside(prisma.E, prisma.dE)) {
                    Fill(it.m_noDC_noDC_cut, gammadetector.E.at(i),
                         gammadetector.E.at(j));
                    Fill(it.m_noDC_noDC_cut, gammadetector.E.at(j),
                         gammadetector.E.at(i));
                }
            }
            if(histoIt != h.ionHistos.end()) {
                Fill(histoIt->second.m_noDC_noDC_ion, gammadetector.E.at(i),
                     gammadetector.E.at(j));
                Fill(histoIt->second.m_noDC_noDC_ion, gammadetector.E.at(j),
                     gammadetector.E.at(i));
                for(auto& it : histoIt->second.exSelection) {
                    GammadetectorPrismaHistograms::IonHistograms::ExSelection&
                        hex
                        = it; // Shadowing other reference!
                    if((prisma.TKEL > hex.vals.first)
                       && (prisma.TKEL < hex.vals.second)) {
                        Fill(hex.m_E_E, gammadetector.E.at(i),
                             gammadetector.E.at(j));
                        Fill(hex.m_E_E, gammadetector.E.at(j),
                             gammadetector.E.at(i));
                    }
                }
            }
        }

        // gamma-gamma with DC filling loop
        for(unsigned int j = 0; j < i ; j++) {
            if(!gammaDC.inCoinc.at(j)) continue;
            Fill(h.m_DC_DC, gammaDC.EDC.at(i).E(), gammaDC.EDC.at(j).E());
            Fill(h.m_DC_DC, gammaDC.EDC.at(j).E(), gammaDC.EDC.at(i).E());

            Fill(h.m_DCBP_DCBP, gammaDC.EDCBP.at(i).E(),
                 gammaDC.EDCBP.at(j).E());
            Fill(h.m_DCBP_DCBP, gammaDC.EDCBP.at(j).E(),
                 gammaDC.EDCBP.at(i).E());

            for(const auto& it : h.cutHistos) {
                if(it.cut->IsInside(prisma.E, prisma.dE)) {
                    Fill(it.m_DC_DC_cut, gammaDC.EDC.at(i).E(),
                         gammaDC.EDC.at(j).E());
                    Fill(it.m_DC_DC_cut, gammaDC.EDC.at(j).E(),
                         gammaDC.EDC.at(i).E());

                    Fill(it.m_DCBP_DCBP_cut, gammaDC.EDCBP.at(i).E(),
                         gammaDC.EDCBP.at(j).E());
                    Fill(it.m_DCBP_DCBP_cut, gammaDC.EDCBP.at(j).E(),
                         gammaDC.EDCBP.at(i).E());
                }
            }
            if(histoIt != h.ionHistos.end()) {
                Fill(histoIt->second.m_DC_DC_ion, gammaDC.EDC.at(i).E(),
                     gammaDC.EDC.at(j).E());
                Fill(histoIt->second.m_DC_DC_ion, gammaDC.EDC.at(j).E(),
                     gammaDC.EDC.at(i).E());

                Fill(histoIt->second.m_DCBP_DCBP_ion, gammaDC.EDCBP.at(i).E(),
                     gammaDC.EDCBP.at(j).E());
                Fill(histoIt->second.m_DCBP_DCBP_ion, gammaDC.EDCBP.at(j).E(),
                     gammaDC.EDCBP.at(i).E());
                for(auto& it : histoIt->second.exSelection) {
                    GammadetectorPrismaHistograms::IonHistograms::ExSelection&
                        hex
                        = it; // Shadowing other reference!
                    if((prisma.TKEL > hex.vals.first)
                       && (prisma.TKEL < hex.vals.second)) {
                        Fill(hex.m_EDC_EDC, gammaDC.EDC.at(i).E(),
                             gammaDC.EDC.at(j).E());
                        Fill(hex.m_EDC_EDC, gammaDC.EDC.at(j).E(),
                             gammaDC.EDC.at(i).E());

                        Fill(hex.m_EDCBP_EDCBP, gammaDC.EDCBP.at(i).E(),
                             gammaDC.EDCBP.at(j).E());
                        Fill(hex.m_EDCBP_EDCBP, gammaDC.EDCBP.at(j).E(),
                             gammaDC.EDCBP.at(i).E());
                    }
                }
            }
        }
    }
    return true;
}

void GammadetectorPrismaAnalysis::Finalize() {
    CoincidenceAnalysis::Finalize();
}
void GammadetectorPrismaAnalysis::Clear() { gammaDC.clear(); }
