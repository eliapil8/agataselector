#include "UserSelector.h"

// ============================================================
// Allocate histos for all sub-classes

bool UserPrismaAnalysis::UserPrismaAnalysis(DetectorAnalysis::Inputs in, Prisma &p)
    : DetectorAnalysis(in, p, ConfManager::getInstance().prisma, prismaHistograms), prisma(p)
{
    if (conf.prisma.enable_treeFill)
        input.tree->Branch("Prisma", &prisma);
}
bool UserPrismaAnalysis::AllocateHistos() {}
void UserPrismaAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();
}
bool UserPrismaAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;
    PrismaHistograms &h = prismaHistograms;
}

void UserPrismaAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Prisma)
    {
        if (input.treeToRead->FindLeaf("TSPrisma") == nullptr)
            throw std::runtime_error("PRISMA enabled in the config but not found in the tree");
        prismaReader.TSPrisma = new TTreeReaderValue<ULong64_t>(fReader, "TSPrisma");

        if (input.treeToRead->FindLeaf("Analyzed.MCP_raw") == nullptr &&
            input.treeToRead->FindLeaf("MCP_raw") == nullptr)
        {
            conf.prisma.raw  = false;
            conf.prisma.misc = false;
        }

        if (input.treeToRead->FindLeaf("Analyzed.MCP_X") != nullptr)
        {
            // PrismaFilters have been used to build the tree
            if (conf.prisma.raw)
            {
                prismaReader.MCP_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.MCP_raw");
                prismaReader.PPAC_Xleft_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Xleft_raw");
                prismaReader.PPAC_Xright_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Xright_raw");
                prismaReader.PPAC_Cathode_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Cathode_raw");
                prismaReader.PPAC_Y_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Y_raw");
                prismaReader.TOF_raw    = new TTreeReaderArray<UShort_t>(fReader, "Raw.TOF_raw");
                prismaReader.IC_A_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_A_raw");
                prismaReader.IC_B_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_B_raw");
                prismaReader.IC_C_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_C_raw");
                prismaReader.IC_D_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_D_raw");
                prismaReader.IC_B_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_A_Drift_raw");
                prismaReader.IC_B_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_B_Drift_raw");
                prismaReader.IC_C_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_C_Drift_raw");
                prismaReader.Side_A_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_A_raw");
                prismaReader.Side_B_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_B_raw");
                prismaReader.Side_C_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_C_raw");
                prismaReader.Side_D_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_D_raw");
                prismaReader.Monitors_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.Monitors_raw");
                prismaReader.TAC_LT_VTS = new TTreeReaderValue<UShort_t>(fReader, "Raw.TAC_LT_VTS");
            }
            prismaReader.MONITOR_1  = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MONITOR_1");
            prismaReader.MONITOR_2  = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MONITOR_2");
            prismaReader.MCP_X      = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_X");
            prismaReader.MCP_Y      = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Y");
            prismaReader.MCP_Q      = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Q");
            prismaReader.MCP_Theta  = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Theta");
            prismaReader.MCP_Phi    = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Phi");
            prismaReader.X_FP       = new TTreeReaderValue<Float_t>(fReader, "Analyzed.X_FP");
            prismaReader.Y_FP       = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Y_FP");
            prismaReader.TOF        = new TTreeReaderValue<Float_t>(fReader, "Analyzed.TOF");
            prismaReader.IC_E       = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_E");
            prismaReader.IC_DE_A    = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_DE_A");
            prismaReader.IC_DE_AB   = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_DE_AB");
            prismaReader.IC_RANGE   = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_RANGE");
            prismaReader.IC_Drift_A = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_Drift_A");
            prismaReader.IC_Drift_B = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_Drift_B");
            prismaReader.IC_Drift_C = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_Drift_C");
            prismaReader.IC_a_numpads =
                new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_a_numpads");
            prismaReader.IC_b_numpads =
                new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_b_numpads");
            prismaReader.IC_c_numpads =
                new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_c_numpads");
            prismaReader.IC_d_numpads =
                new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_d_numpads");
            prismaReader.IC_col_a   = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_a");
            prismaReader.IC_col_b   = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_b");
            prismaReader.IC_col_c   = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_c");
            prismaReader.IC_col_d   = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_d");
            prismaReader.IC_Pads    = new TTreeReaderArray<Float_t>(fReader, "Analyzed.IC_Pads");
            prismaReader.Theta      = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Theta");
            prismaReader.Phi        = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Phi");
            prismaReader.Beta       = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Beta");
            prismaReader.Length     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Length");
            prismaReader.Radius     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Radius");
            prismaReader.RBeta      = new TTreeReaderValue<Float_t>(fReader, "Analyzed.RBeta");
            prismaReader.A_over_q   = new TTreeReaderValue<Float_t>(fReader, "Analyzed.A_over_q");
            prismaReader.Mass       = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Mass");
            prismaReader.Qvalue     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Qvalue");
            prismaReader.Theta_BP   = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Theta_BP");
            prismaReader.Phi_BP     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Phi_BP");
            prismaReader.Beta_BP    = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Beta_BP");
            prismaReader.TAC_LT_LTS = new TTreeReaderValue<Float_t>(fReader, "Analyzed.TAC_LT_LTS");
            prismaReader.Z_Nr       = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.Z_Nr");
            prismaReader.q_Nr       = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.Q_Nr");
            prismaReader.A_Nr       = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.A_Nr");
            prismaReader.MCP_OK     = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.mcp_ok");
            prismaReader.TOF_OK     = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.tof_ok");
            prismaReader.Traj_OK    = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.traj_ok");
            prismaReader.Side_OK    = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.side_ok");
            prismaReader.IC_OK      = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.ic_ok");
        }
        else
        { // femul has been used to build the tree
            prismaReader.TSPrisma = new TTreeReaderValue<ULong64_t>(fReader, "TSPrisma");
            if (conf.prisma.raw)
            {
                prismaReader.MCP_raw = new TTreeReaderArray<UShort_t>(fReader, "MCP_raw");
                prismaReader.PPAC_Xleft_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "PPAC_Xleft_raw");
                if (input.treeToRead->FindLeaf("PPAC_Xright_raw") != nullptr)
                    prismaReader.PPAC_Xright_raw =
                        new TTreeReaderArray<UShort_t>(fReader, "PPAC_Xright_raw");
                else
                    prismaReader.PPAC_Xright_raw =
                        new TTreeReaderArray<UShort_t>(fReader, "PPAC_XRight_raw");
                prismaReader.PPAC_Cathode_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "PPAC_Cathode_raw");
                prismaReader.PPAC_Y_raw = new TTreeReaderArray<UShort_t>(fReader, "PPAC_Y_raw");
                prismaReader.TOF_raw    = new TTreeReaderArray<UShort_t>(fReader, "TOF_raw");
                prismaReader.IC_A_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_A_raw");
                prismaReader.IC_B_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_B_raw");
                prismaReader.IC_C_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_C_raw");
                prismaReader.IC_D_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_D_raw");
                prismaReader.IC_A_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "IC_A_Drift_raw");
                prismaReader.IC_B_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "IC_B_Drift_raw");
                prismaReader.IC_C_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "IC_C_Drift_raw");
                prismaReader.Side_A_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_A_raw");
                prismaReader.Side_B_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_B_raw");
                prismaReader.Side_C_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_C_raw");
                prismaReader.Side_D_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_D_raw");
                prismaReader.Monitors_raw = new TTreeReaderArray<UShort_t>(fReader, "Monitors_raw");
                prismaReader.TAC_LT_VTS   = new TTreeReaderValue<UShort_t>(fReader, "TAC_LT_VTS");
            }
            prismaReader.MONITOR_1    = new TTreeReaderValue<Float_t>(fReader, "MONITOR_1");
            prismaReader.MONITOR_2    = new TTreeReaderValue<Float_t>(fReader, "MONITOR_2");
            prismaReader.MCP_X        = new TTreeReaderValue<Float_t>(fReader, "MCP_X");
            prismaReader.MCP_Y        = new TTreeReaderValue<Float_t>(fReader, "MCP_Y");
            prismaReader.MCP_Q        = new TTreeReaderValue<Float_t>(fReader, "MCP_Q");
            prismaReader.MCP_Theta    = new TTreeReaderValue<Float_t>(fReader, "MCP_Theta");
            prismaReader.MCP_Phi      = new TTreeReaderValue<Float_t>(fReader, "MCP_Phi");
            prismaReader.X_FP         = new TTreeReaderValue<Float_t>(fReader, "X_FP");
            prismaReader.Y_FP         = new TTreeReaderValue<Float_t>(fReader, "Y_FP");
            prismaReader.TOF          = new TTreeReaderValue<Float_t>(fReader, "TOF");
            prismaReader.IC_E         = new TTreeReaderValue<Float_t>(fReader, "IC_E");
            prismaReader.IC_DE_A      = new TTreeReaderValue<Float_t>(fReader, "IC_DE_A");
            prismaReader.IC_DE_AB     = new TTreeReaderValue<Float_t>(fReader, "IC_DE_AB");
            prismaReader.IC_RANGE     = new TTreeReaderValue<Float_t>(fReader, "IC_RANGE");
            prismaReader.IC_Drift_A   = new TTreeReaderValue<Float_t>(fReader, "IC_Drift_A");
            prismaReader.IC_Drift_B   = new TTreeReaderValue<Float_t>(fReader, "IC_Drift_B");
            prismaReader.IC_Drift_C   = new TTreeReaderValue<Float_t>(fReader, "IC_Drift_C");
            prismaReader.IC_a_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_a_numpads");
            prismaReader.IC_b_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_b_numpads");
            prismaReader.IC_c_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_c_numpads");
            prismaReader.IC_d_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_d_numpads");
            prismaReader.IC_col_a     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_a");
            prismaReader.IC_col_b     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_b");
            prismaReader.IC_col_c     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_c");
            prismaReader.IC_col_d     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_d");
            prismaReader.IC_Pads      = new TTreeReaderArray<Float_t>(fReader, "IC_Pads");
            prismaReader.Theta        = new TTreeReaderValue<Float_t>(fReader, "Theta");
            prismaReader.Phi          = new TTreeReaderValue<Float_t>(fReader, "Phi");
            prismaReader.Beta         = new TTreeReaderValue<Float_t>(fReader, "Beta");
            prismaReader.Length       = new TTreeReaderValue<Float_t>(fReader, "Length");
            prismaReader.Radius       = new TTreeReaderValue<Float_t>(fReader, "Radius");
            prismaReader.RBeta        = new TTreeReaderValue<Float_t>(fReader, "RBeta");
            prismaReader.A_over_q     = new TTreeReaderValue<Float_t>(fReader, "A_over_q");
            prismaReader.Mass         = new TTreeReaderValue<Float_t>(fReader, "Mass");
            prismaReader.Qvalue       = new TTreeReaderValue<Float_t>(fReader, "Qvalue");
            prismaReader.Theta_BP     = new TTreeReaderValue<Float_t>(fReader, "Theta_BP");
            if (input.treeToRead->FindLeaf("Phi_BP") != nullptr)
                prismaReader.Phi_BP = new TTreeReaderValue<Float_t>(fReader, "Phi_BP");
            prismaReader.Beta_BP    = new TTreeReaderValue<Float_t>(fReader, "Beta_BP");
            prismaReader.TAC_LT_LTS = new TTreeReaderValue<Float_t>(fReader, "TAC_LT_LTS");
            prismaReader.Z_Nr       = new TTreeReaderValue<UChar_t>(fReader, "Z_Nr");
            prismaReader.q_Nr       = new TTreeReaderValue<UChar_t>(fReader, "Q_Nr");
            prismaReader.A_Nr       = new TTreeReaderValue<UChar_t>(fReader, "A_Nr");
            prismaReader.MCP_OK     = new TTreeReaderValue<UChar_t>(fReader, "mcp_ok");
            prismaReader.TOF_OK     = new TTreeReaderValue<UChar_t>(fReader, "tof_ok");
            prismaReader.Traj_OK    = new TTreeReaderValue<UChar_t>(fReader, "traj_ok");
            prismaReader.Side_OK    = new TTreeReaderValue<UChar_t>(fReader, "side_ok");
            prismaReader.IC_OK      = new TTreeReaderValue<UChar_t>(fReader, "ic_ok");
        }
    }
}

