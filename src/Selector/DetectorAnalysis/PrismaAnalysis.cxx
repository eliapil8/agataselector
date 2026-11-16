#include "PrismaAnalysis.h"
#include "AgataSelector.h"

#include "Prisma.h"
#include "ReactionFragment.h"

#include <iomanip>

PrismaAnalysis::PrismaAnalysis(DetectorAnalysis::Inputs in, Prisma &p, PrismaInput &pi)
    : DetectorAnalysis(in, pi, ConfManager::getInstance().prisma, prismaHistograms),
      PrismaInputAnalysis(in, pi), prisma(p),
      thetaCMLimits(conf.replay.conf_path + conf.prisma.thetaCMLimitsPath)
{
    if (conf.prisma.analysis_stages == PrismaConf::AnalysisStage::RAWPROCESS ||
        conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANAPROCESS)
        return;

    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Prisma", &prisma);

    if (!conf.prisma.prismaUnitsCuts)
    {
        units_aoq    = 1.;
        units_length = 1.;
        units_time   = 1.;
    }

    if (conf.prisma.acceptance_mask_file != "")
    {
        TFile *file_mask = new TFile(conf.prisma.acceptance_mask_file.c_str());
        if (file_mask == nullptr)
        {
            std::cerr << "Error opening file " << conf.prisma.acceptance_mask_file << std::endl;
            return;
        }
        mask_cut = (TCutG *)file_mask->Get("mask_acceptance");
    }
}