// ============================================================

void UserSelector::SlaveBegin(TTree *treeToRead)
{
    AgataSelector::SlaveBegin(treeToRead);

    std::string sRun = fChain->GetDirectory()->GetName();
    sRun.erase(sRun.begin(), sRun.begin() + sRun.find("run_") + 4);
    sRun.erase(sRun.begin() + 4, sRun.end());

    runNr = std::stoi(sRun);

    initTS = AgataSelector::GetFirstTS();
    oldTS  = initTS;

    /*
    UserHistograms& hu = userhistograms;

    hu.dir             = outFile.mkdir("User", "User's custom analysis");
    hu.dir->cd();

    if(conf.user.create_aoqhisto) {
        // std::cout << "USER PART" << std::endl;

        hu.aoqvsxmcp.dir
            = hu.dir->mkdir("AOQvsXMCP", "Prisma A/q analyzed histograms");
        hu.aoqvsxmcp.dir->cd();

        hu.aoqvsxmcp.zGated.dir = hu.aoqvsxmcp.dir->mkdir(
            "Zed_Gated", "Prisma A/q analyzed matrices gated in Z");
        hu.aoqvsxmcp.zGated.dir->cd();

        for(int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second;
            i++) {
            hu.aoqvsxmcp.zGated.m_AoQvsXmcp.push_back(nullptr);
            Initialize<TH2D>(hu.aoqvsxmcp.zGated.m_AoQvsXmcp.back(),
                             new TH2D(Form("m_AoQ_Xmcp_%i", i),
                                      Form("A/q : x MCP (Z = %i)", i), 1000,
                                      -80, 80, 5000, 100, 500),
                             hu.aoqvsxmcp.zGated.ptrs);
        }
        // std::cout << "INIT AOQ vs XMCP" << std::endl;

        hu.aoqvsymcp.dir
            = hu.dir->mkdir("AOQvsYMCP", "Prisma A/q analyzed histograms");
        hu.aoqvsymcp.dir->cd();

        hu.aoqvsymcp.zGated.dir = hu.aoqvsymcp.dir->mkdir(
            "Zed_Gated", "Prisma A/q analyzed matrices gated in Z");
        hu.aoqvsymcp.zGated.dir->cd();

        for(int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second;
            i++) {
            hu.aoqvsymcp.zGated.m_AoQvsYmcp.push_back(nullptr);
            Initialize<TH2D>(hu.aoqvsymcp.zGated.m_AoQvsYmcp.back(),
                             new TH2D(Form("m_AoQ_Ymcp_%i", i),
                                      Form("A/q : y MCP (Z = %i)", i), 1000,
                                      -80, 80, 5000, 100, 500),
                             hu.aoqvsymcp.zGated.ptrs);
        }

        // std::cout << "INIT AOQ vs YMCP" << std::endl;

        hu.aoqvsxfp.dir
            = hu.dir->mkdir("AOQvsXFP", "Prisma A/q analyzed histograms");
        hu.aoqvsxfp.dir->cd();

        hu.aoqvsxfp.zGated.dir = hu.aoqvsxfp.dir->mkdir(
            "Zed_Gated", "Prisma A/q analyzed matrices gated in Z");
        hu.aoqvsxfp.zGated.dir->cd();

        for(int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second;
            i++) {
            hu.aoqvsxfp.zGated.m_AoQvsXfp.push_back(nullptr);
            Initialize<TH2D>(hu.aoqvsxfp.zGated.m_AoQvsXfp.back(),
                             new TH2D(Form("m_AoQ_Xfp_%i", i),
                                      Form("A/q : x FP (Z = %i)", i), 1200,
                                      -100, 1100, 5000, 100, 500),
                             hu.aoqvsxfp.zGated.ptrs);
        }
    }

    if(conf.user.create_agatarunhisto || conf.user.create_prismarunhisto
       || conf.user.create_agataprismarunhisto) {
        UserHistograms::histoRuns& hR = hu.historuns;
        hR.dir = hu.dir->mkdir("HistoRuns", "Histos over the experiment runs");
        hR.dir->cd();

        if(conf.user.create_agatarunhisto){
            if(conf.detectorsPresent.Agata){
                hR.hagataruns.dir = hR.dir->mkdir("AgataRuns", "Agata histos
    over the experiment runs"); hR.hagataruns.dir->cd();
                Initialize<TH2D>(hR.hagataruns.m_EnoDC_vs_runNr,new
    TH2D(Form("m_EnoDC_vs_runNr"),Form("m_EnoDC_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 4000, 0, 4000 ),hR.hagataruns.ptrs);
                Initialize<TH2D>(hR.hagataruns.m_CR_vs_runNr,new
    TH2D(Form("m_CR_vs_runNr"),Form("m_CR_vs_runNr;Run Nr;"), 85, -0.5, 84.5 ,
    1000, 0 ,100000),hR.hagataruns.ptrs); Initialize<TH1D>(
                    hR.hagataruns.h_trackE,
                    new TH1D("h_trackE",
                            Form("gamma tracked;Energy [keV];Counts/%d keV",
                                conf.agata.binWidth),
                            static_cast<int>(conf.agata.gammaEmax / UNITS::keV
                                            / conf.agata.binWidth),
                            0, conf.agata.gammaEmax / UNITS::keV),
                    hR.hagataruns.ptrs);
            }
        }
        if(conf.user.create_prismarunhisto){
            if(conf.detectorsPresent.Prisma){
                hR.hprismaruns.dir= hR.dir->mkdir("PrismaRuns", "Prisma histos
    over the experiment runs"); hR.hprismaruns.dir->cd();
                Initialize<TH2D>(hR.hprismaruns.m_MCP_X_vs_runNr, new
    TH2D(Form("m_MCP_X_vs_runNr"), Form("m_MCP_X_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1000, -100, 100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_MCP_Y_vs_runNr, new
    TH2D(Form("m_MCP_Y_vs_runNr"), Form("m_MCP_Y_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1000, -100, 100), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_MCP_Theta_vs_runNr, new
    TH2D(Form("m_MCP_Theta_vs_runNr"), Form("m_MCP_Theta_vs_runNr;Run Nr;"),
    85, -0.5, 84.5,  1000, -50, 50), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_MCP_Phi_vs_runNr, new
    TH2D(Form("m_MCP_Phi_vs_runNr"), Form("m_MCP_Phi_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1200, -100, 1100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_X_FP_vs_runNr, new
    TH2D(Form("m_X_FP_vs_runNr"), Form("m_X_FP_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1200, -100, 1100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Y_FP_vs_runNr, new
    TH2D(Form("m_Y_FP_vs_runNr"), Form("m_Y_FP_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  400, -100, 100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_TOF_vs_runNr, new
    TH2D(Form("m_TOF_vs_runNr"), Form("m_TOF_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 500, 0, 500), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_IC_E_vs_runNr, new
    TH2D(Form("m_IC_E_vs_runNr"), Form("m_IC_E_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 2000, 0, 10000), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_IC_DE_A_vs_runNr, new
    TH2D(Form("m_IC_DE_A_vs_runNr"), Form("m_IC_DE_A_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 2000, 0, 6000), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_IC_DE_AB_vs_runNr, new
    TH2D(Form("m_IC_DE_AB_vs_runNr"), Form("m_IC_DE_AB_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 2000, 0, 6000), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_IC_RANGE_vs_runNr, new
    TH2D(Form("m_IC_RANGE_vs_runNr"), Form("m_IC_RANGE_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 100, 0, 1000), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_IC_Drift_A_vs_runNr, new
    TH2D(Form("m_IC_Drift_A_vs_runNr"), Form("m_IC_Drift_A_vs_runNr;Run Nr;"),
    85, -0.5, 84.5,  4000, 0, 4000), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_IC_Drift_B_vs_runNr, new
    TH2D(Form("m_IC_Drift_B_vs_runNr"), Form("m_IC_Drift_B_vs_runNr;Run Nr;"),
    85, -0.5, 84.5,  4000, 0, 4000), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_IC_Drift_C_vs_runNr, new
    TH2D(Form("m_IC_Drift_C_vs_runNr"), Form("m_IC_Drift_C_vs_runNr;Run Nr;"),
    85, -0.5, 84.5,  4000, 0, 4000), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Beta_vs_runNr, new
    TH2D(Form("m_Beta_vs_runNr"), Form("m_Beta_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  500, 0, 0.3), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Radius_vs_runNr, new
    TH2D(Form("m_Radius_vs_runNr"), Form("m_Radius_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 1000, 500, 1700), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_RBeta_vs_runNr, new
    TH2D(Form("m_RBeta_vs_runNr"), Form("m_RBeta_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 1000, 1000, 6000), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_A_over_q_vs_runNr, new
    TH2D(Form("m_A_over_q_vs_runNr"), Form("m_A_over_q_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 1000, 100, 500), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Mass_vs_runNr, new
    TH2D(Form("m_Mass_vs_runNr"), Form("m_Mass_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 10000, 50, 150), hR.hprismaruns.ptrs);
                //Initialize<TH2D>(hR.hprismaruns.m_Qvalue_vs_runNr, new
    TH2D(Form("m_Qvalue_vs_runNr"), Form("m_Qvalue_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 1000, 0, 12000), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Z_Nr_vs_runNr, new
    TH2D(Form("m_Z_Nr_vs_runNr"), Form("m_Z_Nr_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 30, 20.5, 50.5), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Q_Nr_vs_runNr, new
    TH2D(Form("m_Q_Nr_vs_runNr"), Form("m_Q_Nr_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 40, 18.5, 58.5), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_A_Nr_vs_runNr, new
    TH2D(Form("m_A_Nr_vs_runNr"), Form("m_A_Nr_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5, 50, 60.5, 110.5), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Mass_z36_vs_runNr, new
    TH2D(Form("m_Mass_z36_vs_runNr"), Form("m_Mass_z36_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  5000, 60, 100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Mass_z34_vs_runNr, new
    TH2D(Form("m_Mass_z34_vs_runNr"), Form("m_Mass_z34_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  5000, 60, 100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Mass_z32_vs_runNr, new
    TH2D(Form("m_Mass_z32_vs_runNr"), Form("m_Mass_z32_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  5000, 60, 100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_Mass_z30_vs_runNr, new
    TH2D(Form("m_Mass_z30_vs_runNr"), Form("m_Mass_z30_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  5000, 60, 100), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_TOF_z36_vs_runNr, new
    TH2D(Form("m_TOF_z36_vs_runNr"), Form("m_TOF_z36_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1000, 100, 350), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_TOF_z34_vs_runNr, new
    TH2D(Form("m_TOF_z34_vs_runNr"), Form("m_TOF_z34_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1000, 100, 350), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_TOF_z32_vs_runNr, new
    TH2D(Form("m_TOF_z32_vs_runNr"), Form("m_TOF_z32_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1000, 100, 350), hR.hprismaruns.ptrs);
                Initialize<TH2D>(hR.hprismaruns.m_TOF_z30_vs_runNr, new
    TH2D(Form("m_TOF_z30_vs_runNr"), Form("m_TOF_z30_vs_runNr;Run Nr;"), 85,
    -0.5, 84.5,  1000, 100, 350), hR.hprismaruns.ptrs);
            }
        }
        if(conf.user.create_agataprismarunhisto){
            if(conf.detectorsPresent.Agata&&conf.detectorsPresent.Prisma){
                hR.hagataprismaruns.dir = hR.dir->mkdir("AgataPrismaRuns",
    "Agata-Prisma histos over the experiment runs");
                hR.hagataprismaruns.dir->cd();
                Initialize<TH2D>(
                    hR.hagataprismaruns.m_Tdiff_PrismaAgata_vs_runNr,
                    new TH2D(Form("m_Tdiff_PrismaAgata_vs_runNr"),
                             Form("m_Tdiff_PrismaAgata_vs_runNr; Run Nr; T "
                                  "diff (10 ns);"),
                             85, -0.5, 84.5, 3000, -200, 100),
                    hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_86Kr_vs_runNr,
                                 new TH2D(Form("m_EDC_86Kr_vs_runNr"),
                                          Form("m_EDC_86Kr_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_88Kr_vs_runNr,
                                 new TH2D(Form("m_EDC_88Kr_vs_runNr"),
                                          Form("m_EDC_88Kr_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_84Se_vs_runNr,
                                 new TH2D(Form("m_EDC_84Se_vs_runNr"),
                                          Form("m_EDC_84Se_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_90Se_vs_runNr,
                                 new TH2D(Form("m_EDC_90Se_vs_runNr"),
                                          Form("m_EDC_90Se_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_78Ge_vs_runNr,
                                 new TH2D(Form("m_EDC_78Ge_vs_runNr"),
                                          Form("m_EDC_78Ge_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_82Ge_vs_runNr,
                                 new TH2D(Form("m_EDC_82Ge_vs_runNr"),
                                          Form("m_EDC_82Ge_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_76Zn_vs_runNr,
                                 new TH2D(Form("m_EDC_76Zn_vs_runNr"),
                                          Form("m_EDC_76Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_78Zn_vs_runNr,
                                 new TH2D(Form("m_EDC_78Zn_vs_runNr"),
                                          Form("m_EDC_78Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_79Zn_vs_runNr,
                                 new TH2D(Form("m_EDC_79Zn_vs_runNr"),
                                          Form("m_EDC_79Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
                Initialize<TH2D>(hR.hagataprismaruns.m_EDC_80Zn_vs_runNr,
                                 new TH2D(Form("m_EDC_80Zn_vs_runNr"),
                                          Form("m_EDC_80Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000),
                                 hR.hagataprismaruns.ptrs);
            }
        }
    }
    if(conf.user.create_agatatimehisto){
        UserHistograms::histoTime& hT = hu.histotime;
        hT.dir  = hu.dir->mkdir("HistoTime", "Histos over time");
        hT.dir  ->cd();
        UserHistograms::histoTime::AgataTime& hAT = hT.hagatatime;
        hAT.dir  = hT.dir->mkdir("HistoAgataTime", "Agata histos over time");
        hAT.dir  ->cd();

        Initialize<TH1D>(hAT.h_CR, new TH1D(Form("h_CR"), Form("h_CR;Time
    (s);Counts / 10s (keV)"),  21*360, 0, 21*3600), hAT.ptrs);

        Initialize<TH2D>(hAT.m_EnoDC_vs_TS, new TH2D(Form("m_EnoDC_vs_TS"),
    Form("m_EnoDC_vs_TS;Time (s);Energy (keV)"),  21*60, 0, 21*3600,  4000, 0,
    4000), hAT.ptrs); Initialize<TH2D>(hAT.m_CrysId_vs_TS, new
    TH2D(Form("m_CrysId_vs_TS"), Form("m_CrysId_vs_TS;Time (s);Crystal ID"),
    21*60, 0, 21*3600,  45, -0.5, 44.5), hAT.ptrs);

        Initialize<TH2D>(hAT.m_trackE_00A_vs_TS, new
    TH2D(Form("m_trackE_00A_vs_TS"), Form("m_trackE_00A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_00B_vs_TS, new
    TH2D(Form("m_trackE_00B_vs_TS"), Form("m_trackE_00B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_00C_vs_TS, new
    TH2D(Form("m_trackE_00C_vs_TS"), Form("m_trackE_00C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_01A_vs_TS, new
    TH2D(Form("m_trackE_01A_vs_TS"), Form("m_trackE_01A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_01B_vs_TS, new
    TH2D(Form("m_trackE_01B_vs_TS"), Form("m_trackE_01B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_01C_vs_TS, new
    TH2D(Form("m_trackE_01C_vs_TS"), Form("m_trackE_01C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_02A_vs_TS, new
    TH2D(Form("m_trackE_02A_vs_TS"), Form("m_trackE_02A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_02B_vs_TS, new
    TH2D(Form("m_trackE_02B_vs_TS"), Form("m_trackE_02B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_02C_vs_TS, new
    TH2D(Form("m_trackE_02C_vs_TS"), Form("m_trackE_02C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_04A_vs_TS, new
    TH2D(Form("m_trackE_04A_vs_TS"), Form("m_trackE_04A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_04B_vs_TS, new
    TH2D(Form("m_trackE_04B_vs_TS"), Form("m_trackE_04B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_04C_vs_TS, new
    TH2D(Form("m_trackE_04C_vs_TS"), Form("m_trackE_04C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_05A_vs_TS, new
    TH2D(Form("m_trackE_05A_vs_TS"), Form("m_trackE_05A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_05B_vs_TS, new
    TH2D(Form("m_trackE_05B_vs_TS"), Form("m_trackE_05B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_05C_vs_TS, new
    TH2D(Form("m_trackE_05C_vs_TS"), Form("m_trackE_05C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_06A_vs_TS, new
    TH2D(Form("m_trackE_06A_vs_TS"), Form("m_trackE_06A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_06B_vs_TS, new
    TH2D(Form("m_trackE_06B_vs_TS"), Form("m_trackE_06B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_06C_vs_TS, new
    TH2D(Form("m_trackE_06C_vs_TS"), Form("m_trackE_06C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_07A_vs_TS, new
    TH2D(Form("m_trackE_07A_vs_TS"), Form("m_trackE_07A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_07B_vs_TS, new
    TH2D(Form("m_trackE_07B_vs_TS"), Form("m_trackE_07B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_07C_vs_TS, new
    TH2D(Form("m_trackE_07C_vs_TS"), Form("m_trackE_07C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_08A_vs_TS, new
    TH2D(Form("m_trackE_08A_vs_TS"), Form("m_trackE_08A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_08B_vs_TS, new
    TH2D(Form("m_trackE_08B_vs_TS"), Form("m_trackE_08B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_08C_vs_TS, new
    TH2D(Form("m_trackE_08C_vs_TS"), Form("m_trackE_08C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_09A_vs_TS, new
    TH2D(Form("m_trackE_09A_vs_TS"), Form("m_trackE_09A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_09B_vs_TS, new
    TH2D(Form("m_trackE_09B_vs_TS"), Form("m_trackE_09B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_09C_vs_TS, new
    TH2D(Form("m_trackE_09C_vs_TS"), Form("m_trackE_09C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_10A_vs_TS, new
    TH2D(Form("m_trackE_10A_vs_TS"), Form("m_trackE_10A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_10B_vs_TS, new
    TH2D(Form("m_trackE_10B_vs_TS"), Form("m_trackE_10B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_10C_vs_TS, new
    TH2D(Form("m_trackE_10C_vs_TS"), Form("m_trackE_10C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_11A_vs_TS, new
    TH2D(Form("m_trackE_11A_vs_TS"), Form("m_trackE_11A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_11B_vs_TS, new
    TH2D(Form("m_trackE_11B_vs_TS"), Form("m_trackE_11B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_11C_vs_TS, new
    TH2D(Form("m_trackE_11C_vs_TS"), Form("m_trackE_11C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_13A_vs_TS, new
    TH2D(Form("m_trackE_13A_vs_TS"), Form("m_trackE_13A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_13B_vs_TS, new
    TH2D(Form("m_trackE_13B_vs_TS"), Form("m_trackE_13B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_13C_vs_TS, new
    TH2D(Form("m_trackE_13C_vs_TS"), Form("m_trackE_13C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_14A_vs_TS, new
    TH2D(Form("m_trackE_14A_vs_TS"), Form("m_trackE_14A_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_14B_vs_TS, new
    TH2D(Form("m_trackE_14B_vs_TS"), Form("m_trackE_14B_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);
        Initialize<TH2D>(hAT.m_trackE_14C_vs_TS, new
    TH2D(Form("m_trackE_14C_vs_TS"), Form("m_trackE_14C_vs_TS;Time (s);Energy
    (keV)"),  21*60, 0, 21*3600,  4000, 0, 4000), hAT.ptrs);

        hAT.m_trackE_Crys_vs_TS[0]=hAT.m_trackE_00A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[1]=hAT.m_trackE_00B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[2]=hAT.m_trackE_00C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[3]=hAT.m_trackE_01A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[4]=hAT.m_trackE_01B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[5]=hAT.m_trackE_01C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[6]=hAT.m_trackE_02A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[7]=hAT.m_trackE_02B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[8]=hAT.m_trackE_02C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[12]=hAT.m_trackE_04A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[13]=hAT.m_trackE_04B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[14]=hAT.m_trackE_04C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[15]=hAT.m_trackE_05A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[16]=hAT.m_trackE_05B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[17]=hAT.m_trackE_05C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[18]=hAT.m_trackE_06A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[19]=hAT.m_trackE_06B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[20]=hAT.m_trackE_06C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[21]=hAT.m_trackE_07A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[22]=hAT.m_trackE_07B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[23]=hAT.m_trackE_07C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[24]=hAT.m_trackE_08A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[25]=hAT.m_trackE_08B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[26]=hAT.m_trackE_08C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[27]=hAT.m_trackE_09A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[28]=hAT.m_trackE_09B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[29]=hAT.m_trackE_09C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[30]=hAT.m_trackE_10A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[31]=hAT.m_trackE_10B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[32]=hAT.m_trackE_10C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[33]=hAT.m_trackE_11A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[34]=hAT.m_trackE_11B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[35]=hAT.m_trackE_11C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[39]=hAT.m_trackE_13A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[40]=hAT.m_trackE_13B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[41]=hAT.m_trackE_13C_vs_TS;
        hAT.m_trackE_Crys_vs_TS[42]=hAT.m_trackE_14A_vs_TS;
        hAT.m_trackE_Crys_vs_TS[43]=hAT.m_trackE_14B_vs_TS;
        hAT.m_trackE_Crys_vs_TS[44]=hAT.m_trackE_14C_vs_TS;

    }
    if(conf.user.create_mixhisto){
        UserHistograms::histoMix& hM = hu.histomix;
        hM.dir  = hu.dir->mkdir("HistoMix", "Mixed histograms");
        hM.dir  ->cd();
        Initialize<TH2D>(hM.m_EnoDC_vs_coreID, new
    TH2D(Form("m_EnoDC_vs_coreID"), Form("m_EnoDC_vs_coreID;Core ID;Energy
    (keV)"), 45, -0.5, 44.5,  4000, 0, 4000), hM.ptrs);
    Initialize<TH1D>(hM.h_Z, new TH1D(Form("h_Z"), Form("h_Z;Z (a.u.);Counts"),
    7000,3000,10000), hM.ptrs); for(auto &ion :
    conf.reaction.fragmentsOfInterest){ UserHistograms::histoMix::IonHistograms
    ion_histos; ion_histos.dir = hM.dir->GetDirectory(Form("Z%d_mix",
    ion.first.Z)); if(ion_histos.dir == nullptr) { ion_histos.dir =
    hM.dir->mkdir( Form("Z%d_mix", ion.first.Z), Form("Gamma spectra with cuts
    on the identified ion: Z=%d", ion.first.Z));
            }
            TDirectory *tmpDir = ion_histos.dir->GetDirectory(Form("A%d_mix",
    ion.first.A)); if(tmpDir){ throw std::runtime_error(Form("A%d directory in
    Z%d already exists. Duplicate ion?", ion.first.A, ion.first.Z));
            }
            ion_histos.dir = ion_histos.dir->mkdir(
                Form("A%d_mix", ion.first.A),
                Form("Gamma spectra with cuts on the identified ion: A=%d",
                    ion.first.A));
            ion_histos.dir->cd();

            Initialize<TH1D>(ion_histos.h_ion_EnoDC_high_energy, new
    TH1D(Form("h_ion_EnoDC_high_energy"), Form("h_ion_EnoDC_high_energy;Energy
    (keV); Counts"), 15000, 0, 15000), ion_histos.ptrs);
            Initialize<TH1D>(ion_histos.h_ion_EDC_high_energy, new
    TH1D(Form("h_ion_EDC_high_energy"), Form("h_ion_EDC_high_energy;Energy
    (keV); Counts"), 15000, 0, 15000), ion_histos.ptrs);
            Initialize<TH2D>(ion_histos.m_ion_EDC_vs_coreID, new
    TH2D(Form("m_ion_EDC_vs_coreID"), Form("m_ion_EDC_vs_coreID;Energy (keV);
    Counts"), 45, -0.5, 44.5, 4000, 0, 4000), ion_histos.ptrs);
            Initialize<TH2D>(ion_histos.m_ion_EnoDC_vs_coreID, new
    TH2D(Form("m_ion_EnoDC_vs_coreID"), Form("m_ion_EnoDC_vs_coreID;Energy
    (keV); Counts"), 45, -0.5, 44.5, 4000, 0, 4000), ion_histos.ptrs);
            if(ion.A==94&&ion.Z==38){
                Initialize<TH2D>(ion_histos.m_ion_EDC_vs_Beta, new
    TH1D(Form("h_ion_EDC_vs_Beta"), Form("h_ion_EDC_vs_Beta;Energy (keV);
    Counts"), 100, -50, 50, 500, 0, 1500), ion_histos.ptrs);
                Initialize<TH2D>(ion_histos.m_ion_EDC_vs_AGATA_zShift, new
    TH1D(Form("h_ion_EDC_vs_AGATA_zShift"), Form("h_ion_EDC_vs_zShift;Energy
    (keV); Counts"), 100, -20, 20, 500, 0, 1500), ion_histos.ptrs);
                Initialize<TH2D>(ion_histos.m_ion_EDC_vs_AGATA_phi, new
    TH1D(Form("h_ion_EDC_vs_AGATA_phi"), Form("h_ion_EDC_vs_AGATA_phi;Energy
    (keV); Counts"), 100, -10, 10, 500, 0, 1500), ion_histos.ptrs);
                Initialize<TH2D>(ion_histos.m_ion_EDC_vs_MCP_zShift, new
    TH1D(Form("h_ion_EDC_vs_MCP_zShift"), Form("h_ion_EDC_vs_MCP_zShift;Energy
    (keV); Counts"), 100, -20, 20, 500, 0, 1500), ion_histos.ptrs);
                Initialize<TH2D>(ion_histos.m_ion_EDC_vs_MCP_xScale, new
    TH1D(Form("h_ion_EDC_vs_MCP_xScale"), Form("h_ion_EDC_vs_MCP_xScale;Energy
    (keV); Counts"), 100, 0.5, 2, 500, 0, 1500), ion_histos.ptrs);
                Initialize<TH2D>(ion_histos.m_ion_EDC_vs_MCP_yScale, new
    TH1D(Form("h_ion_EDC_vs_MCP_yScale"), Form("h_ion_EDC_vs_MCP_yScale;Energy
    (keV); Counts"), 100, 0.5, 2, 500, 0, 1500), ion_histos.ptrs);
            }

            hM.ionHistos[ion.first] = ion_histos;


        }
    }
    */
}