bool PrismaAnalysis::AllocateHistos()
{
    if (conf.prisma.analysis_stages != PrismaConf::AnalysisStage::HIGHLEVEL)
        PrismaInputAnalysis::AllocateHistos();

    if (conf.prisma.analysis_stages == PrismaConf::AnalysisStage::RAWPROCESS ||
        conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANAPROCESS)
        return true;

    PrismaHistograms &h = prismaHistograms;
    if (conf.prisma.minExEne > conf.prisma.maxExEne)
        throw std::runtime_error("Error in conf: MIN_EX_ENE>MAX_EX_ENE\n");

    h.dir = input.outFile.mkdir("Prisma", "Prisma standalone histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    // Raw variables
    if (conf.prisma.raw)
    {
        h.raw.dir = h.dir->mkdir("Raw", "Prisma raw histograms");
        h.raw.dir->cd();

        // MCP
        h.raw.mcp.dir = h.raw.dir->mkdir("MCP", "Prisma MCP raw histograms");
        h.raw.mcp.dir->cd();

        Initialize<TH2D>(
            h.raw.mcp.m_MCPX_MCPY,
            new TH2D("m_MCPX_MCPY_raw_0", "raw MCP_Y : raw MCP_X", 1024, 0, 4096, 1024, 0, 4096),
            h.raw.mcp.ptrs);

        // PPAC
        h.raw.ppac.dir = h.raw.dir->mkdir("PPAC", "Prisma PPAC raw histograms");
        h.raw.ppac.dir->cd();
        for (unsigned int i{0}; i < h.raw.ppac.h_Cathode.size(); i++)
        {
            Initialize<TH1D>(
                h.raw.ppac.h_Cathode.at(i),
                new TH1D(Form("h_Cathode_%i", i), Form("CATHODE section %i", i), 1024, 0, 4096),
                h.raw.ppac.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ppac.h_XR.size(); i++)
        {
            Initialize<TH1D>(
                h.raw.ppac.h_XR.at(i),
                new TH1D(Form("h_XR_%i", i), Form("X_RIGHT section %i", i), 1024, 0, 4096),
                h.raw.ppac.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ppac.h_XL.size(); i++)
        {
            Initialize<TH1D>(
                h.raw.ppac.h_XL.at(i),
                new TH1D(Form("h_XL_%i", i), Form("X_LEFT section %i", i), 1024, 0, 4096),
                h.raw.ppac.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ppac.h_XRmXL.size(); i++)
        {
            Initialize<TH1D>(h.raw.ppac.h_XRmXL.at(i),
                             new TH1D(Form("h_XRmXL_%i", i), Form("X_RIGHT - X_LEFT section %i", i),
                                      1024, -4096, 4096),
                             h.raw.ppac.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ppac.h_XRmCath.size(); i++)
        {
            Initialize<TH1D>(h.raw.ppac.h_XRmCath.at(i),
                             new TH1D(Form("h_XRmCath_%i", i),
                                      Form("X_RIGHT - CATHODE section %i", i), 1024, -2048, 2048),
                             h.raw.ppac.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ppac.h_CathmXL.size(); i++)
        {
            Initialize<TH1D>(h.raw.ppac.h_CathmXL.at(i),
                             new TH1D(Form("h_CathmXL_%i", i),
                                      Form("CATHODE - X_LEFT section %i", i), 1024, -2048, 2048),
                             h.raw.ppac.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ppac.m_XRpXL_CATHODE.size(); i++)
        {
            Initialize<TH2D>(h.raw.ppac.m_XRpXL_CATHODE.at(i),
                             new TH2D(Form("m_XRpXL_CATHODE_%i", i),
                                      Form("X_RIGHT + X_LEFT : CATHODE section %i", i), 1024, 0,
                                      4096, 1024, 0, 8192),
                             h.raw.ppac.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ppac.h_TOF_raw.size(); i++)
        {
            Initialize<TH1D>(
                h.raw.ppac.h_TOF_raw.at(i),
                new TH1D(Form("h_TOF_raw_%i", i), Form("TOF raw section %i", i), 1024, 0, 4096),
                h.raw.ppac.ptrs);
        }

        // IC
        h.raw.ic.dir = h.raw.dir->mkdir("IC", "Prisma IC raw histograms");
        h.raw.ic.dir->cd();
        // ic pads
        for (unsigned int i{0}; i < h.raw.ic.h_IC_A.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_IC_A.at(i),
                             new TH1D(Form("h_IC_A_%i", i),
                                      Form("Ionization Chamber row A section %i", i), 1024, 0,
                                      4096),
                             h.raw.ic.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_IC_B.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_IC_B.at(i),
                             new TH1D(Form("h_IC_B_%i", i),
                                      Form("Ionization Chamber row B section %i", i), 1024, 0,
                                      4096),
                             h.raw.ic.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_IC_C.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_IC_C.at(i),
                             new TH1D(Form("h_IC_C_%i", i),
                                      Form("Ionization Chamber row C section %i", i), 1024, 0,
                                      4096),
                             h.raw.ic.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_IC_D.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_IC_D.at(i),
                             new TH1D(Form("h_IC_D_%i", i),
                                      Form("Ionization Chamber row D section %i", i), 1024, 0,
                                      4096),
                             h.raw.ic.ptrs);
        }
        // side pads
        for (unsigned int i{0}; i < h.raw.ic.h_Side_A.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_Side_A.at(i),
                             new TH1D(Form("h_Side_A_%i", i),
                                      Form("Side detector row A section %i", i), 1024, 0, 4096),
                             h.raw.ic.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_Side_B.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_Side_B.at(i),
                             new TH1D(Form("h_Side_B_%i", i),
                                      Form("Side detector row B section %i", i), 1024, 0, 4096),
                             h.raw.ic.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_Side_C.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_Side_C.at(i),
                             new TH1D(Form("h_Side_C_%i", i),
                                      Form("Side detector row C section %i", i), 1024, 0, 4096),
                             h.raw.ic.ptrs);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_Side_D.size(); ++i)
        {
            Initialize<TH1D>(h.raw.ic.h_Side_D.at(i),
                             new TH1D(Form("h_Side_D_%i", i),
                                      Form("Side detector row D section %i", i), 1024, 0, 4096),
                             h.raw.ic.ptrs);
        }

        Initialize<TH2D>(h.raw.ic.m_ICNr_ICE,
                         new TH2D("m_ICNr_ICE_raw", "IC Pads Nr : IC Energy (Raw)", 1024, 0, 4096,
                                  40, -0.5, 39.5),
                         h.raw.ic.ptrs);
    }

    // Analyzed variables
    if (conf.prisma.ana)
    {
        h.ana.dir = h.dir->mkdir("Ana", "Prisma standalone analyzed histograms");
        h.ana.dir->cd();

        // 1-D histrograms
        Initialize<TH1D>(h.ana.h_ANr,
                         new TH1D("h_A_Nr", "A spectrum",
                                  conf.prisma.Arange.second - conf.prisma.Arange.first,
                                  conf.prisma.Arange.first, conf.prisma.Arange.second),
                         h.ana.ptrs);
        Initialize<TH1D>(h.ana.h_ZNr,
                         new TH1D("h_Z_Nr", "Z spectrum",
                                  conf.prisma.Zrange.second - conf.prisma.Zrange.first,
                                  conf.prisma.Zrange.first, conf.prisma.Zrange.second),
                         h.ana.ptrs);
        Initialize<TH1D>(h.ana.h_qNr,
                         new TH1D("h_q_Nr", "charge state", conf.prisma.Zrange.second, 0,
                                  conf.prisma.Zrange.second),
                         h.ana.ptrs);

        Initialize<TH2D>(h.ana.m_goodEvtsStat,
                         new TH2D("m_goodEvtsStat",
                                  "Statistics of Prisma events by _ok flags "
                                  "Flag;Goodness;Counts",
                                  11, -0.5, 10.5, 3, -1.5, 1.5),
                         h.ana.ptrs);
        if (h.ana.m_goodEvtsStat != nullptr)
        {
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(1, "mcp");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(2, "tof");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(3, "ppac");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(4, "side");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(5, "traj");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(6, "ic");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(7, "tof & mcp");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(8, "tof & ic");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(9, "ic & mcp & side");
            h.ana.m_goodEvtsStat->GetXaxis()->SetBinLabel(10, "all ok");
            h.ana.m_goodEvtsStat->GetYaxis()->SetBinLabel(1, "BAD");
            h.ana.m_goodEvtsStat->GetYaxis()->SetBinLabel(3, "GOOD");
        }

        Initialize<TH1D>(h.ana.h_Beta, new TH1D("h_Beta", "beta", 300, 0, 0.15), h.ana.ptrs);
        Initialize<TH1D>(h.ana.h_XFP,
                         new TH1D("h_XFP", "x focal plane", 1200, -0.100 / GetUnitsLength(),
                                  1.100 / GetUnitsLength()),
                         h.ana.ptrs);
        Initialize<TH2D>(h.ana.m_MCPX_MCPY,
                         new TH2D("m_MCPX_MCPY_0", "MCPY : MCPX", 1000, -0.100 / GetUnitsLength(),
                                  0.100 / GetUnitsLength(), 1000, -0.100 / GetUnitsLength(),
                                  0.100 / GetUnitsLength()),
                         h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_MCPPhi_MCPTheta,
            new TH2D("m_MCPTheta_MCPPhi", "MCP Phi : MCP Theta", 900, 10, 70, 1000, 120, 240),
            h.ana.ptrs);
        Initialize<TH2D>(h.ana.m_XFP_YFP,
                         new TH2D("m_XFP_YFP", "PPAC Y : X", 1200, -0.100 / GetUnitsLength(),
                                  1.100 / GetUnitsLength(), 600, -0.080 / GetUnitsLength(),
                                  0.080 / GetUnitsLength()),
                         h.ana.ptrs);
        Initialize<TH2D>(h.ana.m_TOF_XFP,
                         new TH2D("m_TOF_XFP_0", "tof : x focal plane", 1200,
                                  -0.100 / GetUnitsLength(), 1.100 / GetUnitsLength(), 5000, 0,
                                  500),
                         h.ana.ptrs);

        int Zmin = conf.prisma.Zrange.first, Zmax = conf.prisma.Zrange.second,
            Amin = conf.prisma.Arange.first, Amax = conf.prisma.Arange.second;
        Initialize<TH2D>(h.ana.m_ZNr_NNr,
                         new TH2D("m_Z_N", "Z : N", Zmax - Zmin + 1, Zmin - 0.5, Zmax + 0.5,
                                  (Amax - Zmin) - (Amin - Zmax) + 1, (Amin - Zmax) - 0.5,
                                  (Amax - Zmin) + 0.5),
                         h.ana.ptrs);
        Initialize<TH2D>(h.ana.m_Zed_Mass,
                         new TH2D("m_Zed_Mass", "Zed : Mass", 1000, Amin - 10, Amax + 10, 1000,
                                  Zmin - 5, Zmax + 5),
                         h.ana.ptrs);
        Initialize<TH2D>(h.ana.m_Zfloat_Mass,
                         new TH2D("m_Zfloat_Mass", "Zfloat : Mass", 1000, Amin - 10, Amax + 10,
                                  1000, Zmin - 5, Zmax + 5),
                         h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_Zfloat_Qfloat,
            new TH2D("m_Zfloat_Qfloat", "Zfloat : Charge", 1000, 15, 60, 1000, Zmin - 5, Zmax + 5),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_Qfloat_Mass,
            new TH2D("m_Qfloat_Mass", "Charge : Mass", 1000, Amin - 10, Amax + 10, 1000, 15, 60),
            h.ana.ptrs);

        Initialize<TH2D>(h.ana.m_ICNr_ICE,
                         new TH2D("m_ICNr_ICE_ana", "IC Pads Nr : IC Energy (Ana)", 1024, 0, 4096,
                                  40, -0.5, 39.5),
                         h.ana.ptrs);

        Initialize<TH2D>(h.ana.m_Monitors,
                         new TH2D("m_Monitors", "Monitors", 1000, 0, 5000, 2, 0.5, 2.5),
                         h.ana.ptrs);
        Initialize<TH2D>(h.ana.m_Brho_Theta,
                         new TH2D("m_Brho_Theta", "Brho vs Theta; #theta (deg); B#rho (Tm)", 1000,
                                  std::min(0., conf.prisma.angle / UNITS::deg - 20),
                                  conf.prisma.angle / UNITS::deg + 20, 1000, 0.7, 1.3),
                         h.ana.ptrs);

        h.ana.xfp.dir = h.ana.dir->mkdir("X_FP", "Prisma X_FP analyzed histograms");
        h.ana.xfp.dir->cd();
        for (unsigned int i = 0; i < h.ana.xfp.h_XFP_sections.size(); i++)
        {
            Initialize<TH1D>(h.ana.xfp.h_XFP_sections.at(i),
                             new TH1D(Form("h_XFP_section_%i", i), Form("h_XFP section %i", i), 400,
                                      (i * 0.1 - 0.050) / GetUnitsLength(),
                                      ((i + 1) * 0.1 + 0.05) / GetUnitsLength()),
                             h.ana.xfp.ptrs);
            Initialize<TH1D>(h.ana.xfp.h_XFP_RL_sections.at(i),
                             new TH1D(Form("h_XFP_RL_section_%i", i),
                                      Form("h_XFP_RL section %i", i), 400,
                                      (i * 0.1 - 0.050) / GetUnitsLength(),
                                      ((i + 1) * 0.1 + 0.05) / GetUnitsLength()),
                             h.ana.xfp.ptrs);
            Initialize<TH1D>(h.ana.xfp.h_XFP_RC_sections.at(i),
                             new TH1D(Form("h_XFP_RC_section_%i", i),
                                      Form("h_XFP_RC section %i", i), 400,
                                      (i * 0.1 - 0.050) / GetUnitsLength(),
                                      ((i + 1) * 0.1 + 0.05) / GetUnitsLength()),
                             h.ana.xfp.ptrs);
            Initialize<TH1D>(h.ana.xfp.h_XFP_CL_sections.at(i),
                             new TH1D(Form("h_XFP_CL_section_%i", i),
                                      Form("h_XFP_CL section %i", i), 400,
                                      (i * 0.1 - 0.050) / GetUnitsLength(),
                                      ((i + 1) * 0.1 + 0.05) / GetUnitsLength()),
                             h.ana.xfp.ptrs);
        }

        // deltaE:E plots/////////////////////////////////////
        h.ana.de_e.dir = h.ana.dir->mkdir("DE_E", "Prisma DE:E analyzed histograms");
        h.ana.de_e.dir->cd();

        double e_offset = -100;

        Initialize<TH2D>(h.ana.de_e.dea_e.m_ICDEA_ICE_tot,
                         new TH2D("m_ICDEA_ICE_tot_1", "Total DE_A vs E; E; DE_A", 2000, 0,
                                  conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.dea_e.ptrs);

        Initialize<TH2D>(h.ana.de_e.dea_e.m_ICDEA_ICE_corr_tot,
                         new TH2D("m_ICDEA_ICE_corr_tot", "Corrected DE_A vs E; E; DE_A", 2000,
                                  e_offset, conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.dea_e.ptrs);

        Initialize<TH2D>(h.ana.de_e.deab_e.m_ICDEAB_ICE_tot,
                         new TH2D("m_ICDEAB_ICE_tot_0", "Total DE_AB vs E; E; DE_AB", 2000, 0,
                                  conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.deab_e.ptrs);

        Initialize<TH2D>(h.ana.de_e.deab_e.m_ICDEAB_ICE_corr_tot,
                         new TH2D("m_ICDEAB_ICE_corr_tot", "Corrected DE_AB vs E; E; DE_AB", 2000,
                                  e_offset, conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.deab_e.ptrs);

        Initialize<TH2D>(h.ana.de_e.dea_eres.m_ICDEA_ICEres_tot,
                         new TH2D("m_ICDEA_ICEres_tot", "DE_A vs E_{res}; E_{res}; DE_A", 2000, 0,
                                  conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.dea_eres.ptrs);

        Initialize<TH2D>(h.ana.de_e.dea_eres.m_ICDEA_ICEres_corr_tot,
                         new TH2D("m_ICDEA_ICEres_corr_tot_3",
                                  "Corrected DE_A vs _{res}; E_{res}; DE_A", 2000, e_offset,
                                  conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.dea_eres.ptrs);

        Initialize<TH2D>(h.ana.de_e.deab_eres.m_ICDEAB_ICEres_tot,
                         new TH2D("m_ICDEAB_ICEres_tot", "DE_AB vs _{res}; E_{res}; DE_AB", 2000, 0,
                                  conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.deab_eres.ptrs);

        Initialize<TH2D>(h.ana.de_e.deab_eres.m_ICDEAB_ICEres_corr_tot,
                         new TH2D("m_ICDEAB_ICEres_corr_tot_6",
                                  "Corrected DE_AB vs _{res}; E_{res}; DE_AB", 2000, e_offset,
                                  conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                         h.ana.de_e.deab_eres.ptrs);

        // double halfRangeX =
        // (h.ana.de_e.dea_eres.m_ICDEA_ICEres_tot->GetXaxis()->GetXmax()-h.ana.de_e.dea_eres.m_ICDEA_ICEres_tot->GetXaxis()->GetXmin())
        // / 2.; double halfRangeY =
        // (h.ana.de_e.dea_eres.m_ICDEA_ICEres_tot->GetYaxis()->GetXmax()-h.ana.de_e.dea_eres.m_ICDEA_ICEres_tot->GetYaxis()->GetXmin())
        // / 2.;

        double                    ct          = TMath::Cos(conf.prisma.z_rot_angle);
        double                    st          = TMath::Sin(conf.prisma.z_rot_angle);
        std::pair<double, double> x_range_rot = {e_offset * ct - conf.prisma.maxICdEne * st,
                                                 conf.prisma.maxICEne * ct};
        std::pair<double, double> y_range_rot = {e_offset * st, conf.prisma.maxICdEne * ct +
                                                                    conf.prisma.maxICEne * st};

        Initialize<TH2D>(h.ana.de_e.m_ICen1_ICen2,
                         new TH2D("m_ICen1_ICen2", "Rotated IC matrix; IC En2; IC En1", 2000,
                                  x_range_rot.first, x_range_rot.second, 2000, y_range_rot.first,
                                  y_range_rot.second),
                         h.ana.de_e.ptrs);
        Initialize<TH2D>(h.ana.de_e.m_ICen1_Yfp,
                         new TH2D("m_ICen1_Yfp", "Rotated IC matrix; Y_{FP}; IC En1", 1000,
                                  -0.065 / GetUnitsLength(), 0.065 / GetUnitsLength(), 2000,
                                  y_range_rot.first, y_range_rot.second),
                         h.ana.de_e.ptrs);
        Initialize<TH2D>(h.ana.de_e.m_ICen1_corr1_ICen2,
                         new TH2D("m_ICen1_corr1_ICen2", "Rotated IC matrix; IC En2; IC En1 corr",
                                  2000, x_range_rot.first, x_range_rot.second, 2000,
                                  y_range_rot.first, y_range_rot.second),
                         h.ana.de_e.ptrs);
        Initialize<TH2D>(h.ana.de_e.m_ICen1_corr1_Xfp,
                         new TH2D("m_ICen1_corr1_Xfp", "Rotated IC matrix; X_{FP}; IC En1 corr",
                                  2000, 0, 1.0 / GetUnitsLength(), 2000, y_range_rot.first,
                                  y_range_rot.second),
                         h.ana.de_e.ptrs);
        Initialize<TH2D>(h.ana.de_e.m_ICen1_corr2_ICen2,
                         new TH2D("m_ICen1_corr2_ICen2", "Rotated IC matrix; IC En2; IC En1 corr2",
                                  2000, x_range_rot.first, x_range_rot.second, 2000,
                                  y_range_rot.first, y_range_rot.second),
                         h.ana.de_e.ptrs);

        Initialize<TH1D>(h.ana.de_e.h_Zed, new TH1D("h_Zed", "Zed", 1000, 10, 60), h.ana.de_e.ptrs);
        Initialize<TH1D>(h.ana.de_e.h_Zfloat,
                         new TH1D("h_Zfloat", "Zed corrected by mass", 1000, 10, 60),
                         h.ana.de_e.ptrs);
        Initialize<TH2D>(h.ana.de_e.m_Zfloat_ICEres_corr,
                         new TH2D("m_Zfloat_ICEres_corr", "Zed corrected by mass : ICEres_corr",
                                  2000, 0, conf.prisma.maxICEne, 1000, 10, 60),
                         h.ana.de_e.ptrs);

        h.ana.de_e.dea_e.dir =
            h.ana.de_e.dir->mkdir("DEA_E", "Prisma IC_DE_A:IC_E analyzed histograms");
        h.ana.de_e.dea_e.dir->cd();

        for (unsigned int i = {0}; i < h.ana.de_e.dea_e.m_ICDEA_ICE.size(); i++)
        {
            Initialize<TH2D>(h.ana.de_e.dea_e.m_ICDEA_ICE.at(i),
                             new TH2D(Form("m_ICDEA_ICE_%i", i), Form("DE_A : E (col %i)", i), 2000,
                                      0, conf.prisma.maxICEne, 2000, 0, conf.prisma.maxICdEne),
                             h.ana.de_e.dea_e.ptrs);
        }

        h.ana.de_e.deab_e.dir =
            h.ana.de_e.dir->mkdir("DEAB_E", "Prisma IC_DE_AB:IC_E analyzed histograms");
        h.ana.de_e.deab_e.dir->cd();

        for (unsigned int i = {0}; i < h.ana.de_e.deab_e.m_ICDEAB_ICE.size(); i++)
        {
            Initialize<TH2D>(h.ana.de_e.deab_e.m_ICDEAB_ICE.at(i),
                             new TH2D(Form("m_ICDEAB_ICE_%i", i), Form("DE_AB : E (col %i)", i),
                                      2000, 0, conf.prisma.maxICEne, 2000, 0,
                                      conf.prisma.maxICdEne),
                             h.ana.de_e.deab_e.ptrs);
        }

        // E:XFP plots///////////////////////////////////////
        h.ana.ice_xfp.dir = h.ana.dir->mkdir("ICE_XFP", "Prisma Energy:XFP analyzed histograms");
        h.ana.ice_xfp.dir->cd();
        Initialize<TH2D>(h.ana.ice_xfp.m_ICE_XFP_tot,
                         new TH2D("m_ICE_XFP_tot", "total IC E : XFP", 1000, 0,
                                  1. / GetUnitsLength(), 1000, 0, conf.prisma.maxICEne),
                         h.ana.ice_xfp.ptrs);

        for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++)
        {
            h.ana.ice_xfp.m_ICE_XFP.push_back(nullptr);
            Initialize<TH2D>(h.ana.ice_xfp.m_ICE_XFP.back(),
                             new TH2D(Form("m_ICE_XFP_2_%i", i), Form("IC E : XFP (Z = %i)", i),
                                      1000, 0, 1. / GetUnitsLength(), 1000, 0,
                                      conf.prisma.maxICEne),
                             h.ana.ice_xfp.ptrs);
        }

        // E:Rbeta plots///////////////////////////////////////
        h.ana.ice_rbeta.dir =
            h.ana.dir->mkdir("ICE_RBeta", "Prisma Energy:RBeta analyzed histograms");
        h.ana.ice_rbeta.dir->cd();
        Initialize<TH2D>(h.ana.ice_rbeta.m_ICE_RBeta_tot,
                         new TH2D("m_ICE_RBeta_tot_0", "total IC E : R*Beta", 4000,
                                  conf.prisma.minRBeta / GetUnitsLength(),
                                  conf.prisma.maxRBeta / GetUnitsLength(), 1000, 0,
                                  conf.prisma.maxICEne),
                         h.ana.ice_rbeta.ptrs);

        for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++)
        {
            h.ana.ice_rbeta.m_ICE_RBeta.push_back(nullptr);
            Initialize<TH2D>(h.ana.ice_rbeta.m_ICE_RBeta.back(),
                             new TH2D(Form("m_ICE_RBeta_0_%i", i),
                                      Form("IC E : R*Beta (Z = %i)", i), 4000,
                                      conf.prisma.minRBeta / GetUnitsLength(), conf.prisma.maxRBeta,
                                      1000, 0, conf.prisma.maxICEne),
                             h.ana.ice_rbeta.ptrs);
        }

        // E/RBeta:E plots///////////////////////////////////////
        Initialize<TH2D>(h.ana.iceoverrbeta_ice.m_ICEoverRBeta_ICE_tot,
                         new TH2D("m_ICEoverRBeta_ICE_tot_1", "total IC E/R*Beta : IC E", 1000, 0,
                                  conf.prisma.maxICEne, 4000, 0,
                                  conf.prisma.maxICEne / conf.prisma.minRBeta * GetUnitsLength()),
                         h.ana.iceoverrbeta_ice.ptrs);

        for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++)
        {
            h.ana.iceoverrbeta_ice.m_ICEoverRBeta_ICE.push_back(nullptr);
            Initialize<TH2D>(
                h.ana.iceoverrbeta_ice.m_ICEoverRBeta_ICE.back(),
                new TH2D(Form("m_ICEoverRBeta_ICE_1_%i", i),
                         Form("IC E / R*Beta : IC E (Z = %i)", i), 1000, 0, conf.prisma.maxICEne,
                         4000, 0, conf.prisma.maxICEne / conf.prisma.minRBeta * GetUnitsLength()),
                h.ana.iceoverrbeta_ice.ptrs);
        }

        h.ana.charge.dir = h.ana.dir->mkdir("Charge_AoQ", "Charge state : A/Q analyzed histograms");
        h.ana.charge.dir->cd();
        Initialize<TH2D>(h.ana.charge.m_Charge_cal_AoQ_tot,
                         new TH2D("m_Charge_cal_AoQ_tot", "Corrected charge state : A/Q", 1000, 2,
                                  5, 1000, 20, 50),
                         h.ana.charge.ptrs);
        Initialize<TH2D>(h.ana.charge.m_Charge_cal_AoQ_Z_tot,
                         new TH2D("m_Charge_cal_AoQ_Z_tot",
                                  "Corrected charge state with identified Z : A/Q", 1000, 2, 5,
                                  1000, 20, 50),
                         h.ana.charge.ptrs);
        Initialize<TH2D>(h.ana.charge.m_Charge_cal_beta_tot,
                         new TH2D("m_Charge_cal_beta_tot",
                                  "Calibrated charge state with identified Z : beta", 1000, 0.06,
                                  0.15, 1000, 20, 50),
                         h.ana.charge.ptrs);
        Initialize<TH2D>(h.ana.charge.m_Charge_cal_Xfp_tot,
                         new TH2D("m_Charge_cal_Xfp_tot",
                                  "Calibrated charge state with identified Z : X_FP", 1000, 0, 1,
                                  1000, 20, 50),
                         h.ana.charge.ptrs);
        Initialize<TH1D>(h.ana.charge.h_Charge_cal_Z_tot,
                         new TH1D("h_Charge_cal_Z_tot", "Calibrated charge state with identified Z",
                                  1000, 20, 50),
                         h.ana.charge.ptrs);
        Initialize<TH1D>(h.ana.charge.h_Qfloat,
                         new TH1D("h_Qfloat",
                                  "Calibrated and aligned charge state with identified Z", 1000, 15,
                                  60),
                         h.ana.charge.ptrs);
        Initialize<TH2D>(h.ana.charge.m_Qfloat_AoQ,
                         new TH2D("m_Qfloat_AoQ",
                                  "Calibrated and aligned charge state with identified Z : A/Q",
                                  1000, 2, 5, 1000, 15, 60),
                         h.ana.charge.ptrs);
        Initialize<TH2D>(h.ana.charge.m_Qfloat_beta,
                         new TH2D("m_Qfloat_beta",
                                  "Calibrated and aligned charge state with identified Z : beta",
                                  1000, 0.06, 0.17, 1000, 15, 60),
                         h.ana.charge.ptrs);

        // A/q plots////////////////////////////////////////////
        if (conf.prisma.Aoverq_hists)
        {
            h.ana.aoverq.dir = h.ana.dir->mkdir("Aoverq", "Prisma A/q analyzed histograms");
            h.ana.aoverq.dir->cd();

            Initialize<TH1D>(h.ana.aoverq.h_Aoverq_tot,
                             new TH1D("h_Aoverq_tot", "total A/q", conf.prisma.aq_nbins,
                                      conf.prisma.minAoverQ * GetUnitsAoQ(),
                                      conf.prisma.maxAoverQ * GetUnitsAoQ()),
                             h.ana.aoverq.ptrs);
            for (int i = 0; i < 10; ++i)
            {
                h.ana.aoverq.h_Aoverq_cols.push_back(nullptr);
                Initialize<TH1D>(h.ana.aoverq.h_Aoverq_cols.back(),
                                 new TH1D(Form("h_Aoverq_col%i", i), Form("A/q column %i", i),
                                          conf.prisma.aq_nbins,
                                          conf.prisma.minAoverQ * GetUnitsAoQ(),
                                          conf.prisma.maxAoverQ * GetUnitsAoQ()),
                                 h.ana.aoverq.ptrs);
            }

            Initialize<TH2D>(h.ana.aoverq.m_Aoverq_XFP_tot,
                             new TH2D("m_Aoverq_XFP_tot", "total A/q : x focal plane", 1200,
                                      -0.1 / GetUnitsLength(), 1.1 / GetUnitsLength(),
                                      conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                                      conf.prisma.maxAoverQ * GetUnitsAoQ()),
                             h.ana.aoverq.ptrs);
            Initialize<TH2D>(h.ana.aoverq.m_Aoverq_Xmcp_tot,
                             new TH2D("m_Aoverq_Xmcp_tot", "total A/q : x mcp", 1000,
                                      -0.1 / GetUnitsLength(), 0.1 / GetUnitsLength(),
                                      conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                                      conf.prisma.maxAoverQ * GetUnitsAoQ()),
                             h.ana.aoverq.ptrs);
            Initialize<TH2D>(h.ana.aoverq.m_Aoverq_Ymcp_tot,
                             new TH2D("m_Aoverq_Ymcp_tot", "total A/q : y mcp", 1000,
                                      -0.1 / GetUnitsLength(), 0.1 / GetUnitsLength(),
                                      conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                                      conf.prisma.maxAoverQ * GetUnitsAoQ()),
                             h.ana.aoverq.ptrs);
            // Gated in Z
            h.ana.aoverq.zGated.dir = h.ana.aoverq.dir->mkdir(
                "Zed_Gated", "Prisma A/q analyzed histograms and matrices gated in Z");
            h.ana.aoverq.zGated.dir->cd();
            // 1D plots
            h.ana.aoverq.zGated.histos.dir =
                h.ana.aoverq.zGated.dir->mkdir("1D", "Prisma A/q analyzed histograms gated in Z");
            h.ana.aoverq.zGated.histos.dir->cd();
            for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; ++i)
            {
                h.ana.aoverq.zGated.histos.h_Aoverq.push_back(nullptr);
                Initialize<TH1D>(h.ana.aoverq.zGated.histos.h_Aoverq.back(),
                                 new TH1D(Form("h_Aoverq_%i", i), Form("A/q (Z = %i)", i),
                                          conf.prisma.aq_nbins,
                                          conf.prisma.minAoverQ * GetUnitsAoQ(),
                                          conf.prisma.maxAoverQ * GetUnitsAoQ()),
                                 h.ana.aoverq.zqGated.histos.ptrs);
            }
            // 2D plots
            h.ana.aoverq.zGated.matricesXfp.dir = h.ana.aoverq.zGated.dir->mkdir(
                "2D_Xfp", "Prisma A/q:x_fp analyzed histograms gated in Z");
            h.ana.aoverq.zGated.matricesXfp.dir->cd();
            for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; ++i)
            {
                h.ana.aoverq.zGated.matricesXfp.m_Aoverq.push_back(nullptr);
                Initialize<TH2D>(
                    h.ana.aoverq.zGated.matricesXfp.m_Aoverq.back(),
                    new TH2D(Form("m_Aoverq_Xfp_%i", i), Form("A/q : x focal plane (Z = %i)", i),
                             1200, -0.1 / GetUnitsLength(), 1.1 / GetUnitsLength(),
                             conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                             conf.prisma.maxAoverQ * GetUnitsAoQ()),
                    h.ana.aoverq.zGated.matricesXfp.ptrs);
            }
            h.ana.aoverq.zGated.matricesYfp.dir = h.ana.aoverq.zGated.dir->mkdir(
                "2D_Yfp", "Prisma A/q:y_fp analyzed histograms gated in Z");
            h.ana.aoverq.zGated.matricesYfp.dir->cd();
            for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++)
            {
                h.ana.aoverq.zGated.matricesYfp.m_Aoverq.push_back(nullptr);
                Initialize<TH2D>(
                    h.ana.aoverq.zGated.matricesYfp.m_Aoverq.back(),
                    new TH2D(Form("m_Aoverq_Yfp_%i", i), Form("A/q : y focal plane (Z = %i)", i),
                             1200, -0.1 / GetUnitsLength(), 0.1 / GetUnitsLength(),
                             conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                             conf.prisma.maxAoverQ * GetUnitsAoQ()),
                    h.ana.aoverq.zGated.matricesYfp.ptrs);
            }
            h.ana.aoverq.zGated.matricesXmcp.dir = h.ana.aoverq.zGated.dir->mkdir(
                "2D_Xmcp", "Prisma A/q:x_mcp analyzed histograms gated in Z");
            h.ana.aoverq.zGated.matricesXmcp.dir->cd();
            for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++)
            {
                h.ana.aoverq.zGated.matricesXmcp.m_Aoverq.push_back(nullptr);
                Initialize<TH2D>(
                    h.ana.aoverq.zGated.matricesXmcp.m_Aoverq.back(),
                    new TH2D(Form("m_Aoverq_Xmcp_%i", i), Form("A/q : x mcp (Z = %i)", i), 1200,
                             -0.060 / GetUnitsLength(), 0.060 / GetUnitsLength(),
                             conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                             conf.prisma.maxAoverQ * GetUnitsAoQ()),
                    h.ana.aoverq.zGated.matricesXmcp.ptrs);
            }
            h.ana.aoverq.zGated.matricesYmcp.dir = h.ana.aoverq.zGated.dir->mkdir(
                "2D_Ymcp", "Prisma A/q:y_mcp analyzed histograms gated in Z");
            h.ana.aoverq.zGated.matricesYmcp.dir->cd();
            for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; ++i)
            {
                h.ana.aoverq.zGated.matricesYmcp.m_Aoverq.push_back(nullptr);
                Initialize<TH2D>(
                    h.ana.aoverq.zGated.matricesYmcp.m_Aoverq.back(),
                    new TH2D(Form("m_Aoverq_Ymcp_%i", i), Form("A/q : y mcp (Z = %i)", i), 1200,
                             -0.060 / GetUnitsLength(), 0.060 / GetUnitsLength(),
                             conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                             conf.prisma.maxAoverQ * GetUnitsAoQ()),
                    h.ana.aoverq.zGated.matricesYmcp.ptrs);
            }
            if (conf.prisma.Aoverq_tevo_hists)
            {
                for (int j = 0; j < 10; ++j)
                {
                    h.ana.aoverq.zGated.matricesTime.at(j).dir = h.ana.aoverq.zGated.dir->mkdir(
                        Form("Time_evo_sec_%i", j),
                        Form("Prisma A/q vs Time in min per Z in section "
                             "%i",
                             j));
                    h.ana.aoverq.zGated.matricesTime.at(j).dir->cd();
                    for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; ++i)
                    {
                        h.ana.aoverq.zGated.matricesTime.at(j).m_Aoverq.push_back(nullptr);
                        Initialize<TH2D>(h.ana.aoverq.zGated.matricesTime.at(j).m_Aoverq.back(),
                                         new TH2D(Form("m_Aoverq_Time_%i", i),
                                                  Form("A/q : Time [min] (Z = %i)", i), 24 * 60, 0,
                                                  24 * 60, conf.prisma.aq_nbins,
                                                  conf.prisma.minAoverQ * GetUnitsAoQ(),
                                                  conf.prisma.maxAoverQ * GetUnitsAoQ()),
                                         h.ana.aoverq.zGated.matricesTime.at(j).ptrs);
                    }
                }
            }
            // Gated in Z and q//////////////////////////////////////
            h.ana.aoverq.zqGated.dir =
                h.ana.aoverq.dir->mkdir("ZQ_Gated", "Prisma A/q analyzed histograms and matrices "
                                                    "gated in Z and q");
            h.ana.aoverq.zqGated.dir->cd();
            // 1D plots
            h.ana.aoverq.zqGated.histos.dir = h.ana.aoverq.zqGated.dir->mkdir(
                "1D", "Prisma A/q analyzed histograms gated in Z and q");
            h.ana.aoverq.zqGated.histos.dir->cd();
            for (const auto &aq : conf.prisma.aqConfs)
            {
                std::vector<TH1D *> hist_vec;
                for (int i = aq.qmin; i <= aq.qmax; i++)
                {
                    hist_vec.push_back(nullptr);
                    Initialize<TH1D>(hist_vec.back(),
                                     new TH1D(Form("h_Aoverq_%i_%i", aq.Z, i),
                                              Form("A/q (Z = %i, q = %i)", aq.Z, i),
                                              conf.prisma.aq_nbins,
                                              conf.prisma.minAoverQ * GetUnitsAoQ(),
                                              conf.prisma.maxAoverQ * GetUnitsAoQ()),
                                     h.ana.aoverq.zqGated.histos.ptrs);
                }
                h.ana.aoverq.zqGated.histos.h_Aoverq.push_back(hist_vec);
            }
            // 2D plots
            h.ana.aoverq.zqGated.matricesXfp.dir = h.ana.aoverq.zqGated.dir->mkdir(
                "2D_Xfp", "Prisma A/q analyzed histograms gated in Z and q");
            h.ana.aoverq.zqGated.matricesXfp.dir->cd();
            for (const auto &aq : conf.prisma.aqConfs)
            {
                std::vector<TH2D *> matr_vec;
                for (int i = aq.qmin; i <= aq.qmax; i++)
                {
                    matr_vec.push_back(nullptr);
                    Initialize<TH2D>(matr_vec.back(),
                                     new TH2D(Form("m_Aoverq_Xfp_%i_%i", aq.Z, i),
                                              Form("A/q : x focal plane (Z = %i, q = %i)", aq.Z, i),
                                              1200, -0.100 / GetUnitsLength(),
                                              1.100 / GetUnitsLength(), conf.prisma.aq_nbins,
                                              conf.prisma.minAoverQ * GetUnitsAoQ(),
                                              conf.prisma.maxAoverQ * GetUnitsAoQ()),
                                     h.ana.aoverq.zqGated.matricesXfp.ptrs);
                }
                h.ana.aoverq.zqGated.matricesXfp.m_Aoverq.push_back(matr_vec);
            }
            h.ana.aoverq.zqGated.matricesYfp.dir = h.ana.aoverq.zqGated.dir->mkdir(
                "2D_Yfp", "Prisma A/q analyzed histograms gated in Z and q");
            h.ana.aoverq.zqGated.matricesYfp.dir->cd();
            for (const auto &aq : conf.prisma.aqConfs)
            {
                std::vector<TH2D *> matr_vec;
                for (int i = aq.qmin; i <= aq.qmax; i++)
                {
                    matr_vec.push_back(nullptr);
                    Initialize<TH2D>(matr_vec.back(),
                                     new TH2D(Form("m_Aoverq_Yfp_%i_%i", aq.Z, i),
                                              Form("A/q : y focal plane (Z = %i, q = %i)", aq.Z, i),
                                              1200, -0.15 / GetUnitsLength(),
                                              0.15 / GetUnitsLength(), conf.prisma.aq_nbins,
                                              conf.prisma.minAoverQ * GetUnitsAoQ(),
                                              conf.prisma.maxAoverQ * GetUnitsAoQ()),
                                     h.ana.aoverq.zqGated.matricesYfp.ptrs);
                }
                h.ana.aoverq.zqGated.matricesYfp.m_Aoverq.push_back(matr_vec);
            }
            h.ana.aoverq.zqGated.matricesXmcp.dir = h.ana.aoverq.zqGated.dir->mkdir(
                "2D_Xmcp", "Prisma A/q analyzed histograms gated in Z and q");
            h.ana.aoverq.zqGated.matricesXmcp.dir->cd();
            for (const auto &aq : conf.prisma.aqConfs)
            {
                std::vector<TH2D *> matr_vec;
                for (int i = aq.qmin; i <= aq.qmax; i++)
                {
                    matr_vec.push_back(nullptr);
                    Initialize<TH2D>(matr_vec.back(),
                                     new TH2D(Form("m_Aoverq_Xmcp_%i_%i", aq.Z, i),
                                              Form("A/q : x mcp (Z = %i, q = %i)", aq.Z, i), 1200,
                                              -0.06 / GetUnitsLength(), 0.06 / GetUnitsLength(),
                                              conf.prisma.aq_nbins,
                                              conf.prisma.minAoverQ * GetUnitsAoQ(),
                                              conf.prisma.maxAoverQ * GetUnitsAoQ()),
                                     h.ana.aoverq.zqGated.matricesXmcp.ptrs);
                }
                h.ana.aoverq.zqGated.matricesXmcp.m_Aoverq.push_back(matr_vec);
            }
            h.ana.aoverq.zqGated.matricesYmcp.dir = h.ana.aoverq.zqGated.dir->mkdir(
                "2D_Ymcp", "Prisma A/q analyzed histograms gated in Z and q");
            h.ana.aoverq.zqGated.matricesYmcp.dir->cd();
            for (const auto &aq : conf.prisma.aqConfs)
            {
                std::vector<TH2D *> matr_vec;
                for (int i = aq.qmin; i <= aq.qmax; i++)
                {
                    matr_vec.push_back(nullptr);
                    Initialize<TH2D>(
                        matr_vec.back(),
                        new TH2D(Form("m_Aoverq_Ymcp_%i_%i", aq.Z, i),
                                 Form("A/q : y mcp (Z = %i, q = %i)", aq.Z, i), 1200,
                                 -0.06 / GetUnitsLength(), 0.06 / GetUnitsLength(),
                                 conf.prisma.aq_nbins, conf.prisma.minAoverQ * GetUnitsAoQ(),
                                 conf.prisma.maxAoverQ * GetUnitsAoQ() * GetUnitsAoQ()),
                        h.ana.aoverq.zqGated.matricesYmcp.ptrs);
                }
                h.ana.aoverq.zqGated.matricesYmcp.m_Aoverq.push_back(matr_vec);
            }
        }
        // Mass plots/////////////////////////////////////////////////
        h.ana.mass.dir = h.ana.dir->mkdir("Mass", "Prisma mass analyzed histograms");
        h.ana.mass.dir->cd();

        Initialize<TH1D>(h.ana.mass.h_Mass_tot,
                         new TH1D("m_Mass_tot", "total Mass", 2000, conf.prisma.Arange.first,
                                  conf.prisma.Arange.second),
                         h.ana.mass.ptrs);
        Initialize<TH2D>(h.ana.mass.m_Mass_XFP_tot,
                         new TH2D("m_Mass_XFP_tot", "total Mass : x focal plane", 1200, -100, 1100,
                                  2000, conf.prisma.Arange.first, conf.prisma.Arange.second),
                         h.ana.mass.ptrs);

        // Gated in Z//////////////////////////////////////////////////
        h.ana.mass.zGated.dir = h.ana.mass.dir->mkdir(
            "Zed_Gated", "Prisma Mass analyzed histograms and matrices gated in Z");
        h.ana.mass.zGated.dir->cd();
        // 1D plots
        h.ana.mass.zGated.histos.dir =
            h.ana.mass.zGated.dir->mkdir("1D", "Prisma Mass analyzed histograms gated in Z");
        h.ana.mass.zGated.histos.dir->cd();
        for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++)
        {
            h.ana.mass.zGated.histos.h_Mass.push_back(nullptr);
            Initialize<TH1D>(h.ana.mass.zGated.histos.h_Mass.back(),
                             new TH1D(Form("h_Mass_%i", i), Form("Mass (Z = %i)", i), 2000,
                                      conf.prisma.Arange.first, conf.prisma.Arange.second),
                             h.ana.mass.zGated.histos.ptrs);
        }
        // 2D plots
        h.ana.mass.zGated.matrices.dir =
            h.ana.mass.zGated.dir->mkdir("2D", "Prisma Mass:x_fp analyzed histograms gated in Z");
        h.ana.mass.zGated.matrices.dir->cd();
        for (int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second; i++)
        {
            h.ana.mass.zGated.matrices.m_Mass_XFP.push_back(nullptr);
            Initialize<TH2D>(h.ana.mass.zGated.matrices.m_Mass_XFP.back(),
                             new TH2D(Form("m_Mass_XFP_%i", i),
                                      Form("Mass : x focal plane (Z = %i)", i), 1200, -100, 1100,
                                      2000, conf.prisma.Arange.first, conf.prisma.Arange.second),
                             h.ana.mass.zGated.matrices.ptrs);
        }

        // Gated in Z and q//////////////////////////////////////////
        h.ana.mass.zqGated.dir =
            h.ana.mass.dir->mkdir("ZQ_Gated", "Prisma Mass analyzed histograms and matrices "
                                              "gated in Z and q");
        h.ana.mass.zqGated.dir->cd();
        // 1D plots
        h.ana.mass.zqGated.histos.dir =
            h.ana.mass.zqGated.dir->mkdir("1D", "Prisma Mass analyzed histograms gated in Z and q");
        h.ana.mass.zqGated.histos.dir->cd();
        for (const auto &aq : conf.prisma.aqConfs)
        {
            std::vector<TH1D *> hist_vec;
            for (int i = aq.qmin; i <= aq.qmax; i++)
            {
                hist_vec.push_back(nullptr);
                Initialize<TH1D>(hist_vec.back(),
                                 new TH1D(Form("h_Mass_%i_%i", aq.Z, i),
                                          Form("Mass (Z = %i, q = %i)", aq.Z, i), 1000,
                                          conf.prisma.Arange.first, conf.prisma.Arange.second),
                                 h.ana.mass.zqGated.histos.ptrs);
            }
            h.ana.mass.zqGated.histos.h_Mass.push_back(hist_vec);
        }
        // 2D plots
        h.ana.mass.zqGated.matrices.dir =
            h.ana.mass.zqGated.dir->mkdir("2D", "Prisma Mass analyzed histograms gated in Z and q");
        h.ana.mass.zqGated.matrices.dir->cd();
        for (const auto &aq : conf.prisma.aqConfs)
        {
            std::vector<TH2D *> matr_vec;
            for (int i = aq.qmin; i <= aq.qmax; i++)
            {
                matr_vec.push_back(nullptr);
                Initialize<TH2D>(matr_vec.back(),
                                 new TH2D(Form("m_Mass_XFP_%i_%i", aq.Z, i),
                                          Form("Mass : x focal plane (Z = %i, q = %i)", aq.Z, i),
                                          1200, -100, 1100, 1000, conf.prisma.Arange.first,
                                          conf.prisma.Arange.second),
                                 h.ana.mass.zqGated.matrices.ptrs);
            }
            h.ana.mass.zqGated.matrices.m_Mass_XFP.push_back(matr_vec);
        }

        // Q value plots////////////////////////////////////////////
        h.ana.qvalue.dir = h.ana.dir->mkdir("Qvalue", "Prisma Q value analyzed histograms");
        Initialize<TH1D>(
            h.ana.qvalue.h_Qvalue,
            new TH1D("h_Qvalue", "Q value", 400, conf.prisma.minExEne, conf.prisma.maxExEne),
            h.ana.ptrs);
        // 2-D histrograms
        Initialize<TH2D>(h.ana.qvalue.m_Qvalue_TKEL,
                         new TH2D("m_Qvalue_TKEL",
                                  "Q value vs TKEL;Q-val prisma[MeV];TKEL sel[MeV]", 400, -100, 100,
                                  400, conf.prisma.minExEne, conf.prisma.maxExEne),
                         h.ana.ptrs);
        for (auto &ion : conf.reaction.fragmentsOfInterest)
        {
            PrismaHistograms::PrismaAnaHistograms::Qvalue::IonHistograms ion_histos;
            ion_histos.dir =
                h.ana.qvalue.dir->mkdir(Form("Ion_Cuts_Z%i_A%i", ion.first.Z, ion.first.A),
                                        Form("Gamma spectra with cuts on the identified ion: Z=%d, "
                                             "A=%d",
                                             ion.first.Z, ion.first.A));
            ion_histos.dir->cd();

            Initialize<TH1D>(ion_histos.h_Qvalue,
                             new TH1D(Form("h_Qvalue_%i_%i", ion.first.A, ion.first.Z),
                                      Form("Q value spectrum for A,%i and Z,%i;Total "
                                           "Excitation Energy "
                                           "[MeV];Counts / ",
                                           ion.first.A, ion.first.Z),
                                      300, conf.prisma.minExEne, conf.prisma.maxExEne),
                             ion_histos.ptrs);

            h.ana.qvalue.ionHistos[ion.first] = ion_histos;
        }

        // Time Evolution
        h.ana.timeEvo.dir =
            h.ana.dir->mkdir("Time_Evolution", "Prisma Time Evolution analyzed histograms");
        h.ana.timeEvo.dir->cd();
        Initialize<TH2D>(h.ana.timeEvo.m_TOF_time,
                         new TH2D("m_TOF_time", "TOF vs Time", 75 * 60, 0, 75 * 60, 500, 100, 400),
                         h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_Aoverq_time,
            new TH2D("m_Aoverq_time", "A/q vs Time", 75 * 60, 0, 75 * 60, 2000, 2.5, 4.5),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(h.ana.timeEvo.m_Aoverq_z38_time,
                         new TH2D("m_Aoverq_z38_time", "A/q vs Time (Z=38)", 75 * 60, 0, 75 * 60,
                                  2000, 2.5, 4.5),
                         h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_MassIC_time,
            new TH2D("m_MassIC_time", "Mass from IC vs Time", 75 * 60, 0, 75 * 60, 500, 50, 150),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_MassTOF_time,
            new TH2D("m_MassTOF_time", "Mass from TOF vs Time", 75 * 60, 0, 75 * 60, 1000, 50, 150),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_Charge_time,
            new TH2D("m_Charge_time", "Charge vs Time", 75 * 60, 0, 75 * 60, 500, 20, 50),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(h.ana.timeEvo.m_Xfp_time,
                         new TH2D("m_Xfp_time", "Xfp vs Time", 75 * 60, 0, 75 * 60, 1000, 0, 1),
                         h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_Yfp_time,
            new TH2D("m_Yfp_time", "Yfp vs Time", 75 * 60, 0, 75 * 60, 500, -0.07, 0.07),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_Xmcp_time,
            new TH2D("m_Xmcp_time", "Xmcp vs Time", 75 * 60, 0, 75 * 60, 500, -0.05, 0.05),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(h.ana.timeEvo.m_Xmcp_zoom_time,
                         new TH2D("m_Xmcp_zoom_time", "Xmcp zoom vs Time", 75 * 60, 0, 75 * 60, 500,
                                  -0.04, 0.02),
                         h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_Ymcp_time,
            new TH2D("m_Ymcp_time", "Ymcp vs Time", 75 * 60, 0, 75 * 60, 500, -0.05, 0.05),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_ICdEA_time,
            new TH2D("m_ICdEA_time", "ICdeA vs Time", 75 * 60, 0, 75 * 60, 500, 0, 600),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_ICen1corr2_time,
            new TH2D("m_ICen1corr2_time", "ICen1corr2 vs Time", 75 * 60, 0, 75 * 60, 500, 0, 4000),
            h.ana.timeEvo.ptrs);
        Initialize<TH2D>(
            h.ana.timeEvo.m_ICEres_time,
            new TH2D("m_ICEres_time", "ICEres vs Time", 75 * 60, 0, 75 * 60, 500, 0, 600),
            h.ana.timeEvo.ptrs);

        // Fission CM reconstruction
        if (conf.prisma.enable_cm_reconstruction)
        {
            h.ana.cmReco.dir =
                h.ana.dir->mkdir("CMReconstruction", "CM reconstruction histograms for fission");
            h.ana.cmReco.dir->cd();
            Initialize<TH1D>(h.ana.cmReco.h_betaCM,
                             new TH1D("h_betaCM", "Beta ion in the CM system", 1000, 0, 0.2),
                             h.ana.cmReco.ptrs);
            Initialize<TH1D>(
                h.ana.cmReco.h_betaCM_98Zr,
                new TH1D("h_betaCM_98Zr", "Beta ion in the CM system for 98Zr", 1000, 0, 0.2),
                h.ana.cmReco.ptrs);
            Initialize<TH1D>(h.ana.cmReco.h_thetaCM,
                             new TH1D("h_thetaCM", "Theta ion in the CM system", 1000, 0, 180),
                             h.ana.cmReco.ptrs);
            Initialize<TH1D>(h.ana.cmReco.h_phiCM,
                             new TH1D("h_phiCM", "Phi ion in the CM system", 1000, 0, 360),
                             h.ana.cmReco.ptrs);
            Initialize<TH2D>(h.ana.cmReco.m_betaCM_thetaCM,
                             new TH2D("m_thetaCMvsBetaCM", "Theta ion vs Beta ion in the CM system",
                                      1000, 0, 180, 1000, 0, 0.2),
                             h.ana.cmReco.ptrs);
            Initialize<TH2D>(h.ana.cmReco.m_phiCM_thetaCM,
                             new TH2D("h_thetaCMvsPhiCM", "Theta ion vs Phi ion in the CM system",
                                      1000, 0, 180, 1000, 0, 360),
                             h.ana.cmReco.ptrs);
            Initialize<TH2D>(h.ana.cmReco.m_betaCM_cosThetaCM_sinThetaCM,
                             new TH2D("m_betaCM_cosThetaCM_sinThetaCM",
                                      "#beta_{CM} sin(#theta_{CM}) vs #beta_{CM} cos(#theta_{CM})",
                                      1000, -0.1, 0.1, 1000, 0, 0.1),
                             h.ana.cmReco.ptrs);
            for (auto &ion : conf.reaction.fragmentsOfInterest)
            {
                PrismaHistograms::PrismaAnaHistograms::CMreconstruction::RecoIonHistograms
                    ion_histos;
                ion_histos.dir = h.ana.cmReco.dir->mkdir(Form("Z%i_A%i", ion.first.Z, ion.first.A),
                                                         Form("Reconstruction for ion: Z=%d, "
                                                              "A=%d",
                                                              ion.first.Z, ion.first.A));
                ion_histos.dir->cd();

                Initialize<TH1D>(ion_histos.h_betaCM_ion,
                                 new TH1D(Form("h_betaCM_ion_%i_%i", ion.first.A, ion.first.Z),
                                          Form("#beta in the CM A=%i and Z=%i;#beta_{CM};Counts",
                                               ion.first.A, ion.first.Z),
                                          1000, 0.0, 0.15),
                                 ion_histos.ptrs);
                Initialize<TH2D>(
                    ion_histos.m_betaCM_thetaCM_ion,
                    new TH2D(
                        Form("m_betaCM_thetaCM_ion_%i_%i", ion.first.A, ion.first.Z),
                        Form("#beta_{CM} vs #theta_{CM} for A=%i and Z=%i;#theta_{CM};#beta_{CM}",
                             ion.first.A, ion.first.Z),
                        1000, 0.0, 180.0, 1000, 0.0, 0.1),
                    ion_histos.ptrs);
                Initialize<TH2D>(
                    ion_histos.m_BrhoMid_thetaCM_ion,
                    new TH2D(Form("m_BrhoMid_thetaCM_ion_%i_%i", ion.first.A, ion.first.Z),
                             Form("BrhoMid vs #theta_{CM} for A=%i and Z=%i;#theta_{CM};BrhoMid",
                                  ion.first.A, ion.first.Z),
                             1000, 0.0, 180.0, 1000, 0.7, 1.3),
                    ion_histos.ptrs);
                Initialize<TH2D>(
                    ion_histos.m_betaCM_cosThetaCM_sinThetaCM_ion,
                    new TH2D(
                        Form("m_betaCM_cosThetaCM_sinThetaCM_ion_%i_%i", ion.first.A, ion.first.Z),
                        Form("#beta_{CM} sin(#theta_{CM}) vs #beta_{CM} cos(#theta_{CM}) for A=%i "
                             "and Z=%i; #beta_{CM} cos(#theta_{CM});#beta_{CM} sin(#theta_{CM})",
                             ion.first.A, ion.first.Z),
                        1000, -0.1, 0.1, 1000, -0.1, 0.1),
                    ion_histos.ptrs);
                h.ana.cmReco.recoIonHistos[ion.first] = ion_histos;
            }
        }
    }

    // Misc variables
    if (conf.prisma.misc)
    {
        h.misc.dir = h.dir->mkdir("Misc", "Prisma miscellaneous histograms");
        h.misc.dir->cd();

        for (unsigned int i{0}; i < h.misc.m_XFP_CATHODEmXL.size(); i++)
        {
            Initialize<TH2D>(h.misc.m_XFP_CATHODEmXL.at(i),
                             new TH2D(Form("m_XFP_CATHODEmXL_%i", i),
                                      Form("X_FP : CATHODE - X_LEFT section %i", i), 1024, -4096,
                                      4096, 1200, -100, 1100),
                             h.misc.ptrs);
        }
        for (unsigned int i{0}; i < h.misc.m_XFP_XRmCATHODE.size(); i++)
        {
            Initialize<TH2D>(h.misc.m_XFP_XRmCATHODE.at(i),
                             new TH2D(Form("m_XFP_XRmCATHODE_%i", i),
                                      Form("X_FP : X_RIGHT - CATHODE section %i", i), 1024, -4096,
                                      4096, 1200, -100, 1100),
                             h.misc.ptrs);
        }
        for (unsigned int i{0}; i < h.misc.m_TOFraw_XFP.size(); i++)
        {
            Initialize<TH2D>(h.misc.m_TOFraw_XFP.at(i),
                             new TH2D(Form("m_TOFraw_XFP_%i", i),
                                      Form("raw TOF : x focal plane section %i", i), 1200, -100,
                                      1100, 1024, 0, 4096),
                             h.misc.ptrs);
        }
    }

    EntryNr                  = 0;
    conf.prisma.doneTofAlign = false;

    return true;
}

void PrismaAnalysis::Analyze()
{
    PrismaInputAnalysis::Analyze();
    if (conf.prisma.analysis_stages != PrismaConf::AnalysisStage::ALL &&
        conf.prisma.analysis_stages != PrismaConf::AnalysisStage::ANA_HIGH &&
        conf.prisma.analysis_stages != PrismaConf::AnalysisStage::HIGHLEVEL)
        return;

    if (EntryNr++ > conf.prisma.nrRef && conf.prisma.doneTofAlign == false &&
        conf.prisma.enable_reftreeFill)
    {
        PrismaInputAnalysis::CalculateTofOffset();
        conf.prisma.doneTofAlign = true;
    }

    prisma.resize(1);
    prisma.TS = **prismaReader.TSPrisma;
    if (prismaReader.TAC_LT_VTS != nullptr)
    {
        if (**prismaReader.TAC_LT_VTS > 0)
        {
            prisma.Time.at(0) =
                ((**prismaReader.TAC_LT_VTS) * conf.prisma.tacGain + conf.prisma.tacOffset) *
                conf.prisma.cfd_unit;
        }
    }
    else if (prismaReader.TAC_LT_LTS != nullptr)
    {
        if (**prismaReader.TAC_LT_LTS > 0)
        {
            prisma.Time.at(0) =
                ((**prismaReader.TAC_LT_LTS) * conf.prisma.tacGain + conf.prisma.tacOffset) *
                conf.prisma.cfd_unit;
        }
    }
    else
    {
        prisma.Time.at(0) = 0;
    }

    prisma.A = prismaInput.a_nbr;
    prisma.Z = prismaInput.z_nbr;
    prisma.Q = prismaInput.q_nbr;
    switch (conf.prisma.de_type)
    {
    case 0: {
        prisma.dE = prismaInput.ic_de_ab_traj;
        prisma.E  = prismaInput.ic_e;
        break;
    }
    case 1: {
        prisma.dE = prismaInput.ic_de_a_traj;
        prisma.E  = prismaInput.ic_e;
        break;
    }
    case 2: {
        prisma.dE = prismaInput.ic_range;
        prisma.E  = prismaInput.ic_e;
        break;
    }
    case 3: {
        prisma.dE = prismaInput.ic_de_a_corr;
        prisma.E  = prismaInput.ic_e_res_a_corr;
        break;
    }
    case 4: {
        prisma.dE = prismaInput.ic_de_a_corr;
        prisma.E  = prismaInput.ic_e_res_a_corr;
        break;
    }
    case 5: {
        prisma.dE = prismaInput.ic_de_a_corr;
        prisma.E  = prismaInput.ic_e_res_a_corr;
        break;
    }
    case 6: {
        prisma.dE = prismaInput.ic_de_ab_corr;
        prisma.E  = prismaInput.ic_e_res_ab_corr;
        break;
    }
    default:
        throw std::runtime_error("DE type not correctly read");
    }

    prisma.E    = prismaInput.ic_e;
    prisma.mon1 = prismaInput.monitor_0;
    prisma.mon2 = prismaInput.monitor_1;

    prisma.colNr = prismaInput.col_nbr;
    prisma.X_FP  = prismaInput.x_fp;
    prisma.MCP_X = prismaInput.mcp_x;
    prisma.MCP_Y = prismaInput.mcp_y;

    prisma.mcp_ok  = prismaInput.mcp_ok;
    prisma.tof_ok  = prismaInput.tof_ok;
    prisma.side_ok = prismaInput.side_ok;
    prisma.ic_ok   = prismaInput.ic_ok;
    prisma.traj_ok = prismaInput.traj_ok;

    prisma.AoverQ = prismaInput.a_over_q;
    prisma.Mass   = prismaInput.mass;

    // Eliminate events if any signal is detected in the column of a broken
    // segment
    for (const auto &it : conf.prisma.broken_IC_channels)
    {
        if (it.second == 1 && (prismaInput.ic_pads[(it.first / 4) * 4 + 0] > 0 ||
                               prismaInput.ic_pads[(it.first / 4) * 4 + 1] > 0 ||
                               prismaInput.ic_pads[(it.first / 4) * 4 + 2] > 0 ||
                               prismaInput.ic_pads[(it.first / 4) * 4 + 3] > 0))
        {
            prisma.ic_ok = false;
            break;
        }
    }

    prisma.all_ok = prismaInput.mcp_ok && prismaInput.tof_ok && prismaInput.ppac_ok &&
                    prismaInput.side_ok && prismaInput.traj_ok && prismaInput.ic_ok;

    // MCP positioning part
    TVector3 mcpPos;

    mcpPos.SetXYZ(prismaInput.mcp_x, prismaInput.mcp_y, 0.);
    mcpPos.RotateY(conf.prisma.mcp_angle);
    mcpPos.RotateX(conf.prisma.mcpThetaX);
    mcpPos.SetZ(conf.prisma.target_mcp_distance + conf.prisma.zShift);

    mcpPos.RotateZ(conf.prisma.phi);
    mcpPos.RotateX(conf.prisma.thetaX);
    mcpPos.RotateY(conf.prisma.thetaY);
    mcpPos.RotateY(-conf.prisma.angle);

    // Beta settings
    prisma.TOF    = prismaInput.tof * conf.prisma.tofGain + conf.prisma.tofOffset;
    prisma.Length = prismaInput.length; // TO BE CHANGED!!!!!

    prismaInput.beta = prisma.Length / (prisma.TOF * UNITS::PHYSICS::c_light);

    if (prisma.mcp_ok)
    {
        prisma.beta.SetMagThetaPhi(prismaInput.beta, mcpPos.Theta(), mcpPos.Phi());
    }
    else
    {
        if (conf.prisma.betaAvg.Mag2() > 0)
            prisma.beta.SetMagThetaPhi(prismaInput.beta, conf.prisma.betaAvg.Theta(),
                                       conf.prisma.betaAvg.Phi());
    }

    // Reaction reconstruction
    ReactionFragment::Ion detectedIon{prisma.A, prisma.Z};
    // If the fragment is identified in prisma:
    auto reactionsIt = input.reactions.find(detectedIon);
    if (prisma.beta.Mag2() > 0 && reactionsIt != input.reactions.end())
    {
        // Evaluate mid-target energy
        auto &ion = reactionsIt->second.GetFixedFragment();
        ion.Set_Beta(prisma.beta);
        TLorentzVector p = ion.Get_P();
        EvalInTarget(p, detectedIon);
        prisma.p3 = p;

        auto TKEL =
            static_cast<double>(reactionsIt->second.Set_E_Theta_Phi(p.E(), p.Theta(), p.Phi()));
        prisma.p4 = reactionsIt->second.GetFreeFragment().Get_P();

        if (conf.prisma.prismafilter_tkel)
        {
            prisma.TKEL = prismaInput.qvalue;
        }
        else
        {
            prisma.TKEL        = TKEL;
            prismaInput.qvalue = TKEL;
        }
    }

    if (settings.verbosity_level > 2)
    {
        std::cout << "Prisma A: " << prisma.A << " Z: " << prisma.Z << " Q: " << prisma.Q
                  << " beta: " << prisma.beta.Mag() << std::endl;
    }

    if (conf.prisma.enable_cm_reconstruction)
    {
        LoadReactions();
        FissionCMReconstruction();
    }
    MaskAcceptanceLimits();
}

void PrismaAnalysis::LoadReactions()
{

    // std::vector<std::pair<unsigned int, unsigned int>> a_ranges =
    // {{64,82},{72,88},{86,100},{96,116},{112,138}}; std::vector<std::pair<unsigned int, unsigned
    // int>> z_ranges = {{28,32},{33,38},{38,42},{42,48},{48,56}};

    const auto a = static_cast<unsigned int>(prismaInput.a_nbr);
    const auto z = static_cast<unsigned int>(prismaInput.z_nbr);

    if (a <= 0 || a > 254 || z <= 0 || z > 254)
        return;

    if (settings.verbosity_level > 2)
    {
        std::cout << "Loading reaction for A: " << a << " Z: " << z << std::endl;
    }

    const ReactionFragment::Ion ion{a, z};
    if (mcpELoss.find(ion) != mcpELoss.end() && targetELoss.find(ion) != targetELoss.end())
        return;

    // ReactionFragment::FragmentSettings settingsIon{prismaInput.a_nbr,prismaInput.z_nbr};
    // ReactionFragment fragment;
    // try {fragment = ReactionFragment(settingsIon,conf.replay.conf_path+"/NuclearData/nutab12");}
    // catch (const std::exception& e) {
    //     if(settings.verbosity_level>2) std::cout << "Masses not found for A: " <<
    //     prismaInput.a_nbr << " Z: " << prismaInput.z_nbr << std::endl; return;}

    double midTargetBeamEnergy = input.midTargetBeamEnergy;
    if (settings.verbosity_level > 0)
    {
        std::cout << "Mid target energy " << midTargetBeamEnergy / UNITS::MeV << std::endl;
    }
    if (settings.verbosity_level > 80)
    {
        std::cout << "Before loading eloss for 12C and target for A: " << a << " Z: " << z
                  << std::endl;
    }
    std::string sTempMCP = conf.replay.conf_path + "/EnergyLoss/Target-12-6_Ion-" +
                           std::to_string(a) + "-" + std::to_string(z) + ".txt";
    try
    {
        mcpELoss.emplace(ion, EnergyLoss(sTempMCP, "SRIM", 100));
    }
    catch (const std::exception &e)
    {
        if (settings.verbosity_level > 2)
            std::cout << e.what() << std::endl;
        if (settings.verbosity_level > 2)
            std::cout << "Energy loss not found in 12C for A: " << a << " Z: " << z << std::endl;
        return;
    }
    if (settings.verbosity_level > 80)
    {
        std::cout << "After loading eloss for 12C and target for A: " << a << " Z: " << z
                  << std::endl;
    }
    std::string sTempTarget = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.material +
                              "_Ion-" + std::to_string(a) + "-" + std::to_string(z) + ".txt";
    try
    {
        targetELoss.emplace(ion, EnergyLoss(sTempTarget, "SRIM", 100));
    }
    catch (const std::exception &e)
    {
        if (settings.verbosity_level > 2)
            std::cout << e.what() << std::endl;
        if (settings.verbosity_level > 2)
            std::cout << "Energy loss not found in target for A: " << a << " Z: " << z << std::endl;
        return;
    }
    if (settings.verbosity_level > 80)
    {
        std::cout << "After loading eloss for target for A: " << a << " Z: " << z << std::endl;
        std::cout << "After loading eloss for target for A: " << ion.A << " Z: " << ion.Z
                  << std::endl;
    }

    // Reaction reconstruction part
    try
    {
        prismaReactions.emplace(
            std::piecewise_construct, std::forward_as_tuple(ion),
            std::forward_as_tuple(
                ReactionReconstruction2body<long double>::ReactionInput2body{
                    ReactionFragment::FragmentSettings(conf.reaction.beam.A, conf.reaction.beam.Z,
                                                       0, midTargetBeamEnergy, 0 * UNITS::MeV),
                    ReactionFragment::FragmentSettings(conf.reaction.target.A,
                                                       conf.reaction.target.Z, 0, 0 * UNITS::MeV,
                                                       0 * UNITS::MeV),
                    ReactionFragment::FragmentSettings(
                        conf.reaction.beam.A + conf.reaction.target.A - ion.A,
                        conf.reaction.beam.Z + conf.reaction.target.Z - ion.Z, 0, 0 * UNITS::MeV,
                        0 * UNITS::MeV),
                    ReactionFragment::FragmentSettings(ion.A, ion.Z, 0, 0, 0 * UNITS::MeV)},
                conf.replay.conf_path + "/NuclearData/nutab12"));
        prismaReactions.at(ion).ChooseFixed(4);
        prismaReactions.at(ion).ChooseExFixed(4);
    }
    catch (const std::exception &e)
    {
        if (settings.verbosity_level > 2)
            std::cout << "Reaction reconstruction not found for A: " << ion.A << " Z: " << ion.Z
                      << std::endl;
        if (settings.verbosity_level > 2)
            std::cout << e.what() << std::endl;
        return;
    }

    if (settings.verbosity_level > 2)
    {
        std::cout << "Reaction reconstruction loaded for A: " << ion.A << " Z: " << ion.Z
                  << std::endl;
    }
    // beamInTarget = new EnergyLoss(conf.replay.conf_path + "/EnergyLoss/Target-"
    //                              + conf.target.material + "_Ion-"
    //                              + std::to_string(conf.reaction.beam.A) + "-"
    //                              + std::to_string(conf.reaction.beam.Z) + ".txt", "SRIM", 100);
}

void PrismaAnalysis::FissionCMReconstruction()
{

    double thicknessC = 0.088770528 * UNITS::um;
    double angleC     = 45 * UNITS::deg;

    if (prismaInput.a_nbr <= 0 || prismaInput.a_nbr > 254 || prismaInput.z_nbr <= 0 ||
        prismaInput.z_nbr > 254)
        return;

    // Fission CM reconstructionTable file not found
    if (settings.verbosity_level > 2)
        std::cout << "Fission CM reconstruction  A: "
                  << static_cast<unsigned int>(prismaInput.a_nbr)
                  << " Z: " << static_cast<unsigned int>(prismaInput.z_nbr) << std::endl;
    ReactionFragment::Ion forcedIon{static_cast<unsigned int>(prismaInput.a_nbr),
                                    static_cast<unsigned int>(prismaInput.z_nbr)};
    // unsigned int A2 = conf.reaction.target.A
    //                 + conf.reaction.beam.A - forcedIon.A;
    // unsigned int Z2 = conf.reaction.target.Z
    //                 + conf.reaction.beam.Z - forcedIon.Z;
    // ReactionFragment::Ion ion4{A2, Z2};
    ReactionFragment fragment;
    try
    {
        fragment = ReactionFragment(
            ReactionFragment::FragmentSettings(static_cast<unsigned int>(prismaInput.a_nbr),
                                               static_cast<unsigned int>(prismaInput.z_nbr)),
            conf.replay.conf_path + "/NuclearData/nutab12");
    }
    catch (const std::exception &e)
    {
        if (settings.verbosity_level > 2)
            std::cout << e.what() << std::endl;
        return;
    }
    auto prismaReactionsIt = prismaReactions.find(forcedIon);

    if (prismaReactionsIt == prismaReactions.end() && betaCM.Mag() == 0)
    {
        if (settings.verbosity_level > 2)
            std::cout << "Skipping CM reconstruction for A: " << forcedIon.A
                      << " Z: " << forcedIon.Z << std::endl;
        return;
    }
    if (prismaReactionsIt != prismaReactions.end() && betaCM.Mag() == 0)
    {
        TLorentzVector ptot = prismaReactionsIt->second.GetReactionFragment(1).Get_P() +
                              prismaReactionsIt->second.GetReactionFragment(2).Get_P();
        betaCM = ptot.BoostVector();
        if (settings.verbosity_level > 5)
            std::cout << "Beta of CM set to: " << betaCM << std::endl;
    }

    // Evaluate mid-target energy
    auto mcpELossIt    = mcpELoss.find(forcedIon);
    auto targetELossIt = targetELoss.find(forcedIon);
    if (mcpELossIt == mcpELoss.end())
    {
        if (settings.verbosity_level > 2)
            std::cout << "Energy loss MCP not found for ion A: " << forcedIon.A
                      << " Z: " << forcedIon.Z << std::endl;
        return;
    }
    if (targetELossIt == targetELoss.end())
    {
        if (settings.verbosity_level > 2)
            std::cout << "Energy loss Target not found for ion A: " << forcedIon.A
                      << " Z: " << forcedIon.Z << std::endl;
        return;
    }
    TVector3 normMcp{0, 0, 1};
    normMcp.RotateY(-(angleC + conf.prisma.angle));
    double angleWithCarbon = normMcp.Angle(prisma.beta);
    fragment.Set_Beta(prisma.beta);
    if (settings.verbosity_level > 2)
        std::cout << "Theta " << fragment.Get_P().Theta() << " Phi " << fragment.Get_P().Phi()
                  << std::endl;
    double ionEn   = fragment.Get_Ek();
    double beforeC = mcpELossIt->second.EvaluateInitialEnergy(ionEn, thicknessC, angleWithCarbon);
    fragment.Set_Ek(beforeC);
    if (settings.verbosity_level > 2)
        std::cout << "Prisma beta: " << prisma.beta.Mag() << " Ion energy: " << ionEn
                  << " Energy before C: " << beforeC << " Beta before C " << fragment.Get_P().Beta()
                  << std::endl;
    if (settings.verbosity_level > 2)
        std::cout << "Theta " << fragment.Get_P().Theta() << " Phi " << fragment.Get_P().Phi()
                  << std::endl;
    prismaInput.beta_end   = fragment.Get_P().Beta();
    prismaInput.energy_end = beforeC;
    double angleWithTarget = fragment.Get_P().Angle(conf.target.targetNorm);

    double inTarget;
    try
    {
        inTarget = targetELossIt->second.EvaluateInitialEnergyWithDensity(
            beforeC, conf.target.thickness * (1 - conf.reaction.reaction_position),
            angleWithTarget);
    }
    catch (const std::exception &e)
    {
        std::cout << inTarget / UNITS::MeV << std::endl;
        if (settings.verbosity_level > 2)
            std::cout << e.what() << " A " << forcedIon.A << " Z " << forcedIon.Z << std::endl;
        return;
    }
    fragment.Set_Ek(inTarget);
    if (settings.verbosity_level > 2)
        std::cout << "Beta center target: " << fragment.Get_P().Beta() << std::endl;
    if (settings.verbosity_level > 2)
        std::cout << "Theta " << fragment.Get_P().Theta() << " Phi " << fragment.Get_P().Phi()
                  << std::endl;
    prismaInput.beta_mid  = fragment.Get_P().Beta();
    prismaInput.gamma_mid = 1 / sqrt(1 - pow(prismaInput.beta_mid, 2));
    prismaInput.brho_mid  = 3.107 * prismaInput.beta_mid * prismaInput.gamma_mid *
                           prismaInput.a_nbr / prismaInput.q_nbr;
    prismaInput.energy_mid = inTarget;

    // prismaReactionsIt->second.Set_Ek_Theta_Phi(inTarget, fragment.Get_P().Theta(),
    // fragment.Get_P().Phi()); auto reactionFragment =
    // prismaReactionsIt->second.GetFixedFragment(); TLorentzVector p_cm =
    // reactionFragment.Get_P_cm(); if(settings.verbosity_level>2) std::cout << "Prisma beta cm: "
    // << reactionFragment.Get_betacm()  << std::endl; if(settings.verbosity_level>2) std::cout <<
    // "Theta " << reactionFragment.Get_P_cm().Theta() << " Phi " <<
    // reactionFragment.Get_P_cm().Phi() << std::endl;
    //
    // if(settings.verbosity_level>2)std::cout << "Beta cm : "<< reactionFragment.Get_betacm() <<"
    // Beta center target cm ref: " << reactionFragment.Get_P_cm().Beta() << std::endl;
    // if(settings.verbosity_level>2)std::cout << "Beta CM Theta " <<
    // reactionFragment.Get_betacm().Theta() << " Phi " << reactionFragment.Get_betacm().Phi() <<
    // std::endl;

    if (settings.verbosity_level > 2)
        std::cout << "Beta of CM: " << betaCM << std::endl;
    fragment.Set_betacm(betaCM);
    fragment.BoostToCm();
    TLorentzVector p_cm = fragment.Get_P_cm();

    TVector3 p3_cm       = p_cm.Vect();
    double   theta_cm    = p3_cm.Theta();
    double   phi_cm      = p3_cm.Phi();
    double   beta_cm     = p_cm.Beta();
    prismaInput.beta_cm  = beta_cm;
    prismaInput.theta_cm = theta_cm;
    prismaInput.phi_cm   = phi_cm;
    prismaInput.brho_cm =
        3.107 * beta_cm / sqrt(1 - pow(beta_cm, 2)) * prismaInput.a_nbr / prismaInput.q_nbr;
    if (settings.verbosity_level > 2)
        std::cout << "Beta in CM: " << beta_cm << " Theta in CM: " << theta_cm
                  << " Phi in CM: " << phi_cm << std::endl;

    // else {
    //     if(settings.verbosity_level>2) std::cout << "Reaction not found for A: " <<
    //     static_cast<unsigned int>(prismaInput.a_nbr) << " Z: " << static_cast<unsigned
    //     int>(prismaInput.z_nbr) << std::endl; return;
    // }
}

void PrismaAnalysis::MaskAcceptanceLimits()
{

    // if(prismaInput.a_nbr <= 0 || prismaInput.a_nbr > 254 || prismaInput.z_nbr < 0 ||
    // prismaInput.z_nbr > 254) return;

    if (conf.prisma.acceptance_mask_file != "" && mask_cut != nullptr)
    {

        if (mask_cut->IsInside(prismaInput.theta * TMath::RadToDeg(), prismaInput.brho))
        {
            const int           n  = mask_cut->GetN();
            const double       *xs = mask_cut->GetX();
            const double       *ys = mask_cut->GetY();
            const double        x0 = prismaInput.theta * TMath::RadToDeg();
            std::vector<double> y_intersections;
            double              xmin = std::numeric_limits<double>::max();
            double              xmax = std::numeric_limits<double>::min();
            for (int i = 0; i < n; ++i)
            {
                xmin = std::min(xmin, xs[i]);
                xmax = std::max(xmax, xs[i]);
            }
            if ((x0 < xmin) || (x0 > xmax))
                return;

            for (int i = 0; i < n; ++i)
            {
                int j = (i + 1) % n;

                const double x1 = xs[i], y1 = ys[i];
                const double x2 = xs[j], y2 = ys[j];
                if (y_intersections.size() == 2)
                    break;
                // Skip edge if x0 is outside the horizontal span of the segment

                if ((x0 > x1 && x0 > x2) || (x0 < x2 && x0 < x1))
                    continue;
                // Linear interpolation: find y at x = x0
                const double t = (x0 - x1) / (x2 - x1);
                const double y = y1 + t * (y2 - y1);

                y_intersections.push_back(y);
            }
            if (settings.verbosity_level > 16)
                std::cout << std::setprecision(6) << "Limits Brho: " << y_intersections[0] << " "
                          << y_intersections[1] << std::endl;
            if (y_intersections.size() == 2)
                std::sort(y_intersections.begin(), y_intersections.end());
            else
            {
                std::cout << "2 Brho limits not found" << std::endl;
            }

            prismaInput.acc_brho_min = y_intersections[0];
            prismaInput.acc_brho_max = y_intersections[1];
            double c_min = y_intersections[0] / (3.107 * prismaInput.a_nbr / prismaInput.q_nbr);
            double c_max = y_intersections[1] / (3.107 * prismaInput.a_nbr / prismaInput.q_nbr);
            prismaInput.acc_beta_min = c_min / sqrt(1 + pow(c_min, 2));
            prismaInput.acc_beta_max = c_max / sqrt(1 + pow(c_max, 2));
        }
    }
}

bool PrismaAnalysis::FillHistos()
{
    if (conf.prisma.analysis_stages != PrismaConf::AnalysisStage::HIGHLEVEL)
        PrismaInputAnalysis::FillHistos();

    if (!DetectorAnalysis::FillHistos())
        return false;
    PrismaHistograms &h = prismaHistograms;

    // raw
    if (conf.prisma.raw)
    {
        for (unsigned int i{0}; i < h.raw.ppac.h_XRmXL.size(); i++)
        {
            static short xr, xl, xc, tof_raw;
            xr      = signedToUnsigned<short, unsigned short>(prismaReader.PPAC_Xright_raw->At(i));
            xl      = signedToUnsigned<short, unsigned short>(prismaReader.PPAC_Xleft_raw->At(i));
            xc      = signedToUnsigned<short, unsigned short>(prismaReader.PPAC_Cathode_raw->At(i));
            tof_raw = signedToUnsigned<short, unsigned short>(prismaReader.TOF_raw->At(i));
            // check overflow or underflow
            if (xr > 0 && xr < 4000)
                Fill(h.raw.ppac.h_XR.at(i), xr);
            if (xl > 0 && xl < 4000)
                Fill(h.raw.ppac.h_XL.at(i), xl);
            if (xc > 0 && xc < 4000)
                Fill(h.raw.ppac.h_Cathode.at(i), xc);

            if (xr > 0 && xl > 0 && xr < 4000 && xl < 4000)
                Fill(h.raw.ppac.h_XRmXL.at(i), xr - xl);
            if (xr > 0 && xc > 0 && xr < 4000 && xc < 4000)
                Fill(h.raw.ppac.h_XRmCath.at(i), xr - xc);
            if (xc > 0 && xl > 0 && xc < 4000 && xl < 4000)
                Fill(h.raw.ppac.h_CathmXL.at(i), xc - xl);
            Fill(h.raw.ppac.h_TOF_raw.at(i), tof_raw);
        }

        // ic pads raw
        for (unsigned int i{0}; i < h.raw.ic.h_IC_A.size(); i++)
        {
            // if(!h.raw.ic.h_IC_A.empty())
            Fill(h.raw.ic.h_IC_A.at(i), prismaReader.IC_A_raw->At(i));
            Fill(h.raw.ic.m_ICNr_ICE, prismaReader.IC_A_raw->At(i), i);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_IC_B.size(); i++)
        {
            // if(!h.raw.ic.h_IC_B.empty())
            Fill(h.raw.ic.h_IC_B.at(i), prismaReader.IC_B_raw->At(i));
            Fill(h.raw.ic.m_ICNr_ICE, prismaReader.IC_B_raw->At(i), i + 10);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_IC_C.size(); i++)
        {
            // if(!h.raw.ic.h_IC_C.empty())
            Fill(h.raw.ic.h_IC_C.at(i), prismaReader.IC_C_raw->At(i));
            Fill(h.raw.ic.m_ICNr_ICE, prismaReader.IC_C_raw->At(i), i + 20);
        }
        for (unsigned int i{0}; i < h.raw.ic.h_IC_D.size(); i++)
        {
            // if(!h.raw.ic.h_IC_D.empty())
            Fill(h.raw.ic.h_IC_D.at(i), prismaReader.IC_D_raw->At(i));
            Fill(h.raw.ic.m_ICNr_ICE, prismaReader.IC_D_raw->At(i), i + 30);
        }

        // sides
        Fill(h.raw.ic.h_Side_A.at(0), prismaReader.Side_A_raw->At(0));
        Fill(h.raw.ic.h_Side_A.at(1), prismaReader.Side_A_raw->At(1));
        Fill(h.raw.ic.h_Side_B.at(0), prismaReader.Side_B_raw->At(0));
        Fill(h.raw.ic.h_Side_B.at(1), prismaReader.Side_B_raw->At(1));
        Fill(h.raw.ic.h_Side_C.at(0), prismaReader.Side_C_raw->At(0));
        Fill(h.raw.ic.h_Side_C.at(1), prismaReader.Side_C_raw->At(1));
        Fill(h.raw.ic.h_Side_D.at(0), prismaReader.Side_D_raw->At(0));
        Fill(h.raw.ic.h_Side_D.at(1), prismaReader.Side_D_raw->At(1));

        // 2-D histrograms
        // raw
        for (unsigned int i{0}; i < h.raw.ppac.m_XRpXL_CATHODE.size(); i++)
        {
            // if(h.raw.ppac.m_XRpXL_CATHODE!=nullptr)
            Fill(h.raw.ppac.m_XRpXL_CATHODE.at(i), prismaReader.PPAC_Cathode_raw->At(i),
                 prismaReader.PPAC_Xright_raw->At(i) + prismaReader.PPAC_Xleft_raw->At(i));
        }
        Fill(h.raw.mcp.m_MCPX_MCPY, prismaReader.MCP_raw->At(0),
             prismaReader.MCP_raw->At(1)); // real coord Y,-X,Z
    }

    // misc
    if (conf.prisma.misc)
    {
        for (unsigned int i{0}; i < h.misc.m_TOFraw_XFP.size(); i++)
        {
            // if(h.misc.m_TOFraw_XFP!=nullptr)
            Fill(h.misc.m_TOFraw_XFP.at(i), prismaInput.x_fp / UNITS::mm,
                 prismaReader.TOF_raw->At(i));
        }
        for (unsigned int i{0}; i < h.misc.m_XFP_CATHODEmXL.size(); i++)
        {
            // if(h.misc.m_TOFraw_XFP!=nullptr)
            Fill(h.misc.m_XFP_CATHODEmXL.at(i),
                 prismaReader.PPAC_Cathode_raw->At(i) - prismaReader.PPAC_Xleft_raw->At(i),
                 prismaInput.x_fp / UNITS::mm);
        }
        for (unsigned int i{0}; i < h.misc.m_XFP_XRmCATHODE.size(); i++)
        {
            // if(h.misc.m_TOFraw_XFP!=nullptr)
            Fill(h.misc.m_XFP_XRmCATHODE.at(i),
                 prismaReader.PPAC_Xright_raw->At(i) - prismaReader.PPAC_Cathode_raw->At(i),
                 prismaInput.x_fp / UNITS::mm);
        }
    }

    // analyzed
    if (conf.prisma.ana && prisma.TS > 0)
    {
        Fill(h.ana.m_goodEvtsStat, 0., prismaInput.mcp_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 1., prismaInput.tof_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 2., prismaInput.ppac_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 3., prismaInput.side_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 4., prismaInput.traj_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 5., prismaInput.ic_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 6., prismaInput.tof_ok && prismaInput.mcp_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 7., prismaInput.tof_ok && prismaInput.ic_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 8.,
             prismaInput.ic_ok && prismaInput.mcp_ok && prismaInput.side_ok ? 1 : -1);
        Fill(h.ana.m_goodEvtsStat, 9., prismaInput.all_ok ? 1 : -1);

        // if(!prisma.all_ok) return;
        if (!prisma.mcp_ok && conf.prisma.require_mcpok)
            return true;
        if (!prisma.tof_ok && conf.prisma.require_tofok)
            return true;
        if (!prisma.side_ok && conf.prisma.require_sideok)
            return true;
        if (!prisma.traj_ok && conf.prisma.require_trajok)
            return true;
        if (!prisma.ic_ok && conf.prisma.require_icok)
            return true;

        // Plots from here on are only with prisma ok!
        Fill(h.ana.h_XFP, prismaInput.x_fp / GetUnitsLength());

        if (static_cast<int>(prismaInput.col_nbr) >= 0 &&
            static_cast<int>(prismaInput.col_nbr) <= 10)
        {
            Fill(h.ana.xfp.h_XFP_sections.at(static_cast<int>(prismaInput.col_nbr)),
                 prismaInput.x_fp / GetUnitsLength());
            if (prismaInput.x_fp_type == 0)
                Fill(h.ana.xfp.h_XFP_RL_sections.at(static_cast<int>(prismaInput.col_nbr)),
                     prismaInput.x_fp / GetUnitsLength());
            if (prismaInput.x_fp_type == 1)
                Fill(h.ana.xfp.h_XFP_RC_sections.at(static_cast<int>(prismaInput.col_nbr)),
                     prismaInput.x_fp / GetUnitsLength());
            if (prismaInput.x_fp_type == 2)
                Fill(h.ana.xfp.h_XFP_CL_sections.at(static_cast<int>(prismaInput.col_nbr)),
                     prismaInput.x_fp / GetUnitsLength());
        }

        Fill(h.ana.m_MCPX_MCPY, prismaInput.mcp_x / GetUnitsLength(),
             prismaInput.mcp_y / GetUnitsLength());
        Fill(h.ana.m_MCPPhi_MCPTheta, prismaInput.theta / UNITS::deg,
             (prismaInput.phi > 0 ? prismaInput.phi : prismaInput.phi + UNITS::CONSTANTS::pi * 2) /
                 UNITS::deg);
        Fill(h.ana.m_XFP_YFP, prismaInput.x_fp / GetUnitsLength(),
             prismaInput.y_fp / GetUnitsLength());
        Fill(h.ana.m_TOF_XFP, prismaInput.x_fp / GetUnitsLength(), prismaInput.tof * 1e9);

        Fill(h.ana.m_Monitors, prismaInput.monitor_0, 1.);
        Fill(h.ana.m_Monitors, prismaInput.monitor_1, 2.);

        // Plots all IC_Pads
        for (unsigned int i{0}; i < 40; i++)
            Fill(h.ana.m_ICNr_ICE, prismaInput.ic_pads[i], i);

        // de:e for single column with some conditions
        if (prisma.mcp_ok && prisma.tof_ok && prisma.traj_ok && prisma.side_ok && prisma.ic_ok &&
            prismaInput.ic_a_numpads == 1 && prismaInput.ic_b_numpads == 1)
        {
            for (unsigned int i{0}; i < 10; i++)
            {
                if (prismaInput.ic_col_a == i && prismaInput.ic_col_b == i)
                {
                    Fill(h.ana.de_e.dea_e.m_ICDEA_ICE.at(i), prismaInput.ic_e, prismaInput.ic_de_a);
                }
            }
            if (prismaInput.ic_c_numpads == 1)
            {
                for (unsigned int i{0}; i < 10; i++)
                {
                    if (prismaInput.ic_col_a == i && prismaInput.ic_col_b == i &&
                        prismaInput.ic_col_c == i)
                    {
                        Fill(h.ana.de_e.deab_e.m_ICDEAB_ICE.at(i), prismaInput.ic_e,
                             prismaInput.ic_de_ab);
                    }
                }
            }
        }

        Fill(h.ana.h_ANr, prismaInput.a_nbr);
        Fill(h.ana.h_ZNr, prismaInput.z_nbr);
        Fill(h.ana.h_qNr, prismaInput.q_nbr);
        Fill(h.ana.h_Beta, prisma.beta.Mag());
        // Fill(h.ana.h_Beta, prisma.p3.BoostVector().Mag());
        Fill(h.ana.qvalue.h_Qvalue, prismaInput.qvalue);

        Fill(h.ana.qvalue.m_Qvalue_TKEL, prismaInput.qvalue, prisma.TKEL);

        Fill(h.ana.m_ZNr_NNr, prismaInput.z_nbr, prismaInput.a_nbr - prismaInput.z_nbr);
        Fill(h.ana.m_Zed_Mass, prismaInput.mass, prismaInput.zed);
        Fill(h.ana.m_Zfloat_Mass, prismaInput.mass, prismaInput.z_float);
        Fill(h.ana.m_Zfloat_Qfloat, prismaInput.q_float, prismaInput.z_float);
        Fill(h.ana.m_Qfloat_Mass, prismaInput.mass, prismaInput.q_float);
        Fill(h.ana.m_Brho_Theta, prismaInput.theta / UNITS::deg, prismaInput.brho);

        // de_e
        Fill(h.ana.de_e.dea_e.m_ICDEA_ICE_tot, prismaInput.ic_e, prismaInput.ic_de_a_traj);
        Fill(h.ana.de_e.dea_e.m_ICDEA_ICE_corr_tot, prismaInput.ic_e, prismaInput.ic_de_a_corr);
        Fill(h.ana.de_e.deab_e.m_ICDEAB_ICE_tot, prismaInput.ic_e, prismaInput.ic_de_ab_traj);
        Fill(h.ana.de_e.deab_e.m_ICDEAB_ICE_corr_tot, prismaInput.ic_e, prismaInput.ic_de_ab_corr);
        Fill(h.ana.de_e.dea_eres.m_ICDEA_ICEres_tot, prismaInput.ic_e_res_a, prismaInput.ic_de_a);
        Fill(h.ana.de_e.dea_eres.m_ICDEA_ICEres_corr_tot, prismaInput.ic_e_res_a_corr,
             prismaInput.ic_de_a_corr);
        Fill(h.ana.de_e.deab_eres.m_ICDEAB_ICEres_tot, prismaInput.ic_e_res_ab,
             prismaInput.ic_de_ab);
        Fill(h.ana.de_e.deab_eres.m_ICDEAB_ICEres_corr_tot, prismaInput.ic_e_res_ab_corr,
             prismaInput.ic_de_ab_corr);

        Fill(h.ana.de_e.m_ICen1_ICen2, prismaInput.ic_en2, prismaInput.ic_en1);
        Fill(h.ana.de_e.m_ICen1_Yfp, prismaInput.y_fp / GetUnitsLength(), prismaInput.ic_en1);
        Fill(h.ana.de_e.m_ICen1_corr1_ICen2, prismaInput.ic_en2, prismaInput.ic_en1_corr1);
        Fill(h.ana.de_e.m_ICen1_corr1_Xfp, prismaInput.x_fp / GetUnitsLength(),
             prismaInput.ic_en1_corr1);
        Fill(h.ana.de_e.m_ICen1_corr2_ICen2, prismaInput.ic_en2, prismaInput.ic_en1_corr2);

        Fill(h.ana.de_e.h_Zed, prismaInput.zed);
        Fill(h.ana.de_e.h_Zfloat, prismaInput.z_float);
        Fill(h.ana.de_e.m_Zfloat_ICEres_corr,
             conf.prisma.de_type == 6 ? prismaInput.ic_e_res_ab_corr : prismaInput.ic_e_res_a_corr,
             prismaInput.z_float);

        // ic_e:xfp
        Fill(h.ana.ice_xfp.m_ICE_XFP_tot, prismaInput.x_fp / GetUnitsLength(),
             prismaInput.ic_e_corr);
        for (unsigned int i = 0; i < h.ana.ice_xfp.m_ICE_XFP.size(); i++)
        {
            if (prismaInput.z_nbr == i + conf.prisma.Zrange.first)
            {
                if (!h.ana.ice_xfp.m_ICE_XFP.empty())
                    Fill(h.ana.ice_xfp.m_ICE_XFP.at(i), prismaInput.x_fp / GetUnitsLength(),
                         prismaInput.ic_e_corr);
            }
        }

        // ic_e:rbeta
        Fill(h.ana.ice_rbeta.m_ICE_RBeta_tot, prismaInput.rbeta / GetUnitsLength(),
             prismaInput.ic_e_corr);
        for (unsigned int i = 0; i < h.ana.ice_rbeta.m_ICE_RBeta.size(); i++)
        {
            if (prismaInput.z_nbr == i + conf.prisma.Zrange.first)
            {
                if (!h.ana.ice_rbeta.m_ICE_RBeta.empty())
                    Fill(h.ana.ice_rbeta.m_ICE_RBeta.at(i), prismaInput.rbeta / GetUnitsLength(),
                         prismaInput.ic_e_corr);
            }
        }

        // I_CE/RBeta : IC_E
        Fill(h.ana.iceoverrbeta_ice.m_ICEoverRBeta_ICE_tot, prismaInput.ic_e_corr,
             prismaInput.ic_e_corr / ((prismaInput.radius / GetUnitsLength()) * prismaInput.beta));
        for (unsigned int i = 0; i < h.ana.iceoverrbeta_ice.m_ICEoverRBeta_ICE.size(); i++)
        {
            if (prismaInput.z_nbr == i + conf.prisma.Zrange.first)
            {
                if (!h.ana.iceoverrbeta_ice.m_ICEoverRBeta_ICE.empty())
                    Fill(h.ana.iceoverrbeta_ice.m_ICEoverRBeta_ICE.at(i), prismaInput.ic_e_corr,
                         prismaInput.ic_e_corr /
                             ((prismaInput.radius / GetUnitsLength()) * prismaInput.beta));
            }
        }

        // Charge state : A/Q
        Fill(h.ana.charge.m_Charge_cal_AoQ_tot, prismaInput.a_over_q, prismaInput.charge_cal);
        if (prismaInput.z_nbr > 0)
        {
            Fill(h.ana.charge.m_Charge_cal_AoQ_Z_tot, prismaInput.a_over_q, prismaInput.charge_cal);
            Fill(h.ana.charge.m_Charge_cal_AoQ_Z_tot, prismaInput.a_over_q, prismaInput.charge_cal);
            Fill(h.ana.charge.m_Charge_cal_beta_tot, prismaInput.beta, prismaInput.charge_cal);
            Fill(h.ana.charge.m_Charge_cal_Xfp_tot, prismaInput.x_fp / GetUnitsLength(),
                 prismaInput.charge_cal);
            Fill(h.ana.charge.h_Charge_cal_Z_tot, prismaInput.charge_cal);
            Fill(h.ana.charge.h_Qfloat, prismaInput.q_float);
            Fill(h.ana.charge.m_Qfloat_AoQ, prismaInput.a_over_q, prismaInput.q_float);
            Fill(h.ana.charge.m_Qfloat_beta, prismaInput.beta, prismaInput.q_float);
        }

        // A/q
        Fill(h.ana.aoverq.h_Aoverq_tot, prismaInput.a_over_q * GetUnitsAoQ());
        if (prismaInput.col_nbr > 0 && prismaInput.col_nbr < 10)
            Fill(h.ana.aoverq.h_Aoverq_cols[prismaInput.col_nbr],
                 prismaInput.a_over_q * GetUnitsAoQ());
        Fill(h.ana.aoverq.m_Aoverq_XFP_tot, prismaInput.x_fp / GetUnitsLength(),
             prismaInput.a_over_q * GetUnitsAoQ());
        Fill(h.ana.aoverq.m_Aoverq_Xmcp_tot, prismaInput.mcp_x / GetUnitsLength(),
             prismaInput.a_over_q * GetUnitsAoQ());
        Fill(h.ana.aoverq.m_Aoverq_Ymcp_tot, prismaInput.mcp_y / GetUnitsLength(),
             prismaInput.a_over_q * GetUnitsAoQ());
        // Z gated
        for (unsigned int z_idx = 0; z_idx < h.ana.aoverq.zGated.histos.h_Aoverq.size(); z_idx++)
        {
            unsigned int z{z_idx + conf.prisma.Zrange.first};
            if (prismaInput.z_nbr == z)
            {
                if (!h.ana.aoverq.zGated.histos.h_Aoverq.empty())
                    Fill(h.ana.aoverq.zGated.histos.h_Aoverq.at(z_idx),
                         prismaInput.a_over_q * GetUnitsAoQ());
                if (!h.ana.aoverq.zGated.matricesXfp.m_Aoverq.empty())
                    Fill(h.ana.aoverq.zGated.matricesXfp.m_Aoverq.at(z_idx),
                         prismaInput.x_fp / GetUnitsLength(), prismaInput.a_over_q * GetUnitsAoQ());
                if (!h.ana.aoverq.zGated.matricesYfp.m_Aoverq.empty())
                    Fill(h.ana.aoverq.zGated.matricesYfp.m_Aoverq.at(z_idx),
                         prismaInput.y_fp / GetUnitsLength(), prismaInput.a_over_q * GetUnitsAoQ());
                if (!h.ana.aoverq.zGated.matricesXmcp.m_Aoverq.empty())
                    Fill(h.ana.aoverq.zGated.matricesXmcp.m_Aoverq.at(z_idx),
                         prismaInput.mcp_x / GetUnitsLength(),
                         prismaInput.a_over_q * GetUnitsAoQ());
                if (!h.ana.aoverq.zGated.matricesYmcp.m_Aoverq.empty())
                    Fill(h.ana.aoverq.zGated.matricesYmcp.m_Aoverq.at(z_idx),
                         prismaInput.mcp_y / GetUnitsLength(),
                         prismaInput.a_over_q * GetUnitsAoQ());

                // Time stability of a over q
                auto ppac_section{
                    static_cast<int>(std::floor(prismaInput.x_fp / GetUnitsLength() / 100))};
                if (ppac_section < 10 && ppac_section >= 0 && conf.prisma.Aoverq_tevo_hists)
                {
                    Fill(h.ana.aoverq.zGated.matricesTime.at(ppac_section).m_Aoverq.at(z_idx),
                         unsignedDiff<double, unsigned long long>(prisma.TS, input.firstTS) / 1.e8 /
                             60.,
                         prismaInput.a_over_q * GetUnitsAoQ());
                }
            }
        }
        // Z,Q gated
        for (unsigned int i = 0; i < conf.prisma.aqConfs.size(); i++)
        {
            const PrismaConf::AqConf &currentConf = conf.prisma.aqConfs.at(i);
            for (int j = 0; j <= currentConf.qmax - currentConf.qmin; j++)
            {
                if (prismaInput.z_nbr == currentConf.Z && prismaInput.q_nbr == j + currentConf.qmin)
                {
                    if (!h.ana.aoverq.zqGated.histos.h_Aoverq.empty())
                        Fill(h.ana.aoverq.zqGated.histos.h_Aoverq.at(i).at(j),
                             prismaInput.a_over_q * GetUnitsAoQ());
                    if (!h.ana.aoverq.zqGated.matricesXfp.m_Aoverq.empty())
                        Fill(h.ana.aoverq.zqGated.matricesXfp.m_Aoverq.at(i).at(j),
                             prismaInput.x_fp / GetUnitsLength(),
                             prismaInput.a_over_q * GetUnitsAoQ());
                    if (!h.ana.aoverq.zqGated.matricesYfp.m_Aoverq.empty())
                        Fill(h.ana.aoverq.zqGated.matricesYfp.m_Aoverq.at(i).at(j),
                             prismaInput.y_fp / GetUnitsLength(),
                             prismaInput.a_over_q * GetUnitsAoQ());
                    if (!h.ana.aoverq.zqGated.matricesXmcp.m_Aoverq.empty())
                        Fill(h.ana.aoverq.zqGated.matricesXmcp.m_Aoverq.at(i).at(j),
                             prismaInput.mcp_x / GetUnitsLength(),
                             prismaInput.a_over_q * GetUnitsAoQ());
                    if (!h.ana.aoverq.zqGated.matricesYmcp.m_Aoverq.empty())
                        Fill(h.ana.aoverq.zqGated.matricesYmcp.m_Aoverq.at(i).at(j),
                             prismaInput.mcp_y / GetUnitsLength(),
                             prismaInput.a_over_q * GetUnitsAoQ());
                }
            }
        }

        // Mass
        Fill(h.ana.mass.h_Mass_tot, prismaInput.mass);
        Fill(h.ana.mass.m_Mass_XFP_tot, prismaInput.x_fp / UNITS::mm, prismaInput.mass);
        // Z gated
        for (unsigned int i = 0; i < h.ana.mass.zGated.histos.h_Mass.size(); i++)
        {
            unsigned int z;
            z = i + conf.prisma.Zrange.first;
            if (prismaInput.z_nbr == z)
            {
                if (!h.ana.mass.zGated.histos.h_Mass.empty())
                    Fill(h.ana.mass.zGated.histos.h_Mass.at(i), prismaInput.mass);
                if (!h.ana.mass.zGated.matrices.m_Mass_XFP.empty())
                    Fill(h.ana.mass.zGated.matrices.m_Mass_XFP.at(i), prismaInput.x_fp / UNITS::mm,
                         prismaInput.mass);
            }
        }
        // Z,Q gated
        for (unsigned int i = 0; i < conf.prisma.aqConfs.size(); i++)
        {
            const PrismaConf::AqConf &currentConf = conf.prisma.aqConfs.at(i);
            for (int j = 0; j <= currentConf.qmax - currentConf.qmin; j++)
            {
                if (prismaInput.z_nbr == currentConf.Z && prismaInput.q_nbr == j + currentConf.qmin)
                {
                    if (!h.ana.mass.zGated.histos.h_Mass.empty())
                        Fill(h.ana.mass.zqGated.histos.h_Mass.at(i).at(j), prismaInput.mass);
                    if (!h.ana.mass.zGated.matrices.m_Mass_XFP.empty())
                        Fill(h.ana.mass.zqGated.matrices.m_Mass_XFP.at(i).at(j),
                             prismaInput.x_fp / UNITS::mm, prismaInput.mass);
                }
            }
        }

        // Ion histograms
        ReactionFragment::Ion detectedIon{prisma.A, prisma.Z};
        auto                  histoIt = h.ana.qvalue.ionHistos.find(detectedIon);
        if (histoIt != h.ana.qvalue.ionHistos.end())
        {
            Fill(histoIt->second.h_Qvalue, prisma.TKEL / UNITS::MeV);
        }

        // Time evolution
        Fill(h.ana.timeEvo.m_TOF_time, prisma.TS / 1.e8 / 60., prismaInput.tof * 1e9);
        Fill(h.ana.timeEvo.m_Aoverq_time, prisma.TS / 1.e8 / 60., prismaInput.a_over_q);
        if (prismaInput.z_nbr == 38 && prismaInput.q_nbr == 32)
            Fill(h.ana.timeEvo.m_Aoverq_z38_time, prisma.TS / 1.e8 / 60., prismaInput.a_over_q);
        if (abs(prismaInput.a_over_q - 3) < 0.015)
            Fill(h.ana.timeEvo.m_MassIC_time, prisma.TS / 1.e8 / 60., prismaInput.ic_mass);
        Fill(h.ana.timeEvo.m_MassTOF_time, prisma.TS / 1.e8 / 60., prismaInput.mass);
        Fill(h.ana.timeEvo.m_Charge_time, prisma.TS / 1.e8 / 60., prismaInput.charge_cal);
        Fill(h.ana.timeEvo.m_Xfp_time, prisma.TS / 1.e8 / 60., prismaInput.x_fp);
        Fill(h.ana.timeEvo.m_Yfp_time, prisma.TS / 1.e8 / 60., prismaInput.y_fp);
        Fill(h.ana.timeEvo.m_Xmcp_time, prisma.TS / 1.e8 / 60., prismaInput.mcp_x);
        if (prismaInput.mcp_y < 0)
            Fill(h.ana.timeEvo.m_Xmcp_zoom_time, prisma.TS / 1.e8 / 60., prismaInput.mcp_x);
        Fill(h.ana.timeEvo.m_Ymcp_time, prisma.TS / 1.e8 / 60., prismaInput.mcp_y);
        Fill(h.ana.timeEvo.m_ICdEA_time, prisma.TS / 1.e8 / 60., prismaInput.ic_de_a_corr);
        Fill(h.ana.timeEvo.m_ICen1corr2_time, prisma.TS / 1.e8 / 60., prismaInput.ic_en1_corr2);
        Fill(h.ana.timeEvo.m_ICEres_time, prisma.TS / 1.e8 / 60.,
             conf.prisma.de_type == 6 ? prismaInput.ic_e_res_ab_corr : prismaInput.ic_e_res_a_corr);

        // Fission CM reconstruction
        if (conf.prisma.enable_cm_reconstruction)
        {
            Fill(h.ana.cmReco.h_betaCM, prismaInput.beta_cm);
            if (prismaInput.a_nbr == 98 && prismaInput.z_nbr == 40 &&
                prismaInput.theta_cm / UNITS::deg > 65)
                Fill(h.ana.cmReco.h_betaCM_98Zr, prismaInput.beta_cm);
            Fill(h.ana.cmReco.h_thetaCM, prismaInput.theta_cm / UNITS::deg);
            Fill(h.ana.cmReco.h_phiCM,
                 (prismaInput.phi_cm > 0 ? prismaInput.phi_cm
                                         : prismaInput.phi_cm + TMath::Pi() * 2) /
                     UNITS::deg);
            Fill(h.ana.cmReco.m_betaCM_thetaCM, prismaInput.theta_cm / UNITS::deg,
                 prismaInput.beta_cm);
            Fill(h.ana.cmReco.m_phiCM_thetaCM, prismaInput.theta_cm / UNITS::deg,
                 prismaInput.phi_cm / UNITS::deg);
            Fill(h.ana.cmReco.m_betaCM_cosThetaCM_sinThetaCM,
                 prismaInput.beta_cm * TMath::Cos(prismaInput.theta_cm),
                 prismaInput.beta_cm * TMath::Sin(prismaInput.theta_cm), prismaInput.beta_cm);

            auto recoHistoIt = h.ana.cmReco.recoIonHistos.find(detectedIon);
            if (recoHistoIt != h.ana.cmReco.recoIonHistos.end())
            {
                if (thetaCMLimits.InsideThreshold({prismaInput.a_nbr, prismaInput.z_nbr},
                                                  prismaInput.theta_cm / UNITS::deg))
                {
                    Fill(recoHistoIt->second.h_betaCM_ion, prismaInput.beta_cm);
                    Fill(recoHistoIt->second.m_betaCM_thetaCM_ion,
                         prismaInput.theta_cm / UNITS::deg, prismaInput.beta_cm);
                    Fill(recoHistoIt->second.m_BrhoMid_thetaCM_ion,
                         prismaInput.theta_cm / UNITS::deg, prismaInput.brho_mid);
                    Fill(recoHistoIt->second.m_betaCM_cosThetaCM_sinThetaCM_ion,
                         prismaInput.beta_cm * TMath::Cos(prismaInput.theta_cm),
                         prismaInput.beta_cm * TMath::Sin(prismaInput.theta_cm));
                }
            }
        }
    }

    return true;
}

void PrismaAnalysis::Instantiate()
{
    PrismaInputAnalysis::Instantiate();
    DetectorAnalysis::Instantiate();
}

void PrismaAnalysis::Finalize()
{
    if (conf.prisma.analysis_stages != PrismaConf::AnalysisStage::ALL &&
        conf.prisma.analysis_stages != PrismaConf::AnalysisStage::ANA_HIGH &&
        conf.prisma.analysis_stages != PrismaConf::AnalysisStage::HIGHLEVEL)
        PrismaInputAnalysis::Finalize();

    EntryNr                  = -1;
    conf.prisma.doneTofAlign = false;

    if (conf.prisma.analysis_stages == PrismaConf::AnalysisStage::RAWPROCESS ||
        conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANAPROCESS)
        return;

    DetectorAnalysis::Finalize();
}