Bool_t UserSelector::Process(Long64_t entry)
{
    AgataSelector::Process(entry);

    /*
    UserHistograms& hu = userhistograms;
    UserHistograms::histoRuns& hR = hu.historuns;
    UserHistograms::histoTime::AgataTime& hAT = hu.histotime.hagatatime;
    UserHistograms::histoMix& hM = hu.histomix;

    //if(conf.user.create_aoqhisto){

    UserHistograms&            hu = userhistograms;
    UserHistograms::histoRuns& hR = hu.historuns;


    if(conf.user.create_aoqhisto) {
        if(!prismaOutput.all_ok) return kTRUE;

        for(unsigned int i = 0; i < hu.aoqvsxmcp.zGated.m_AoQvsXmcp.size();
            i++) {
            unsigned int z;
            z = i + conf.prisma.Zrange.first;
            if(prismaOutput.Z == z) {
                if(!hu.aoqvsxmcp.zGated.m_AoQvsXmcp.empty())
                    Fill(hu.aoqvsxmcp.zGated.m_AoQvsXmcp.at(i),
                         **prismaReader.MCP_X, **prismaReader.A_over_q);
            }
        }

        for(unsigned int i = 0; i < hu.aoqvsymcp.zGated.m_AoQvsYmcp.size();
            i++) {
            unsigned int z;
            z = i + conf.prisma.Zrange.first;
            if(prismaOutput.Z == z) {
                if(!hu.aoqvsymcp.zGated.m_AoQvsYmcp.empty())
                    Fill(hu.aoqvsymcp.zGated.m_AoQvsYmcp.at(i),
                         **prismaReader.MCP_Y, **prismaReader.A_over_q);
            }
        }

        for(unsigned int i = 0; i < hu.aoqvsxfp.zGated.m_AoQvsXfp.size();
            i++) {
            unsigned int z;
            z = i + conf.prisma.Zrange.first;
            if(prismaOutput.Z == z) {
                if(!hu.aoqvsxfp.zGated.m_AoQvsXfp.empty())
                    Fill(hu.aoqvsxfp.zGated.m_AoQvsXfp.at(i),
                         **prismaReader.X_FP, **prismaReader.A_over_q);
            }
        }
    }
    if(conf.user.create_agatarunhisto || conf.user.create_prismarunhisto ||
conf.user.create_agataprismarunhisto||conf.user.create_mixhisto){

        if(conf.user.create_agatarunhisto){

            if(conf.detectorsPresent.Agata){
                for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
                    Fill(hR.hagataruns.m_EnoDC_vs_runNr, runNr,
agataOutput.E.at(i)); Fill(hR.hagataruns.m_CR_vs_runNr, runNr,
unsignedDiff<double, unsigned long long>(agataOutput.TS,initTS)/1.e8);
                    Fill(hR.hagataruns.h_trackE, agataReader.trackE->At(i));
                }
            }
        }
        if(conf.user.create_prismarunhisto) {
            if(conf.detectorsPresent.Prisma) {
                if(!prismaOutput.all_ok) return kTRUE;
                Fill(hR.hprismaruns.m_MCP_X_vs_runNr, runNr,
                     **prismaReader.MCP_X);
                Fill(hR.hprismaruns.m_MCP_Y_vs_runNr, runNr,
                     **prismaReader.MCP_Y);
                // Fill(hR.hprismaruns.m_MCP_Theta_vs_runNr,
                // runNr,**prismaReader.MCP_Theta);
                // Fill(hR.hprismaruns.m_MCP_Phi_vs_runNr,
                // runNr,**prismaReader.MCP_Phi);
                Fill(hR.hprismaruns.m_X_FP_vs_runNr, runNr,
                     **prismaReader.X_FP);
                Fill(hR.hprismaruns.m_Y_FP_vs_runNr, runNr,
                     **prismaReader.Y_FP);
                Fill(hR.hprismaruns.m_TOF_vs_runNr, runNr, **prismaReader.TOF);
                Fill(hR.hprismaruns.m_IC_E_vs_runNr, runNr,
                     **prismaReader.IC_E);
                // Fill(hR.hprismaruns.m_IC_DE_A_vs_runNr,
                // runNr,**prismaReader.IC_DE_A);
                // Fill(hR.hprismaruns.m_IC_DE_AB_vs_runNr,
                // runNr,**prismaReader.IC_DE_AB);
                // Fill(hR.hprismaruns.m_IC_RANGE_vs_runNr,
                // runNr,**prismaReader.IC_RANGE);
                // Fill(hR.hprismaruns.m_IC_Drift_A_vs_runNr,
                // runNr,**prismaReader.IC_Drift_A);
                // Fill(hR.hprismaruns.m_IC_Drift_B_vs_runNr,
                // runNr,**prismaReader.IC_Drift_B);
                // Fill(hR.hprismaruns.m_IC_Drift_C_vs_runNr,
                // runNr,**prismaReader.IC_Drift_C);
                Fill(hR.hprismaruns.m_Beta_vs_runNr, runNr,
                     **prismaReader.Beta);
                Fill(hR.hprismaruns.m_Radius_vs_runNr, runNr,
                     **prismaReader.Radius);
                Fill(hR.hprismaruns.m_RBeta_vs_runNr, runNr,
                     **prismaReader.RBeta);
                Fill(hR.hprismaruns.m_A_over_q_vs_runNr, runNr,
                     **prismaReader.A_over_q);
                Fill(hR.hprismaruns.m_Mass_vs_runNr, runNr,
                     **prismaReader.Mass);
                // Fill(hR.hprismaruns.m_Qvalue_vs_runNr,
                // runNr,**prismaReader.Qvalue);
                Fill(hR.hprismaruns.m_Z_Nr_vs_runNr, runNr,
                     **prismaReader.Z_Nr);
                Fill(hR.hprismaruns.m_Q_Nr_vs_runNr, runNr,
                     **prismaReader.q_Nr);
                Fill(hR.hprismaruns.m_A_Nr_vs_runNr, runNr,
                     **prismaReader.A_Nr);

                if(prismaOutput.Z == 36) {
                    Fill(hR.hprismaruns.m_Mass_z36_vs_runNr, runNr,
                         **prismaReader.Mass);
                    Fill(hR.hprismaruns.m_TOF_z36_vs_runNr, runNr,
                         **prismaReader.TOF);
                }
                if(prismaOutput.Z == 34) {
                    Fill(hR.hprismaruns.m_Mass_z34_vs_runNr, runNr,
                         **prismaReader.Mass);
                    Fill(hR.hprismaruns.m_TOF_z34_vs_runNr, runNr,
                         **prismaReader.TOF);
                }
                if(prismaOutput.Z == 32) {
                    Fill(hR.hprismaruns.m_Mass_z32_vs_runNr, runNr,
                         **prismaReader.Mass);
                    Fill(hR.hprismaruns.m_TOF_z32_vs_runNr, runNr,
                         **prismaReader.TOF);
                }
                if(prismaOutput.Z == 30) {
                    Fill(hR.hprismaruns.m_Mass_z30_vs_runNr, runNr,
                         **prismaReader.Mass);
                    Fill(hR.hprismaruns.m_TOF_z30_vs_runNr, runNr,
                         **prismaReader.TOF);
                }
            }
        }
        if(conf.user.create_agataprismarunhisto || conf.user.create_mixhisto){
            if(prismaOutput.is_agata_coincidence){
                for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
                    if(!agataOutput.prisma_coinc.at(i)) continue;



                    std::vector<TLorentzVector> gammaDC;
                    TLorentzVector              p3;
                    TVector3                    beta3;
                    gammaDC.push_back(agataOutput.P.at(i));
                    if(prismaOutput.p3.Mag() != 0) {
                        ReactionFragment::Ion ion{prismaOutput.A,
                                                  prismaOutput.Z};

                        p3    = SlowInTarget(prismaOutput.p3,
                                             conf.agataPrisma.dcPos, ion);
                        beta3 = p3.BoostVector();

                        gammaDC.back().Boost(-p3.BoostVector());
                    } else {
                        if(prismaOutput.beta.Mag() > 0) {
                            // Fragment has beta and either is not identified
                            // or no energy losses are calculated
                            beta3 = prismaOutput.beta;
                            gammaDC.back().Boost(-prismaOutput.beta);
                        } else {
                            // Fragment has no beta, take the average beta
                            beta3 = conf.prisma.betaAvg;
                            gammaDC.back().Boost(-conf.prisma.betaAvg);
                        }
                    }
                    if(conf.user.create_agataprismarunhisto){
                        auto diff = unsignedDiff<double, unsigned long
long>(prismaOutput.TS, agataOutput.TS)+(prismaOutput.Time -
agataOutput.Time.at(i))/(10. * UNITS::ns);
                        Fill(hR.hagataprismaruns.m_Tdiff_PrismaAgata_vs_runNr,
runNr, diff); if(prismaOutput.Z==36){ if(prismaOutput.A==88){
                                Fill(hR.hagataprismaruns.m_EDC_88Kr_vs_runNr,  1.0*runNr,
gammaDC.back().E());
                            }
                            else if(prismaOutput.A==86){
                                Fill(hR.hagataprismaruns.m_EDC_86Kr_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                        }
                        else if(prismaOutput.Z==34){
                            if(prismaOutput.A==84){
                                Fill(hR.hagataprismaruns.m_EDC_84Se_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                            else if(prismaOutput.A==90){
                                Fill(hR.hagataprismaruns.m_EDC_90Se_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                        }
                        else if(prismaOutput.Z==32){
                            if(prismaOutput.A==78){
                                Fill(hR.hagataprismaruns.m_EDC_78Ge_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                            else if(prismaOutput.A==82){
                                Fill(hR.hagataprismaruns.m_EDC_82Ge_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                        }
                        else if(prismaOutput.Z==30){
                            if(prismaOutput.A==76){
                                Fill(hR.hagataprismaruns.m_EDC_76Zn_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                            else if(prismaOutput.A==78){
                                Fill(hR.hagataprismaruns.m_EDC_78Zn_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                            else if(prismaOutput.A==79){
                                Fill(hR.hagataprismaruns.m_EDC_79Zn_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                            else if(prismaOutput.A==80){
                                Fill(hR.hagataprismaruns.m_EDC_80Zn_vs_runNr, 1.0*runNr,
gammaDC.back().E());
                            }
                        }
                    }
                    if(conf.user.create_mixhisto){
                        ReactionFragment::Ion detectedIon{prismaOutput.A,
prismaOutput.Z}; auto    histoIt = hM.ionHistos.find(detectedIon); if(histoIt
!= hM.ionHistos.end()) { Fill(histoIt->second.h_ion_EDC_high_energy,
gammaDC.back().E()); Fill(histoIt->second.h_ion_EnoDC_high_energy,
agataOutput.E.at(i)); Fill(histoIt->second.m_ion_EDC_vs_coreID,
agataOutput.CoreID.at(i), gammaDC.back().E());
                            Fill(histoIt->second.m_ion_EnoDC_vs_coreID,
agataOutput.CoreID.at(i), agataOutput.E.at(i));
                            if(detectedIon.A==94&&detectedIon.Z==38){
                                for(int nBeta = -50; nBeta < 50; ++nBeta) {
                                    TLorentzVector ptemp    =
agataOutput.P.at(i); TVector3       betatemp = prismaOutput.beta;
                                    betatemp.SetMag(prismaOutput.beta.Mag() +
nBeta * 0.0001); ptemp.Boost(-betatemp);
Fill(histoIt->second.m_ion_EDC_vs_Beta, (nBeta + 0.5) * 0.0002, ptemp.E());
                                }

                                Initialize<TH2D>(ion_histos.h_ion_EDC_vs_Beta,
new TH1D(Form("h_ion_EDC_vs_Beta"), Form("h_ion_EDC_vs_Beta;Energy (keV);
Counts"), 100, -50, 50, 500, 0, 1500), ion_histos.ptrs);
                                Initialize<TH2D>(ion_histos.h_ion_EDC_vs_AGATA_zShift,
new TH1D(Form("h_ion_EDC_vs_AGATA_zShift"), Form("h_ion_EDC_vs_zShift;Energy
(keV); Counts"), 100, -20, 20, 500, 0, 1500), ion_histos.ptrs);
                                Initialize<TH2D>(ion_histos.h_ion_EDC_vs_AGATA_phi,
new TH1D(Form("h_ion_EDC_vs_AGATA_phi"), Form("h_ion_EDC_vs_AGATA_phi;Energy
(keV); Counts"), 100, -10, 10, 500, 0, 1500), ion_histos.ptrs);
                                Initialize<TH2D>(ion_histos.h_ion_EDC_vs_MCP_zShift,
new TH1D(Form("h_ion_EDC_vs_MCP_zShift"), Form("h_ion_EDC_vs_MCP_zShift;Energy
(keV); Counts"), 100, -20, 20, 500, 0, 1500), ion_histos.ptrs);
                                Initialize<TH2D>(ion_histos.h_ion_EDC_vs_MCP_xScale,
new TH1D(Form("h_ion_EDC_vs_MCP_xScale"), Form("h_ion_EDC_vs_MCP_xScale;Energy
(keV); Counts"), 100, 0.5, 2, 500, 0, 1500), ion_histos.ptrs);
                                Initialize<TH2D>(ion_histos.h_ion_EDC_vs_MCP_yScale,
new TH1D(Form("h_ion_EDC_vs_MCP_yScale"), Form("h_ion_EDC_vs_MCP_yScale;Energy
(keV); Counts"), 100, 0.5, 2, 500, 0, 1500), ion_histos.ptrs);
                            }
                        }
                    }
                }
            }
        }
    }
    if(conf.user.create_agatatimehisto){
        if(conf.detectorsPresent.Agata){
            for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
                double time = unsignedDiff<double, unsigned long
long>(agataOutput.TS,initTS)/1e8; Fill(hAT.h_CR, time); Fill(hAT.m_EnoDC_vs_TS,
time, agataOutput.E.at(i)); Fill(hAT.m_CrysId_vs_TS, time,
agataOutput.CoreID.at(i));
Fill(hAT.m_trackE_Crys_vs_TS[agataOutput.CoreID.at(i)], time,
agataOutput.E.at(i));
            }
        }
    }
    if(conf.user.create_mixhisto){
        if(conf.detectorsPresent.Agata){
            for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
                Fill(hM.m_EnoDC_vs_coreID, agataOutput.CoreID.at(i),
agataOutput.E.at(i));
            }
        }
        if(conf.detectorsPresent.Prisma){
            TVector2 p(prismaOutput.dE, prismaOutput.E);
            if(prismaOutput.E>3900) Fill(hM.h_Z,
(ProjectionPointOnLine(p,8.04313,-30612.704)).Mod());
        }
    }

    /*
    UserHistograms& hu = userHistograms;

    if(prismaOutput.agata_coincidence) {

std::vector<TLorentzVector> gammaDC;
std::vector<TLorentzVector> gammaDCBP;

      for(unsigned int i{0}; i<agataOutput.E.size(); i++){

    TLorentzVector p3;
    TLorentzVector p4;
    gammaDC.push_back(agataOutput.P.at(i));
    gammaDCBP.push_back(agataOutput.P.at(i));
    if (prismaOutput.p3.Mag() != 0 ){
      //Fragment is identified, evaluate energy losses

      ReactionFragment::Ion ion{prismaOutput.A, prismaOutput.Z};

      p3 = SlowInTarget(prismaOutput.p3, conf.agataPrisma.dcPos, ion);
      p4 = SlowInTarget(prismaOutput.p4, conf.agataPrisma.bpDcPos, ion);

      gammaDC.back().Boost(-p3.BoostVector());
      gammaDCBP.back().Boost(-p4.BoostVector());
    }else{
      gammaDC.back().Boost(-prismaOutput.beta);
      gammaDCBP.emplace_back();
    }

    if(prismaOutput.beta.Mag2() > 0){
      for(const auto& it: h.cutHistos){
            if(it.cut->IsInside(prismaOutput.E, prismaOutput.dE)){
      if(prismaOutoput.Q == 30 && gammaDC.back().E() > 1400 &&
gammaDC.back().E() < 1500) Fill(it.m_aoq_Eg_q30, gammaDC.back().E());
      if(prismaOutoput.Q == 31 && gammaDC.back().E() > 1400 &&
gammaDC.back().E() < 1500) Fill(it.m_aoq_Eg_q31, gammaDC.back().E());

            }
      }
    }
      }
    */

    // User process

    return kTRUE;
}

void UserSelector::SlaveTerminate()
{
    AgataSelector::SlaveTerminate();
    // User terminate
    // outFile.Write();
}

/*
TVector2 UserSelector::ProjectionPointOnLine(TVector2 point, double m, double
c){

    TVector2 line(1,m);
    TVector2 intercept(0,c);
    return point.Proj(line)+intercept-intercept.Proj(line);

}
*/