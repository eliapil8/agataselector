#include <filesystem>
#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    std::string sRun = fChain->GetDirectory()->GetName();
    sRun.erase(sRun.begin(), sRun.begin() + sRun.find("run_") + 4);
    sRun.erase(sRun.begin() + 4, sRun.end());

    runNr              = std::stoi(sRun);

    initTS             = AgataSelector::GetFirstTS();
    oldTS              = initTS;

    //PrismaAnalysis

    //inputs = DetectorAnalysis::Inputs(tree,        fChain,       fReader,
    //                          outFile,     allHistos,    enabledHistos,
    //                          firstTS,     reactions,    nBodyReactions,
    //                          targetELoss, degraderELoss);
//

    UserHistograms& hu = userhistograms;

    hu.dir             = outFile.mkdir("User", "User's custom analysis");
    hu.dir->cd();
    std::cout << "USER PART" << std::endl;


    if(conf.user.create_aoqhisto) {
        //std::cout << "USER PART" << std::endl;

        hu.aoqvsxmcp.dir
            = hu.dir->mkdir("AOQvsXMCP", "Prisma A/q analyzed histograms");
        hu.aoqvsxmcp.dir->cd();

        hu.aoqvsxmcp.zGated.dir = hu.aoqvsxmcp.dir->mkdir(
            "Zed_Gated", "Prisma A/q analyzed matrices gated in Z");
        hu.aoqvsxmcp.zGated.dir->cd();

        for(int i = conf.prisma.Zrange.first; i <= conf.prisma.Zrange.second;
            i++) {
            hu.aoqvsxmcp.zGated.m_AoQvsXmcp.push_back(nullptr);
            hu.aoqvsxmcp.zGated.m_AoQvsXmcp.back() = new TH2D(
                Form("m_AoQ_Xmcp_%i", i), Form("A/q : x MCP (Z = %i)", i),
                1000, -80, 80, 5000, 100, 500);
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
            hu.aoqvsymcp.zGated.m_AoQvsYmcp.back() = new TH2D(
                Form("m_AoQ_Ymcp_%i", i), Form("A/q : y MCP (Z = %i)", i),
                1000, -80, 80, 5000, 100, 500);
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
            hu.aoqvsxfp.zGated.m_AoQvsXfp.back() = new TH2D(
                Form("m_AoQ_Xfp_%i", i), Form("A/q : x FP (Z = %i)", i), 1200,
                -100, 1100, 5000, 100, 500);
        }
    }

    if(conf.user.create_agatarunhisto || conf.user.create_prismarunhisto
       || conf.user.create_agataprismarunhisto) {
        UserHistograms::histoRuns& hR = hu.historuns;
        hR.dir = hu.dir->mkdir("HistoRuns", "Histos over the experiment runs");
        hR.dir->cd();

        if(conf.user.create_agatarunhisto) {
            if(conf.detectorsPresent.Agata) {
                hR.hagataruns.dir = hR.dir->mkdir(
                    "AgataRuns", "Agata histos over the experiment runs");
                hR.hagataruns.dir->cd();
                hR.hagataruns.m_EnoDC_vs_runNr=
                                 new TH2D(Form("m_EnoDC_vs_runNr"),
                                          Form("m_EnoDC_vs_runNr"), 85, -0.5,
                                          84.5, 16000, 0, 4000);
                hR.hagataruns.m_CR_vs_runNr=
                                 new TH2D(Form("m_CR_vs_runNr"),
                                          Form("m_CR_vs_runNr"), 85, -0.5,
                                          84.5, 12*360, 0, 12*3600);
                hR.hagataruns.m_Integral_vs_runNr=
                                 new TH2D(Form("m_Integral_vs_runNr"),
                                          Form("m_Integral_vs_runNr"), 85, -0.5,
                                          84.5, 1, 0.5, 1.5);
                hR.hagataruns.m_CrysId_vs_runNr=
                                 new TH2D(Form("m_CrysId_vs_runNr"),
                                          Form("m_CrysId_vs_runNr"), 85, -0.5,
                                          84.5, 42, -0.5, 41.5);
                hR.hagataruns.h_EnoDC=
                                 new TH1D(Form("h_EnoDC"),
                                          Form("h_EnoDC"), 
                                          16000, 0, 4000);
                
                hR.hagataruns.m_EnoDC_vs_runNr_norm=
                                 new TH2D(Form("m_EnoDC_vs_runNr_norm"),
                                          Form("m_EnoDC_vs_runNr_norm"), 85, -0.5,
                                          84.5, 16000, 0, 4000);
                hR.hagataruns.m_CR_vs_runNr_norm=
                                 new TH2D(Form("m_CR_vs_runNr_norm"),
                                          Form("m_CR_vs_runNr_norm"), 85, -0.5,
                                          84.5, 12*360, 0, 12*3600);    
                hR.hagataruns.m_CrysId_vs_runNr_norm=
                                 new TH2D(Form("m_CrysId_vs_runNr_norm"),
                                          Form("m_CrysId_vs_runNr_norm"), 85, -0.5,
                                          84.5, 42, -0.5, 41.5);

            }
        }
        if(conf.user.create_prismarunhisto) {
            if(conf.detectorsPresent.Prisma) {
                hR.hprismaruns.dir = hR.dir->mkdir(
                    "PrismaRuns", "Prisma histos over the experiment runs");
                hR.hprismaruns.dir->cd();
                hR.hprismaruns.m_MCP_X_vs_runNr=
                                 new TH2D(Form("m_MCP_X_vs_runNr"),
                                          Form("m_MCP_X_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, -100, 100);
                hR.hprismaruns.m_MCP_Y_vs_runNr=
                                 new TH2D(Form("m_MCP_Y_vs_runNr"),
                                          Form("m_MCP_Y_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, -100, 100);
                hR.hprismaruns.m_X_FP_vs_runNr=
                                 new TH2D(Form("m_X_FP_vs_runNr"),
                                          Form("m_X_FP_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 1200, -100, 1100);
                hR.hprismaruns.m_Y_FP_vs_runNr=
                                 new TH2D(Form("m_Y_FP_vs_runNr"),
                                          Form("m_Y_FP_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 400, -100, 100);
                hR.hprismaruns.m_TOF_vs_runNr=
                                 new TH2D(Form("m_TOF_vs_runNr"),
                                          Form("m_TOF_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 500, 0, 500);
                hR.hprismaruns.m_Beta_vs_runNr=
                                 new TH2D(Form("m_Beta_vs_runNr"),
                                          Form("m_Beta_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 500, 0, 0.3);
                hR.hprismaruns.m_Radius_vs_runNr=
                                 new TH2D(Form("m_Radius_vs_runNr"),
                                          Form("m_Radius_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 1000, 500, 1700);
                hR.hprismaruns.m_RBeta_vs_runNr=
                                 new TH2D(Form("m_RBeta_vs_runNr"),
                                          Form("m_RBeta_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, 1000, 6000);
                hR.hprismaruns.m_A_over_q_vs_runNr=
                                 new TH2D(Form("m_A_over_q_vs_runNr"),
                                          Form("m_A_over_q_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 1000, 100, 500);
                hR.hprismaruns.m_Mass_vs_runNr=
                                 new TH2D(Form("m_Mass_vs_runNr"),
                                          Form("m_Mass_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, 50, 150);
                hR.hprismaruns.m_Z_Nr_vs_runNr=
                                 new TH2D(Form("m_Z_Nr_vs_runNr"),
                                          Form("m_Z_Nr_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 30, 20.5, 50.5);
                hR.hprismaruns.m_Q_Nr_vs_runNr=
                                 new TH2D(Form("m_Q_Nr_vs_runNr"),
                                          Form("m_Q_Nr_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 40, 18.5, 58.5);
                hR.hprismaruns.m_A_Nr_vs_runNr=
                                 new TH2D(Form("m_A_Nr_vs_runNr"),
                                          Form("m_A_Nr_vs_runNr;Run Nr;"), 85,
                                          -0.5, 84.5, 50, 60.5, 110.5);
                hR.hprismaruns.m_Mass_z38_vs_runNr=
                                 new TH2D(Form("m_Mass_z38_vs_runNr"),
                                          Form("m_Mass_z38_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z37_vs_runNr=
                                 new TH2D(Form("m_Mass_z37_vs_runNr"),
                                          Form("m_Mass_z37_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z36_vs_runNr=
                                 new TH2D(Form("m_Mass_z36_vs_runNr"),
                                          Form("m_Mass_z36_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z35_vs_runNr=
                                 new TH2D(Form("m_Mass_z35_vs_runNr"),
                                          Form("m_Mass_z35_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z34_vs_runNr=
                                 new TH2D(Form("m_Mass_z34_vs_runNr"),
                                          Form("m_Mass_z34_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z33_vs_runNr=
                                 new TH2D(Form("m_Mass_z33_vs_runNr"),
                                          Form("m_Mass_z33_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z32_vs_runNr=
                                 new TH2D(Form("m_Mass_z32_vs_runNr"),
                                          Form("m_Mass_z32_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z31_vs_runNr=
                                 new TH2D(Form("m_Mass_z31_vs_runNr"),
                                          Form("m_Mass_z31_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z30_vs_runNr=
                                 new TH2D(Form("m_Mass_z30_vs_runNr"),
                                          Form("m_Mass_z30_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z29_vs_runNr=
                                 new TH2D(Form("m_Mass_z29_vs_runNr"),
                                          Form("m_Mass_z29_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);                                  
                hR.hprismaruns.m_TOF_z34_vs_runNr=
                                 new TH2D(Form("m_TOF_z34_vs_runNr"),
                                          Form("m_TOF_z34_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 1000, 100, 350);
                


                hR.hprismaruns.m_MCP_X_vs_runNr_norm=
                                 new TH2D(Form("m_MCP_X_vs_runNr_norm"),
                                          Form("m_MCP_X_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, -100, 100);
                hR.hprismaruns.m_MCP_Y_vs_runNr_norm=
                                 new TH2D(Form("m_MCP_Y_vs_runNr_norm"),
                                          Form("m_MCP_Y_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, -100, 100);
                hR.hprismaruns.m_X_FP_vs_runNr_norm=
                                 new TH2D(Form("m_X_FP_vs_runNr_norm"),
                                          Form("m_X_FP_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 1200, -100, 1100);
                hR.hprismaruns.m_Y_FP_vs_runNr_norm=
                                 new TH2D(Form("m_Y_FP_vs_runNr_norm"),
                                          Form("m_Y_FP_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 400, -100, 100);
                hR.hprismaruns.m_TOF_vs_runNr_norm=
                                 new TH2D(Form("m_TOF_vs_runNr_norm"),
                                          Form("m_TOF_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 500, 0, 500);
                hR.hprismaruns.m_Beta_vs_runNr_norm=
                                 new TH2D(Form("m_Beta_vs_runNr_norm"),
                                          Form("m_Beta_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 500, 0, 0.3);
                hR.hprismaruns.m_Radius_vs_runNr_norm=
                                 new TH2D(Form("m_Radius_vs_runNr_norm"),
                                          Form("m_Radius_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 1000, 500, 1700);
                hR.hprismaruns.m_RBeta_vs_runNr_norm=
                                 new TH2D(Form("m_RBeta_vs_runNr_norm"),
                                          Form("m_RBeta_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, 1000, 6000);
                hR.hprismaruns.m_A_over_q_vs_runNr_norm=
                                 new TH2D(Form("m_A_over_q_vs_runNr_norm"),
                                          Form("m_A_over_q_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 1000, 100, 500);
                hR.hprismaruns.m_Mass_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_vs_runNr_norm"),
                                          Form("m_Mass_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 1000, 50, 150);
                hR.hprismaruns.m_Z_Nr_vs_runNr_norm=
                                 new TH2D(Form("m_Z_Nr_vs_runNr_norm"),
                                          Form("m_Z_Nr_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 30, 20.5, 50.5);
                hR.hprismaruns.m_Q_Nr_vs_runNr_norm=
                                 new TH2D(Form("m_Q_Nr_vs_runNr_norm"),
                                          Form("m_Q_Nr_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 40, 18.5, 58.5);
                hR.hprismaruns.m_A_Nr_vs_runNr_norm=
                                 new TH2D(Form("m_A_Nr_vs_runNr_norm"),
                                          Form("m_A_Nr_vs_runNr_norm;Run Nr;"), 85,
                                          -0.5, 84.5, 50, 60.5, 110.5);
                hR.hprismaruns.m_Mass_z38_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z38_vs_runNr_norm"),
                                          Form("m_Mass_z38_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z37_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z37_vs_runNr_norm"),
                                          Form("m_Mass_z37_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z36_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z36_vs_runNr_norm"),
                                          Form("m_Mass_z36_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z35_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z35_vs_runNr_norm"),
                                          Form("m_Mass_z35_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z34_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z34_vs_runNr_norm"),
                                          Form("m_Mass_z34_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z33_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z33_vs_runNr_norm"),
                                          Form("m_Mass_z33_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z32_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z32_vs_runNr_norm"),
                                          Form("m_Mass_z32_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z31_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z31_vs_runNr_norm"),
                                          Form("m_Mass_z31_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z30_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z30_vs_runNr_norm"),
                                          Form("m_Mass_z30_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);
                hR.hprismaruns.m_Mass_z29_vs_runNr_norm=
                                 new TH2D(Form("m_Mass_z29_vs_runNr_norm"),
                                          Form("m_Mass_z29_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 500, 60, 100);                                  
                hR.hprismaruns.m_TOF_z34_vs_runNr_norm=
                                 new TH2D(Form("m_TOF_z34_vs_runNr_norm"),
                                          Form("m_TOF_z34_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 1000, 100, 350);






            }
        }
        if(conf.user.create_agataprismarunhisto) {
            if(conf.detectorsPresent.Agata && conf.detectorsPresent.Prisma) {
                hR.hagataprismaruns.dir = hR.dir->mkdir(
                    "AgataPrismaRuns",
                    "Agata-Prisma histos over the experiment runs");
                hR.hagataprismaruns.dir->cd();

                hR.hagataprismaruns.m_Tdiff_PrismaAgata_vs_runNr=
                    new TH2D(Form("m_Tdiff_PrismaAgata_vs_runNr"),
                             Form("m_Tdiff_PrismaAgata_vs_runNr; Run Nr; T "
                                  "diff (10 ns);"),
                             85, -0.5, 84.5, 3000, -200, 100);
                hR.hagataprismaruns.m_EDC_86Kr_vs_runNr=
                                 new TH2D(Form("m_EDC_86Kr_vs_runNr"),
                                          Form("m_EDC_86Kr_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_88Kr_vs_runNr=
                                 new TH2D(Form("m_EDC_88Kr_vs_runNr"),
                                          Form("m_EDC_88Kr_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_84Se_vs_runNr=
                                 new TH2D(Form("m_EDC_84Se_vs_runNr"),
                                          Form("m_EDC_84Se_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_90Se_vs_runNr=
                                 new TH2D(Form("m_EDC_90Se_vs_runNr"),
                                          Form("m_EDC_90Se_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_78Ge_vs_runNr=
                                 new TH2D(Form("m_EDC_78Ge_vs_runNr"),
                                          Form("m_EDC_78Ge_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_82Ge_vs_runNr=
                                 new TH2D(Form("m_EDC_82Ge_vs_runNr"),
                                          Form("m_EDC_82Ge_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_76Zn_vs_runNr=
                                 new TH2D(Form("m_EDC_76Zn_vs_runNr"),
                                          Form("m_EDC_76Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_78Zn_vs_runNr=
                                 new TH2D(Form("m_EDC_78Zn_vs_runNr"),
                                          Form("m_EDC_78Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_79Zn_vs_runNr=
                                 new TH2D(Form("m_EDC_79Zn_vs_runNr"),
                                          Form("m_EDC_79Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_80Zn_vs_runNr=
                                 new TH2D(Form("m_EDC_80Zn_vs_runNr"),
                                          Form("m_EDC_80Zn_vs_runNr;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);



                hR.hagataprismaruns.m_Tdiff_PrismaAgata_vs_runNr_norm=
                    new TH2D(Form("m_Tdiff_PrismaAgata_vs_runNr_norm"),
                             Form("m_Tdiff_PrismaAgata_vs_runNr_norm; Run Nr; T "
                                  "diff (10 ns);"),
                             85, -0.5, 84.5, 3000, -200, 100);
                hR.hagataprismaruns.m_EDC_86Kr_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_86Kr_vs_runNr_norm"),
                                          Form("m_EDC_86Kr_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_88Kr_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_88Kr_vs_runNr_norm"),
                                          Form("m_EDC_88Kr_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_84Se_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_84Se_vs_runNr_norm"),
                                          Form("m_EDC_84Se_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_90Se_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_90Se_vs_runNr_norm"),
                                          Form("m_EDC_90Se_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_78Ge_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_78Ge_vs_runNr_norm"),
                                          Form("m_EDC_78Ge_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_82Ge_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_82Ge_vs_runNr_norm"),
                                          Form("m_EDC_82Ge_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_76Zn_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_76Zn_vs_runNr_norm"),
                                          Form("m_EDC_76Zn_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_78Zn_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_78Zn_vs_runNr_norm"),
                                          Form("m_EDC_78Zn_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_79Zn_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_79Zn_vs_runNr_norm"),
                                          Form("m_EDC_79Zn_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
                hR.hagataprismaruns.m_EDC_80Zn_vs_runNr_norm=
                                 new TH2D(Form("m_EDC_80Zn_vs_runNr_norm"),
                                          Form("m_EDC_80Zn_vs_runNr_norm;Run Nr;"),
                                          85, -0.5, 84.5, 4000, 0, 4000);
            }
        }
        if(conf.user.create_agatatimehisto) {
            UserHistograms::histoTime& hT = hu.histotime;
            hT.dir = hu.dir->mkdir("HistoTime", "Histos over time");
            hT.dir->cd();
            UserHistograms::histoTime::AgataTime& hAT = hT.hagatatime;
            hAT.dir
                = hT.dir->mkdir("HistoAgataTime", "Agata histos over time");
            hAT.dir->cd();

            hAT.h_CR = new TH1D(Form("h_CR"),
                                Form("h_CR;Time (s);Counts / 10s (keV)"),
                                21 * 360, 0, 21 * 3600);
            hAT.m_EnoDC_vs_TS
                = new TH2D(Form("m_EnoDC_vs_TS"),
                           Form("m_EnoDC_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_CrysId_vs_TS
                = new TH2D(Form("m_CrysId_vs_TS"),
                           Form("m_CrysId_vs_TS;Time (s);Crystal ID"), 21 * 60,
                           0, 21 * 3600, 45, -0.5, 44.5);
            hAT.m_trackE_00A_vs_TS
                = new TH2D(Form("m_trackE_00A_vs_TS"),
                           Form("m_trackE_00A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_00B_vs_TS
                = new TH2D(Form("m_trackE_00B_vs_TS"),
                           Form("m_trackE_00B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_00C_vs_TS
                = new TH2D(Form("m_trackE_00C_vs_TS"),
                           Form("m_trackE_00C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_01A_vs_TS
                = new TH2D(Form("m_trackE_01A_vs_TS"),
                           Form("m_trackE_01A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_01B_vs_TS
                = new TH2D(Form("m_trackE_01B_vs_TS"),
                           Form("m_trackE_01B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_01C_vs_TS
                = new TH2D(Form("m_trackE_01C_vs_TS"),
                           Form("m_trackE_01C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_02A_vs_TS
                = new TH2D(Form("m_trackE_02A_vs_TS"),
                           Form("m_trackE_02A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_02B_vs_TS
                = new TH2D(Form("m_trackE_02B_vs_TS"),
                           Form("m_trackE_02B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_02C_vs_TS
                = new TH2D(Form("m_trackE_02C_vs_TS"),
                           Form("m_trackE_02C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_04A_vs_TS
                = new TH2D(Form("m_trackE_04A_vs_TS"),
                           Form("m_trackE_04A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_04B_vs_TS
                = new TH2D(Form("m_trackE_04B_vs_TS"),
                           Form("m_trackE_04B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_04C_vs_TS
                = new TH2D(Form("m_trackE_04C_vs_TS"),
                           Form("m_trackE_04C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_05A_vs_TS
                = new TH2D(Form("m_trackE_05A_vs_TS"),
                           Form("m_trackE_05A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_05B_vs_TS
                = new TH2D(Form("m_trackE_05B_vs_TS"),
                           Form("m_trackE_05B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_05C_vs_TS
                = new TH2D(Form("m_trackE_05C_vs_TS"),
                           Form("m_trackE_05C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_06A_vs_TS
                = new TH2D(Form("m_trackE_06A_vs_TS"),
                           Form("m_trackE_06A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_06B_vs_TS
                = new TH2D(Form("m_trackE_06B_vs_TS"),
                           Form("m_trackE_06B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_06C_vs_TS
                = new TH2D(Form("m_trackE_06C_vs_TS"),
                           Form("m_trackE_06C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_07A_vs_TS
                = new TH2D(Form("m_trackE_07A_vs_TS"),
                           Form("m_trackE_07A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_07B_vs_TS
                = new TH2D(Form("m_trackE_07B_vs_TS"),
                           Form("m_trackE_07B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_07C_vs_TS
                = new TH2D(Form("m_trackE_07C_vs_TS"),
                           Form("m_trackE_07C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_08A_vs_TS
                = new TH2D(Form("m_trackE_08A_vs_TS"),
                           Form("m_trackE_08A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_08B_vs_TS
                = new TH2D(Form("m_trackE_08B_vs_TS"),
                           Form("m_trackE_08B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_08C_vs_TS
                = new TH2D(Form("m_trackE_08C_vs_TS"),
                           Form("m_trackE_08C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_09A_vs_TS
                = new TH2D(Form("m_trackE_09A_vs_TS"),
                           Form("m_trackE_09A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_09B_vs_TS
                = new TH2D(Form("m_trackE_09B_vs_TS"),
                           Form("m_trackE_09B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_09C_vs_TS
                = new TH2D(Form("m_trackE_09C_vs_TS"),
                           Form("m_trackE_09C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_10A_vs_TS
                = new TH2D(Form("m_trackE_10A_vs_TS"),
                           Form("m_trackE_10A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_10B_vs_TS
                = new TH2D(Form("m_trackE_10B_vs_TS"),
                           Form("m_trackE_10B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_10C_vs_TS
                = new TH2D(Form("m_trackE_10C_vs_TS"),
                           Form("m_trackE_10C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_11A_vs_TS
                = new TH2D(Form("m_trackE_11A_vs_TS"),
                           Form("m_trackE_11A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_11B_vs_TS
                = new TH2D(Form("m_trackE_11B_vs_TS"),
                           Form("m_trackE_11B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_11C_vs_TS
                = new TH2D(Form("m_trackE_11C_vs_TS"),
                           Form("m_trackE_11C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_13A_vs_TS
                = new TH2D(Form("m_trackE_13A_vs_TS"),
                           Form("m_trackE_13A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_13B_vs_TS
                = new TH2D(Form("m_trackE_13B_vs_TS"),
                           Form("m_trackE_13B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_13C_vs_TS
                = new TH2D(Form("m_trackE_13C_vs_TS"),
                           Form("m_trackE_13C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_14A_vs_TS
                = new TH2D(Form("m_trackE_14A_vs_TS"),
                           Form("m_trackE_14A_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_14B_vs_TS
                = new TH2D(Form("m_trackE_14B_vs_TS"),
                           Form("m_trackE_14B_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);
            hAT.m_trackE_14C_vs_TS
                = new TH2D(Form("m_trackE_14C_vs_TS"),
                           Form("m_trackE_14C_vs_TS;Time (s);Energy (keV)"),
                           21 * 60, 0, 21 * 3600, 4000, 0, 4000);

            hAT.m_trackE_Crys_vs_TS[0]  = hAT.m_trackE_00A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[1]  = hAT.m_trackE_00B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[2]  = hAT.m_trackE_00C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[3]  = hAT.m_trackE_01A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[4]  = hAT.m_trackE_01B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[5]  = hAT.m_trackE_01C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[6]  = hAT.m_trackE_02A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[7]  = hAT.m_trackE_02B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[8]  = hAT.m_trackE_02C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[12] = hAT.m_trackE_04A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[13] = hAT.m_trackE_04B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[14] = hAT.m_trackE_04C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[15] = hAT.m_trackE_05A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[16] = hAT.m_trackE_05B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[17] = hAT.m_trackE_05C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[18] = hAT.m_trackE_06A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[19] = hAT.m_trackE_06B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[20] = hAT.m_trackE_06C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[21] = hAT.m_trackE_07A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[22] = hAT.m_trackE_07B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[23] = hAT.m_trackE_07C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[24] = hAT.m_trackE_08A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[25] = hAT.m_trackE_08B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[26] = hAT.m_trackE_08C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[27] = hAT.m_trackE_09A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[28] = hAT.m_trackE_09B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[29] = hAT.m_trackE_09C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[30] = hAT.m_trackE_10A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[31] = hAT.m_trackE_10B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[32] = hAT.m_trackE_10C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[33] = hAT.m_trackE_11A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[34] = hAT.m_trackE_11B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[35] = hAT.m_trackE_11C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[39] = hAT.m_trackE_13A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[40] = hAT.m_trackE_13B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[41] = hAT.m_trackE_13C_vs_TS;
            hAT.m_trackE_Crys_vs_TS[42] = hAT.m_trackE_14A_vs_TS;
            hAT.m_trackE_Crys_vs_TS[43] = hAT.m_trackE_14B_vs_TS;
            hAT.m_trackE_Crys_vs_TS[44] = hAT.m_trackE_14C_vs_TS;
        }
        
    }
    if(conf.user.create_mixhisto){
            UserHistograms::histoMix& hM = hu.histomix;
            hM.dir  = hu.dir->mkdir("HistoMix", "Mixed histograms");
            hM.dir  ->cd();
            std::cout << "USER MIX PART" << std::endl;

            hM.m_EnoDC_vs_coreID = new TH2D(Form("m_EnoDC_vs_coreID"), Form("m_EnoDC_vs_coreID;Core ID;Energy (keV)"), 45, -0.5, 44.5,  16000, 0, 4000);
            hM.h_Z = new TH1D(Form("h_Z"), Form("h_Z;Z (a.u.);Counts"), 7000,3000,10000);
            hM.h_thetaCM_94Sr = new TH1D(Form("h_thetaCM_94Sr"), Form("h_thetaCM_94Sr; #theta_{cm} ;Counts"), 300, 0, 2*TMath::Pi());
            hM.m_vCM_thetaCM_94Sr = new TH2D(Form("m_vCM_thetaCM_94Sr"), Form("m_vCM_thetaCM_94Sr; #theta_{cm} ; v_{cm}"), 300, 0, TMath::Pi(), 1000, 0, 0.2);
            hM.m_vCM_phiCM_94Sr = new TH2D(Form("m_vCM_phiCM_94Sr"), Form("m_vCM_phiCM_94Sr; #theta_{cm} ; v_{cm}"), 300, 0, 2*TMath::Pi(), 1000, 0, 0.2);
            hM.m_vLAB_thetaLAB_94Sr = new TH2D(Form("m_vLAB_thetaLAB_94Sr"), Form("m_vLAB_thetaLAB_94Sr; #theta_{lab} ; v_{lab}"), 300, 0, TMath::Pi(), 1000, 0, 0.2);

            hM.m_xy_MCP = new TH2D(Form("m_xy_MCP"),Form("m_xy_MCP; X(mm); Y(mm)"),1000,-0.600,0.600,1000,-0.600,0.600);
            hM.m_xz_MCP = new TH2D(Form("m_xz_MCP"),Form("m_xz_MCP; X(mm); Z(mm)"),1000,-0.600,0.600,1000,-0.600,0.600);
            hM.m_yz_MCP = new TH2D(Form("m_yz_MCP"),Form("m_yz_MCP; Y(mm); Z(mm)"),1000,-0.600,0.600,1000,-0.600,0.600);

            for(auto &ion : conf.reaction.fragmentsOfInterest){
                UserHistograms::histoMix::IonHistograms ion_histos;
                ion_histos.dir
                    = hM.dir->GetDirectory(Form("Z%d_mix", ion.first.Z));
                if(ion_histos.dir == nullptr) {
                    ion_histos.dir
                        = hM.dir->mkdir(Form("Z%d_mix", ion.first.Z),
                                        Form("Gamma spectra with cuts on the "
                                             "identified ion: Z=%d",
                                             ion.first.Z));
                }
                TDirectory* tmpDir = ion_histos.dir->GetDirectory(
                    Form("A%d_mix", ion.first.A));
                if(tmpDir) {
                    throw std::runtime_error(Form(
                        "A%d directory in Z%d already exists. Duplicate ion?",
                        ion.first.A, ion.first.Z));
                }
                ion_histos.dir = ion_histos.dir->mkdir(
                    Form("A%d_mix", ion.first.A),
                    Form("Gamma spectra with cuts on the identified ion: A=%d",
                         ion.first.A));
                ion_histos.dir->cd();

                ion_histos.h_ion_EnoDC_high_energy = new TH1D(
                    Form("h_ion_EnoDC_high_energy"),
                    Form("h_ion_EnoDC_high_energy;Energy (keV); Counts"),
                    15000, 0, 15000);
                ion_histos.h_ion_EDC_high_energy = new TH1D(
                    Form("h_ion_EDC_high_energy"),
                    Form("h_ion_EDC_high_energy;Energy (keV); Counts"), 15000,
                    0, 15000);
                ion_histos.m_ion_EDC_vs_coreID = new TH2D(
                    Form("m_ion_EDC_vs_coreID"),
                    Form("m_ion_EDC_vs_coreID;Energy (keV); Counts"), 45, -0.5,
                    44.5, 4000, 0, 4000);
                ion_histos.m_ion_EnoDC_vs_coreID = new TH2D(
                    Form("m_ion_EnoDC_vs_coreID"),
                    Form("m_ion_EnoDC_vs_coreID;Energy (keV); Counts"), 45,
                    -0.5, 44.5, 4000, 0, 4000);
                if(ion.first.A == 94 && ion.first.Z == 38) {
                    ion_histos.m_ion_EDC_vs_Beta = new TH2D(
                        Form("h_ion_EDC_vs_Beta"),
                        Form("h_ion_EDC_vs_Beta;Energy (keV); Counts"), 100,
                        -50, 50, 500, 0, 1500);
                    ion_histos.m_ion_EDC_vs_AGATA_zShift = new TH2D(
                        Form("h_ion_EDC_vs_AGATA_zShift"),
                        Form("h_ion_EDC_vs_zShift;Energy (keV); Counts"), 100,
                        -20, 20, 500, 0, 1500);
                    ion_histos.m_ion_EDC_vs_AGATA_phi = new TH2D(
                        Form("h_ion_EDC_vs_AGATA_phi"),
                        Form("h_ion_EDC_vs_AGATA_phi;Energy (keV); Counts"),
                        100, -10, 10, 500, 0, 1500);
                    ion_histos.m_ion_EDC_vs_MCP_zShift = new TH2D(
                        Form("h_ion_EDC_vs_MCP_zShift"),
                        Form("h_ion_EDC_vs_MCP_zShift;Energy (keV); Counts"),
                        100, -20, 20, 500, 0, 1500);
                    ion_histos.m_ion_EDC_vs_MCP_xScale = new TH2D(
                        Form("h_ion_EDC_vs_MCP_xScale"),
                        Form("h_ion_EDC_vs_MCP_xScale;Energy (keV); Counts"),
                        100, 0.5, 2, 500, 0, 1500);
                    ion_histos.m_ion_EDC_vs_MCP_yScale = new TH2D(
                        Form("h_ion_EDC_vs_MCP_yScale"),
                        Form("h_ion_EDC_vs_MCP_yScale;Energy (keV); Counts"),
                        100, 0.5, 2, 500, 0, 1500);
                }

                hM.ionHistos[ion.first] = ion_histos;
            }

            UserHistograms::histoMix::histoTest& ht = hu.histomix.histotest;
            ht.dir  = hM.dir->mkdir("HistoTest", "Tests histograms");
            ht.dir  ->cd();
            ht.h_Mult_PRISMA_LABR   = new TH1D("h_Mult_PRISMA_LABR","h_Mult_PRISMA_LABR",20,-0.5,19.5);
            ht.h_Mult_PRISMA_AGATA  = new TH1D("h_Mult_PRISMA_AGATA","h_Mult_PRISMA_AGATA",20,-0.5,19.5);
            ht.h_ions_gamma_LABR    = new TH1D("h_ions_gamma_LABR","h_ions_gamma_LABR",2,-0.5,1.5);
            ht.h_ions_gamma_AGATA_core = new TH1D("h_ions_gamma_AGATA_core","h_ions_gamma_AGATA_core",2,-0.5,1.5);
            ht.h_ions_gamma_AGATA_tracked = new TH1D("h_ions_gamma_AGATA_tracked","h_ions_gamma_AGATA_tracked",2,-0.5,1.5);
            ht.h_ions_gamma_AGATA_addback = new TH1D("h_ions_gamma_AGATA_addback","h_ions_gamma_AGATA_addback",2,-0.5,1.5);
            ht.h_prisma_gamma_LABR    = new TH1D("h_prisma_gamma_LABR","h_prisma_gamma_LABR",2,-0.5,1.5);
            ht.h_prisma_gamma_AGATA_core = new TH1D("h_prisma_gamma_AGATA_core","h_prisma_gamma_AGATA_core",2,-0.5,1.5);
            ht.h_prisma_gamma_AGATA_tracked = new TH1D("h_prisma_gamma_AGATA_tracked","h_prisma_gamma_AGATA_tracked",2,-0.5,1.5);
            ht.h_prisma_gamma_AGATA_addback = new TH1D("h_prisma_gamma_AGATA_addback","h_prisma_gamma_AGATA_addback",2,-0.5,1.5);
            ht.h_Tdiff_84Se_2p  = new TH1D("h_Tdiff_84Se_2p","h_Tdiff_84Se_2p",500,-300,200);
            ht.h_Tdiff_84Se_bg  = new TH1D("h_Tdiff_84Se_bg","h_Tdiff_84Se_bg",500,-300,200);
            
            ht.m_EDC_Theta_84Se  = new TH2D("m_EDC_Theta_84Se","m_EDC_Theta_84Se",180,0,180,4000,0,4000);
            ht.m_EDC_Phi_84Se  = new TH2D("m_EDC_Phi_84Se","m_EDC_Phi_84Se",180,0,180,4000,0,4000);
            ht.m_EDC_Theta_94Sr  = new TH2D("m_EDC_Theta_94Sr","m_EDC_Theta_94Sr",180,0,180,4000,0,4000);
            ht.m_EDC_Phi_94Sr  = new TH2D("m_EDC_Phi_94Sr","m_EDC_Phi_94Sr",180,0,180,4000,0,4000);
            ht.m_EDC_coreID_84Se  = new TH2D("m_EDC_coreID_84Se","m_EDC_coreID_84Se",45,-0.5,44.5,4000,0,4000);
            ht.m_EDC_coreID_94Sr  = new TH2D("m_EDC_coreID_94Sr","m_EDC_coreID_94Sr",45,-0.5,44.5,4000,0,4000);

        }

        if(conf.user.eloss){

            ReactionFragment::Ion beam{84,34};
            
            std::vector<std::pair<unsigned int,unsigned int>> A_range;
            std::vector<std::pair<unsigned int,unsigned int>> Z_range;

            //Z_range.push_back(std::make_pair(25,30));
            //Z_range.push_back(std::make_pair(26,35));
            //Z_range.push_back(std::make_pair(30,40));
            //Z_range.push_back(std::make_pair(33,44));
            //Z_range.push_back(std::make_pair(38,48));
            //Z_range.push_back(std::make_pair(42,52));
            //Z_range.push_back(std::make_pair(45,55));
            //Z_range.push_back(std::make_pair(50,58));
            //Z_range.push_back(std::make_pair(54,62));
            //Z_range.push_back(std::make_pair(58,64));
            Z_range.push_back(std::make_pair(28,32));
            Z_range.push_back(std::make_pair(33,38));
            Z_range.push_back(std::make_pair(38,42));
            Z_range.push_back(std::make_pair(42,48));
            Z_range.push_back(std::make_pair(48,56));



            //A_range.push_back(std::make_pair(58,70));
            //A_range.push_back(std::make_pair(71,80));
            //A_range.push_back(std::make_pair(81,90));
            //A_range.push_back(std::make_pair(93,100));
            //A_range.push_back(std::make_pair(103,110));
            //A_range.push_back(std::make_pair(111,120));
            //A_range.push_back(std::make_pair(121,130));
            //A_range.push_back(std::make_pair(131,140));
            //A_range.push_back(std::make_pair(141,150));
            //A_range.push_back(std::make_pair(151,160));
            A_range.push_back(std::make_pair(64,82));
            A_range.push_back(std::make_pair(72,88));
            A_range.push_back(std::make_pair(86,100));
            A_range.push_back(std::make_pair(96,116));
            A_range.push_back(std::make_pair(112,138));


            auto MCP_ELossIt   = elossPrisma.MCP_ELoss.find(beam);

            double beamEnergy = 500 * UNITS::MeV;
            double thicknessC = 0.088770528 * UNITS::um ;
            double angleC = 45 * UNITS::deg;

            double thicknessPPACwindow =  1.5 * UNITS::um;

            double thicknessPPACgas =  10.8 * UNITS::mm;
            
            double thicknessICgas =  265 * UNITS::mm;

            std::ofstream efile("eloss.dat");

            std::vector<double> betas{0.09,0.1,0.11,0.12,0.13};

            for(int j = 0; j < betas.size(); j++){               

                TVector3 beta {0,0,betas[j]};
                for(int i = 0; i < A_range.size(); i++){
                    unsigned int A_min = A_range[i].first;   
                    unsigned int A_max = A_range[i].second;   
                    unsigned int Z_min = Z_range[i].first;   
                    unsigned int Z_max = Z_range[i].second;   
                    for(unsigned int z = Z_min; z <= Z_max; z++){
                        for(unsigned int a = A_min; a <= A_max; a++){

                            
                            ReactionFragment::FragmentSettings ion{a,z};

                            ReactionFragment fragment;
                            try {fragment = ReactionFragment(ion,conf.replay.conf_path+"/NuclearData/nutab12");} 
                            catch (const std::exception& e) {continue;}
                            fragment.Set_Beta(beta);
                            double beamEn = fragment.Get_Ek();

                            std::ifstream file;

                            std::string sTempMCP = "/home/engy/FusionFission/Processing/PRISMA/Eloss/Eloss_12C/Target-12-6_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempMCP);
                            if(!file.good()) continue;
                            file.close();
                            elossPrisma.MCP_ELoss.emplace(ion,
                                    EnergyLoss(sTempMCP, "SRIM", 100));
                            auto MCP_ELossIt   = elossPrisma.MCP_ELoss.find(ion);
                            std::cout << "Table file : " << sTempMCP << std::endl;


                            std::string sTempPPACwindow = "/home/engy/FusionFission/Processing/PRISMA/Eloss/Eloss_Mylar/Target-Mylar_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempPPACwindow);
                            if(!file.good()) continue;
                            file.close();
                            elossPrisma.PPAC_window_ELoss.emplace(ion,
                                    EnergyLoss(sTempPPACwindow, "SRIM", 100));
                            auto PPAC_window_ELossIt   = elossPrisma.PPAC_window_ELoss.find(ion);
                            std::cout << "Table file : " << sTempPPACwindow << std::endl;

                            std::string sTempPPACgas = "/home/engy/FusionFission/Processing/PRISMA/Eloss/Eloss_C4H10/Target-C4H10_6mbar_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempPPACgas);
                            file.close();
                            if(!file.good()) continue;                        
                            elossPrisma.PPAC_gas_ELoss.emplace(ion,
                                    EnergyLoss(sTempPPACgas, "SRIM", 100));
                            auto PPAC_gas_ELossIt   = elossPrisma.PPAC_gas_ELoss.find(ion);
                            std::cout << "Table file : " << sTempPPACgas << std::endl;

                            std::string sTempICgas = "/home/engy/FusionFission/Processing/PRISMA/Eloss/Eloss_CF4_70mbar/Target-CF4_70mbar_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempICgas);
                            file.close();
                            if(!file.good()) continue;
                            elossPrisma.IC_gas_ELoss.emplace(ion,
                                    EnergyLoss(sTempICgas, "SRIM", 100));
                            auto IC_gas_ELossIt   = elossPrisma.IC_gas_ELoss.find(ion);
                            std::cout << "Table file : " << sTempICgas << std::endl;


                            if(MCP_ELossIt != elossPrisma.MCP_ELoss.end() && PPAC_window_ELossIt != elossPrisma.PPAC_window_ELoss.end() && PPAC_gas_ELossIt != elossPrisma.PPAC_gas_ELoss.end() && IC_gas_ELossIt != elossPrisma.IC_gas_ELoss.end()) {
                                if(settings.verbosity_level > 0) {
                                    std::cout << "Ion: " << a << " - " << z << std::endl;
                                    std::cout << "Initial energy : "<< beamEn / UNITS::MeV << " MeV" << std::endl;


                                    std::cout << "Thickness of C " << thicknessC / (UNITS::um) << " um (" << thicknessC << " ) and an angle of " << angleC / UNITS::deg << " deg"<< std::endl;
                                    double afterC = MCP_ELossIt->second.Slow(beamEn , thicknessC, angleC );         
                                    std::cout << "Energy after C is "<< afterC / UNITS::MeV << " MeV" << std::endl;


                                    std::cout << "Thickness of Mylar window 1 " << thicknessPPACwindow / (UNITS::mm) << " mm (" << thicknessPPACwindow << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                    double afterPPACw1 = PPAC_window_ELossIt->second.Slow(afterC, thicknessPPACwindow, 0 );         
                                    std::cout << "Energy after PPAC window 1 is "<< afterPPACw1 / UNITS::MeV << " MeV" << std::endl;


                                    std::cout << "Thickness of PPAC gas " << thicknessPPACgas / (UNITS::mm) << " mm (" << thicknessPPACgas << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                    double afterPPACgas = PPAC_gas_ELossIt->second.Slow(afterPPACw1, thicknessPPACgas, 0 );         
                                    std::cout << "Energy after PPAC gas is "<< afterPPACgas / UNITS::MeV << " MeV" << std::endl;


                                    std::cout << "Thickness of Mylar window 2 " << thicknessPPACwindow / (UNITS::mm) << " mm (" << thicknessPPACwindow << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                    double afterPPACw2 = PPAC_window_ELossIt->second.Slow(afterPPACgas, thicknessPPACwindow, 0 );         
                                    std::cout << "Energy after PPAC window 2 is "<< afterPPACw2 / UNITS::MeV << " MeV" << std::endl;


                                    std::cout << "Thickness of Mylar window 3 " << thicknessPPACwindow / (UNITS::mm) << " mm (" << thicknessPPACwindow << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                    double afterICw = PPAC_window_ELossIt->second.Slow(afterPPACw2, thicknessPPACwindow, 0 );         
                                    std::cout << "Energy after IC window is "<< afterICw / UNITS::MeV << " MeV" << std::endl;


                                    std::cout << "Thickness of IC gas " << thicknessICgas / (UNITS::mm) << " mm (" << thicknessICgas << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                    double afterIC = IC_gas_ELossIt->second.Slow(afterICw, thicknessICgas, 0 );         
                                    std::cout << "Target energy after target is "<< afterIC / UNITS::MeV << " MeV" << std::endl;


                                    efile << a << " " << z << " " << (beamEn-afterC) / UNITS::MeV << " " << (afterC-afterPPACw1) / UNITS::MeV << " " << (afterPPACw1-afterPPACgas) / UNITS::MeV << " " << (afterPPACgas-afterPPACw2) / UNITS::MeV << " " << (afterPPACw2-afterICw) / UNITS::MeV << " " << (afterICw-afterIC) / UNITS::MeV << " " << afterIC / UNITS::MeV << std::endl;

                                }
                            }


                        }
                    }
                }
                efile <<  std::endl;
            }
        //std::cerr << "Stop to check" << std::endl;

        }
}


Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    UserHistograms&            hu = userhistograms;
    UserHistograms::histoRuns& hR = hu.historuns;
    //UserHistograms::histoTime::AgataTime& hAT = hu.histotime.hagatatime;
    UserHistograms::histoMix& hM = hu.histomix;
    if(settings.verbosity_level>1) std::cout << "Processing User sel"<< std::endl;
    if(conf.user.create_aoqhisto) {
        if(!prismaOutput.all_ok) return kTRUE;

        for(unsigned int i = 0; i < hu.aoqvsxmcp.zGated.m_AoQvsXmcp.size();
            i++) {
            unsigned int z;
            z = i + conf.prisma.Zrange.first;
            if(prismaOutput.Z == z) {
                if(!hu.aoqvsxmcp.zGated.m_AoQvsXmcp.empty())
                    hu.aoqvsxmcp.zGated.m_AoQvsXmcp.at(i)->Fill(
                        **prismaAnalysis->prismaReader.MCP_X,
                        **prismaAnalysis->prismaReader.A_over_q);
            }
        }

        for(unsigned int i = 0; i < hu.aoqvsymcp.zGated.m_AoQvsYmcp.size();
            i++) {
            unsigned int z;
            z = i + conf.prisma.Zrange.first;
            if(prismaOutput.Z == z) {
                if(!hu.aoqvsymcp.zGated.m_AoQvsYmcp.empty())
                    hu.aoqvsymcp.zGated.m_AoQvsYmcp.at(i)->Fill(
                        **prismaAnalysis->prismaReader.MCP_Y,
                        **prismaAnalysis->prismaReader.A_over_q);
            }
        }

        for(unsigned int i = 0; i < hu.aoqvsxfp.zGated.m_AoQvsXfp.size();
            i++) {
            unsigned int z;
            z = i + conf.prisma.Zrange.first;
            if(prismaOutput.Z == z) {
                if(!hu.aoqvsxfp.zGated.m_AoQvsXfp.empty())
                    hu.aoqvsxfp.zGated.m_AoQvsXfp.at(i)->Fill(
                        **prismaAnalysis->prismaReader.X_FP,
                        **prismaAnalysis->prismaReader.A_over_q);
            }
        }
    }

    //std::cout << "check 1"<< std::endl;
    if(conf.user.create_agatarunhisto || conf.user.create_prismarunhisto
       || conf.user.create_agataprismarunhisto) {
        if(conf.user.create_agatarunhisto) {
            if(conf.detectorsPresent.Agata) {
                for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
                   hR.hagataruns.h_EnoDC->Fill(agataOutput.E.at(i));
                   hR.hagataruns.m_EnoDC_vs_runNr->Fill( runNr,
                         agataOutput.E.at(i));
                   hR.hagataruns.m_CR_vs_runNr->Fill( runNr,
                         unsignedDiff<double, unsigned long long>(
                             agataOutput.TS, initTS)
                             / 1.e8);
                   hR.hagataruns.m_Integral_vs_runNr->Fill( runNr, 1.);
                   hR.hagataruns.m_CrysId_vs_runNr->Fill( runNr,
                            agataOutput.CoreID.at(i));
                }
            }
        }
        if(conf.user.create_prismarunhisto) {
            if(conf.detectorsPresent.Prisma) {
                if(!prismaOutput.all_ok) return kTRUE;           
               hR.hprismaruns.m_MCP_X_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.MCP_X);
               hR.hprismaruns.m_MCP_Y_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.MCP_Y);
               hR.hprismaruns.m_X_FP_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.X_FP);
               hR.hprismaruns.m_Y_FP_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.Y_FP);
               hR.hprismaruns.m_TOF_vs_runNr->Fill( runNr, **prismaAnalysis->prismaReader.TOF);
               hR.hprismaruns.m_Beta_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.Beta);
               hR.hprismaruns.m_Radius_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.Radius);
               hR.hprismaruns.m_RBeta_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.RBeta);
               hR.hprismaruns.m_A_over_q_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.A_over_q);
               hR.hprismaruns.m_Mass_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.Mass);
               hR.hprismaruns.m_Z_Nr_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.Z_Nr);
               hR.hprismaruns.m_Q_Nr_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.Q_Nr);
               hR.hprismaruns.m_A_Nr_vs_runNr->Fill( runNr,
                     **prismaAnalysis->prismaReader.A_Nr);
                
                if(prismaOutput.Z == 38) {
                   hR.hprismaruns.m_Mass_z38_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 37) {
                   hR.hprismaruns.m_Mass_z37_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 36) {
                   hR.hprismaruns.m_Mass_z36_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 35) {
                   hR.hprismaruns.m_Mass_z35_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 34) {
                    hR.hprismaruns.m_Mass_z34_vs_runNr->Fill(
                        runNr, **prismaAnalysis->prismaReader.Mass);
                    hR.hprismaruns.m_TOF_z34_vs_runNr->Fill(
                        runNr, **prismaAnalysis->prismaReader.TOF);
                }
                if(prismaOutput.Z == 33) {
                   hR.hprismaruns.m_Mass_z33_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 32) {
                   hR.hprismaruns.m_Mass_z32_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 31) {
                   hR.hprismaruns.m_Mass_z31_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 30) {
                   hR.hprismaruns.m_Mass_z30_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                if(prismaOutput.Z == 29) {
                   hR.hprismaruns.m_Mass_z29_vs_runNr->Fill( runNr,
                         **prismaAnalysis->prismaReader.Mass);
                }
                
                
            }
        }
        //std::cout << "check 3"<< std::endl;
        if(conf.user.create_agataprismarunhisto) {
            // if(prismaOutput.is_agata_coincidence) {
            if(1) {
                for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
                    // if(!agataOutput.prisma_coinc.at(i)) continue;
                    auto diff
                        = unsignedDiff<double, unsigned long long>(
                              prismaOutput.TS, agataOutput.TS)
                        + (prismaOutput.Time.at(0) - agataOutput.Time.at(i))
                              / (10. * UNITS::ns);
                    hR.hagataprismaruns.m_Tdiff_PrismaAgata_vs_runNr->Fill(
                        runNr, diff);

                    std::vector<TLorentzVector> gammaDC;
                    TLorentzVector              p3;
                    TVector3                    beta3;
                    gammaDC.push_back(agataOutput.P.at(i));
                    if(prismaOutput.p3.Mag() != 0) {
                        ReactionFragment::Ion ion{prismaOutput.A,
                                                  prismaOutput.Z};

                        // p3    = SlowInTarget(prismaOutput.p3,
                        //                      conf.agataPrisma.dcPos, ion);
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

                    if(prismaOutput.Z == 36) {
                        if(prismaOutput.A == 88) {
                            hR.hagataprismaruns.m_EDC_88Kr_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        } else if(prismaOutput.A == 86) {
                            hR.hagataprismaruns.m_EDC_86Kr_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        }
                    } else if(prismaOutput.Z == 34) {
                        if(prismaOutput.A == 84) {
                            hR.hagataprismaruns.m_EDC_84Se_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        } else if(prismaOutput.A == 90) {
                            hR.hagataprismaruns.m_EDC_90Se_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        }
                    } else if(prismaOutput.Z == 32) {
                        if(prismaOutput.A == 78) {
                            hR.hagataprismaruns.m_EDC_78Ge_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        } else if(prismaOutput.A == 82) {
                            hR.hagataprismaruns.m_EDC_82Ge_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        }
                    } else if(prismaOutput.Z == 30) {
                        if(prismaOutput.A == 76) {
                            hR.hagataprismaruns.m_EDC_76Zn_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        } else if(prismaOutput.A == 78) {
                            hR.hagataprismaruns.m_EDC_78Zn_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                            std::ofstream ofile;
                            ofile.open("/agata07_data5/fangelini/"
                                       "FusionFission/Processing/Analysis/log",
                                       std::ios_base::app);
                            ofile << "Found gamma 78Zn: User; TS = "
                                  << agataOutput.TS
                                  << "  1.0*runNr: " << 1.0 * runNr
                                  << "    Egamma: " << gammaDC.back().E()
                                  << std::endl;
                            ofile.close();
                        } else if(prismaOutput.A == 79) {
                            hR.hagataprismaruns.m_EDC_79Zn_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        } else if(prismaOutput.A == 80) {
                            hR.hagataprismaruns.m_EDC_80Zn_vs_runNr->Fill(
                                1.0 * runNr, gammaDC.back().E());
                        }
                    }
                }
            }
        }
        //std::cout << "check 4"<< std::endl;


        
    }

            if(conf.user.create_mixhisto){
            for(unsigned int i{0}; i < agataOutput.E.size(); ++i) {
                hM.m_EnoDC_vs_coreID->Fill(agataOutput.CoreID.at(i),agataOutput.E.at(i));
            }

                // MCP positioning part
                double   mcp_distance   = 250 * UNITS::mm;
                double   mcp_intr_angle = 135 * UNITS::deg;
                TVector3 mcpPos;

                mcpPos.SetXYZ(((**prismaAnalysis->prismaReader.MCP_X) * UNITS::mm + conf.prisma.xShift)
                                  * conf.prisma.xScale,
                              ((**prismaAnalysis->prismaReader.MCP_Y) * UNITS::mm + conf.prisma.yShift)
                                  * conf.prisma.yScale,
                              0.);
                mcpPos.RotateY(mcp_intr_angle - conf.prisma.mcp_angle);
                mcpPos.RotateX(conf.prisma.mcpThetaX);
                mcpPos.SetZ(mcp_distance + conf.prisma.zShift);

                mcpPos.RotateZ(conf.prisma.phi);
                mcpPos.RotateX(conf.prisma.thetaX);
                mcpPos.RotateY(conf.prisma.thetaY);

                // Major variation
                mcpPos.RotateY(-conf.prisma.angle);

                hM.m_xy_MCP->Fill(mcpPos.X(),mcpPos.Y());
                hM.m_xz_MCP->Fill(mcpPos.X(),mcpPos.Z());
                hM.m_yz_MCP->Fill(mcpPos.Y(),mcpPos.Z());

            if(prismaOutput.A==94&&prismaOutput.Z==38){
                //ReactionFragment::FragmentSettings prismaSettings(prismaOutput.A, prismaOutput.Z);
                //ReactionFragment ionPrisma (prismaSettings,conf.replay.conf_path + "/NuclearData/nutab12");

                ReactionFragment::Ion detectedIon{prismaOutput.A, prismaOutput.Z};
                auto reactionsIt = reactions.find(detectedIon);

                TLorentzVector p_cm = reactionsIt->second.GetFixedFragment().Get_P_cm();
                hM.h_thetaCM_94Sr->Fill(p_cm.Theta());
                hM.m_vCM_thetaCM_94Sr->Fill(p_cm.Theta(),p_cm.Beta());
                hM.m_vCM_phiCM_94Sr->Fill(p_cm.Phi(),p_cm.Beta());
                
                TLorentzVector p_lab = reactionsIt->second.GetFixedFragment().Get_P();
                hM.m_vLAB_thetaLAB_94Sr->Fill(p_lab.Theta(),p_lab.Beta());


                //TVector3 betaRn(0,0,0.1104952);
                //ionPrisma.Set_Beta(prismaOutput.beta);
                //ionPrisma.Set_Beta_cm(betaRn);

                //TLorentzVector p_cm = ionPrisma.Get_P_cm();
                //hM.h_thetaCM_94Sr->Fill(p_cm.Theta());
                //hM.m_vCM_thetaCM_94Sr->Fill(p_cm.Theta(),p_cm.Beta());

            }

            ReactionFragment::Ion detectedIon{prismaOutput.A, prismaOutput.Z};
            auto                  histoIt = hM.ionHistos.find(detectedIon);
            for(unsigned int i{0}; i < gammaAgataPrisma.E.size(); ++i) {
                if(histoIt != hM.ionHistos.end()) {
                    histoIt->second.h_ion_EDC_high_energy->Fill(
                        gammaAgataPrisma.EDC.at(i).E());
                    histoIt->second.h_ion_EnoDC_high_energy->Fill(
                        agataOutput.E.at(i));
                    histoIt->second.m_ion_EDC_vs_coreID->Fill(
                        agataOutput.CoreID.at(i),
                        gammaAgataPrisma.EDC.at(i).E());
                    histoIt->second.m_ion_EnoDC_vs_coreID->Fill(
                        agataOutput.CoreID.at(i), agataOutput.E.at(i));
                    if(detectedIon.A == 94 && detectedIon.Z == 38) {
                        for(int nBeta = -50; nBeta < 50; ++nBeta) {
                            TLorentzVector ptemp    = agataOutput.P.at(i);
                            TVector3       betatemp = prismaOutput.beta;
                            betatemp.SetMag(prismaOutput.beta.Mag()
                                            + nBeta * 0.0001);
                            ptemp.Boost(-betatemp);
                            histoIt->second.m_ion_EDC_vs_Beta->Fill(
                                (nBeta + 0.5) * 0.0002, ptemp.E());
                        }

                    }
                }
            }
            
            if(conf.detectorsPresent.Prisma&&conf.detectorsPresent.Labr&&prismaOutput.TS>0){
                if(prismaOutput.TS>0){
                    if(labrOutput.E.size()>0) hM.histotest.h_prisma_gamma_LABR->Fill(1);
                    hM.histotest.h_prisma_gamma_LABR->Fill(0);
                }
                
                if(prismaOutput.A>1&&prismaOutput.Z>1){
                    if(labrOutput.E.size()>0) hM.histotest.h_ions_gamma_LABR->Fill(1);
                    hM.histotest.h_ions_gamma_LABR->Fill(0);
                }

                //std::cout<< "here 1"<<std::endl;
                if(prismaOutput.A>1&&prismaOutput.Z>1) hM.histotest.h_Mult_PRISMA_LABR->Fill(labrOutput.E.size());
                //std::cout<< "here 2"<<std::endl;


            }
            if(conf.detectorsPresent.Prisma&&conf.detectorsPresent.Agata && prismaOutput.TS>0){
                if(prismaOutput.TS>0){
                    if(**agataAnalysis->agataReader.nbCores>0) hM.histotest.h_prisma_gamma_AGATA_core->Fill(1);
                    if(**agataAnalysis->agataReader.nbTrack>0) hM.histotest.h_prisma_gamma_AGATA_tracked->Fill(1);
                    if(**agataAnalysis->agataReader.nbAdd>0) hM.histotest.h_prisma_gamma_AGATA_addback->Fill(1);
                    hM.histotest.h_prisma_gamma_AGATA_core->Fill(0);
                    hM.histotest.h_prisma_gamma_AGATA_tracked->Fill(0);
                    hM.histotest.h_prisma_gamma_AGATA_addback->Fill(0);
                }
                if(prismaOutput.A>1&&prismaOutput.Z>1){
                   // std::cout<< "here 3"<<std::endl;
                    if(**agataAnalysis->agataReader.nbCores>0) hM.histotest.h_ions_gamma_AGATA_core->Fill(1);
                    if(**agataAnalysis->agataReader.nbTrack>0) hM.histotest.h_ions_gamma_AGATA_tracked->Fill(1);
                    if(**agataAnalysis->agataReader.nbAdd>0) hM.histotest.h_ions_gamma_AGATA_addback->Fill(1);
                    hM.histotest.h_ions_gamma_AGATA_core->Fill(0);
                    hM.histotest.h_ions_gamma_AGATA_tracked->Fill(0);
                    hM.histotest.h_ions_gamma_AGATA_addback->Fill(0);
                }
                if(prismaOutput.A>1&&prismaOutput.Z>1) hM.histotest.h_Mult_PRISMA_AGATA->Fill(agataOutput.E.size());
                   // std::cout<< "here 4"<<std::endl;

                if(prismaOutput.A==84&&prismaOutput.Z==34&&agataOutput.E.size()>0){
                  //  std::cout<< "here 4.5"<<std::endl;

                    for(unsigned int i{0}; i < gammaAgataPrisma.E.size(); ++i){
                        if(!gammaAgataPrisma.inCoinc.at(i)) continue;
                        auto edc = gammaAgataPrisma.EDC.at(i).E();
                        int index = -1;
                        for(int j{0};j<agataOutput.E.size();++j){
                            if ((gammaAgataPrisma.E.at(i).E()-agataOutput.E.at(j))<0.1) index = j;
                        }
                       // std::cout<< "here 5"<<std::endl;
                        if(index==-1) continue;
                        if(edc>1434.0&&edc<1476.0){
                            auto diff = unsignedDiff<double, unsigned long long>(prismaOutput.TS, agataOutput.TS) + (prismaOutput.Time.at(0) - agataOutput  .Time.at(index))/ (10. * UNITS::ns);
                            hM.histotest.h_Tdiff_84Se_2p->Fill(diff);
                        }
                        if(edc>350.0&&edc<500.0){
                            auto diff = unsignedDiff<double, unsigned long long>(prismaOutput.TS, agataOutput.TS) + (prismaOutput.Time.at(0) - agataOutput.Time.at(index))/ (10. * UNITS::ns);
                            hM.histotest.h_Tdiff_84Se_bg->Fill(diff);
                        }
                       // std::cout<< "here 6"<<std::endl;
                        hM.histotest.m_EDC_Theta_84Se->Fill(agataOutput.Pos.at(i).Theta()*180./UNITS::CONSTANTS::pi,edc);
                        hM.histotest.m_EDC_Phi_84Se->Fill(agataOutput.Pos.at(i).Phi()*180./UNITS::CONSTANTS::pi,edc);
                        hM.histotest.m_EDC_coreID_84Se->Fill(agataOutput.CoreID.at(i),edc);

                    }

                 
                }

                if(prismaOutput.A==94&&prismaOutput.Z==38&&agataOutput.E.size()>0){
                    for(unsigned int i{0}; i < gammaAgataPrisma.E.size(); ++i){
                        if(!gammaAgataPrisma.inCoinc.at(i)) continue;
                        auto edc = gammaAgataPrisma.EDC.at(i).E();
                        hM.histotest.m_EDC_Theta_94Sr->Fill(agataOutput.Pos.at(i).Theta()*180./UNITS::CONSTANTS::pi,edc);
                        hM.histotest.m_EDC_Phi_94Sr->Fill(agataOutput.Pos.at(i).Phi()*180./UNITS::CONSTANTS::pi,edc);
                        hM.histotest.m_EDC_coreID_94Sr->Fill(agataOutput.CoreID.at(i),edc);
                    }
                }

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
gammaDC.back().E() < 1500)it.m_aoq_Eg_q30, gammaDC.back().E());
      if(prismaOutoput.Q == 31 && gammaDC.back().E() > 1400 &&
gammaDC.back().E() < 1500)it.m_aoq_Eg_q31, gammaDC.back().E());

            }
      }
    }
      }
    }*/

    // User process
    return kTRUE;
}

void UserSelector::NormalizeRunsMatrix(TH2D * initial, TH2D * final){
    TH2D * histo = (TH2D*) initial->Clone();
    if(histo->Integral()==0) histo->Scale(0.);
    else histo->Scale(1.0/histo->Integral()/nTrees);

    for(int i = 0; i < histo->GetNbinsY(); i++){
                final->SetBinContent(   histo->FindBin( runNr,histo->GetYaxis()->GetBinCenter(i)  ),  histo->GetBinContent(   histo->GetBin(histo->GetXaxis()->FindBin(runNr),i)  ) );
    }
}



TVector3 UserSelector::GetPrismaPos(double dx, double dy, double dz, double sx,
                                    double sy, double dPhi, double dTh,
                                    double intr_angle, double intr_thetaX) {
    TVector3 mcpPos;
    // mcpPos.SetMagThetaPhi(250 * UNITS::mm, (**prismaAnalysis->prismaReader.Theta) *
    // UNITS::deg,
    //                       (**prismaAnalysis->prismaReader.Phi - 90.) * UNITS::deg);
    // mcpPos += TVector3(conf.prisma.xShift, conf.prisma.yShift,
    //                    conf.prisma.zShift);
    // mcpPos.SetX(mcpPos.x() * conf.prisma.xScale);
    // mcpPos.SetY(mcpPos.y() * conf.prisma.yScale);

    double   mcp_distance   = 250 * UNITS::mm;
    double   mcp_intr_angle = 135 * UNITS::deg;

    mcpPos.SetXYZ(((**prismaAnalysis->prismaReader.MCP_X) * UNITS::mm + conf.prisma.xShift
                   + dx * UNITS::mm)
                      * (conf.prisma.xScale + sx),
                  ((**prismaAnalysis->prismaReader.MCP_Y) * UNITS::mm + conf.prisma.yShift
                   + dy * UNITS::mm)
                      * (conf.prisma.yScale + sy),
                  0.);
    mcpPos.RotateY(mcp_intr_angle
                   - ( ( conf.prisma.mcp_angle + intr_angle ) * UNITS::deg));
    mcpPos.RotateX( ( conf.prisma.mcpThetaX + intr_thetaX ) * UNITS::deg);
    mcpPos.SetZ( ( mcp_distance + conf.prisma.zShift + dz )* UNITS::mm);

    mcpPos.RotateZ(conf.prisma.phi + dPhi * UNITS::deg);
    mcpPos.RotateX(conf.prisma.thetaX);
    mcpPos.RotateY(conf.prisma.thetaY);
    mcpPos.RotateY(-(conf.prisma.angle + dTh * UNITS::deg));

    return mcpPos;
}
/*
TLorentzVector UserSelector::SlowInTarget(TLorentzVector p, const CoincidenceConf::DcPos& pos,
                               const ReactionFragment::Ion&  ion) const {


    auto inputs = prismaAnalysis->input;

    double normAng         = p.Angle(conf.target.targetNorm);

    auto   degraderELossIt = inputs.degraderELoss.find(ion);
    auto   targetELossIt   = inputs.targetELoss.find(ion);

    double Ek              = p.E() - p.M();
    if(settings.verbosity_level > 2) {
        std::cout << "Target normal angle: "
                  << conf.target.targetNorm.Theta() / UNITS::deg << std::endl;
        std::cout << "Target phi: "
                  << conf.target.targetNorm.Phi() / UNITS::deg << std::endl;
        std::cout << "Ion angle: " << p.Theta() / UNITS::deg << std::endl;
        std::cout << "Ion phi: " << p.Phi() / UNITS::deg << std::endl;
        std::cout << "SlowInTarget ini energy: " << Ek
                  << "; Angle: " << normAng / UNITS::deg << "; Z = " << ion.Z
                  << std::endl;
    }

    switch(pos) {
    case CoincidenceConf::DcPos::BEGIN_TARGET:
        if(settings.verbosity_level > 2) {
            std::cout << "ELoss: begin target" << std::endl;
        }
        if(targetELossIt != inputs.targetELoss.end()) {
            if(normAng > UNITS::CONSTANTS::pi / 2) {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek,
                    conf.target.thickness * (conf.reaction.reaction_position),
                    normAng);
            } else {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek,
                    conf.target.thickness
                        * (1 - conf.reaction.reaction_position),
                    normAng);
            }
        }
        break;
    case CoincidenceConf::DcPos::MID_TARGET:
        if(settings.verbosity_level > 2) {
            std::cout << "ELoss: mid target" << std::endl;
        }
        break;
    case CoincidenceConf::DcPos::END_TARGET:
        if(settings.verbosity_level > 2) {
            std::cout << "ELoss: end target" << std::endl;
        }
        if(targetELossIt != inputs.targetELoss.end()) {
            if(normAng < UNITS::CONSTANTS::pi / 2) {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek,
                    conf.target.thickness
                        * (1 - conf.reaction.reaction_position),
                    normAng);
            } else {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek,
                    conf.target.thickness * (conf.reaction.reaction_position),
                    normAng);
            }
        }
        break;
    case CoincidenceConf::DcPos::MID_DEGRADER:
        if(settings.verbosity_level > 2) {
            std::cout << "ELoss: mid degrader" << std::endl;
        }
        if(targetELossIt != inputs.targetELoss.end()) {
            if(normAng < UNITS::CONSTANTS::pi / 2) {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek,
                    conf.target.thickness
                        * (1 - conf.reaction.reaction_position),
                    normAng);
            } else {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek,
                    conf.target.thickness * (conf.reaction.reaction_position),
                    normAng);
            }
        }
        if((conf.target.degPos == TargetConf::DegPos::AFTER
            && normAng < UNITS::CONSTANTS::pi / 2)
           || (conf.target.degPos == TargetConf::DegPos::BEFORE
               && normAng > UNITS::CONSTANTS::pi / 2)) {
            if(conf.target.degPresent
               && degraderELossIt != inputs.degraderELoss.end()) {
                Ek = degraderELossIt->second.SlowWithDensity(
                    Ek, conf.target.degThickness / 2., normAng);
            }
        }
        break;
    case CoincidenceConf::DcPos::END_DEGRADER:
        if(settings.verbosity_level > 2) {
            std::cout << "ELoss: end degrader" << std::endl;
        }
        if(targetELossIt != inputs.targetELoss.end()) {
            if(normAng < UNITS::CONSTANTS::pi / 2) {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek,
                    conf.target.thickness
                        * (1 - conf.reaction.reaction_position),
                    normAng);
            } else {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek,
                    conf.target.thickness * (conf.reaction.reaction_position),
                    normAng);
            }
        }
        if((conf.target.degPos == TargetConf::DegPos::AFTER
            && normAng < UNITS::CONSTANTS::pi / 2)
           || (conf.target.degPos == TargetConf::DegPos::BEFORE
               && normAng > UNITS::CONSTANTS::pi / 2)) {
            if(conf.target.degPresent
               && degraderELossIt != inputs.degraderELoss.end()) {
                Ek = degraderELossIt->second.SlowWithDensity(
                    Ek, conf.target.degThickness, normAng);
            }
        }
        break;
    default: throw std::runtime_error("DC pos makes no sense\n");
    }

    if(settings.verbosity_level > 2) {
        std::cout << "SlowInTarget final energy: " << Ek << std::endl;
    }
    double oldP = p.Vect().Mag();
    double newP = sqrt(Ek * Ek + 2 * Ek * p.M());
    p.SetPxPyPzE(p.X() * newP / oldP, p.Y() * newP / oldP, p.Z() * newP / oldP,
                 p.M() + Ek);

    return p;
}

void UserSelector::EvalInTarget(TLorentzVector& p, const ReactionFragment::Ion& ion) const {
    double normAng         = p.Angle(conf.target.targetNorm);
    double Ek              = p.E() - p.M();

    auto   degraderELossIt = inputs.degraderELoss.find(ion);
    auto   targetELossIt   = inputs.targetELoss.find(ion);

    if(settings.verbosity_level > 2) {
        std::cout << "Target normal angle: "
                  << conf.target.targetNorm.Theta() / UNITS::deg << std::endl;
        std::cout << "Target phi: "
                  << conf.target.targetNorm.Phi() / UNITS::deg << std::endl;
        std::cout << "Target normal angle: "
                  << conf.target.targetNorm.Theta() / UNITS::deg << std::endl;
        std::cout << "Target vect print: " << std::endl;
        conf.target.targetNorm.Print();
        std::cout << "Ion angle: " << p.Theta() / UNITS::deg << std::endl;
        std::cout << "Ion phi: " << p.Phi() / UNITS::deg << std::endl;
        std::cout << "Ion vector: " << std::endl;
        p.Vect().Print();
        std::cout << "EvalInTarget ini energy: " << Ek
                  << "; Angle: " << normAng / UNITS::deg << "; Z = " << ion.Z
                  << std::endl;
    }
    // Degrader energy loss if the ion passes through it.
    // The norm of the target should point in the direction of the beam
    // (positive Z)
    if(normAng < UNITS::CONSTANTS::pi / 2) {
        if(conf.target.degPresent
           && degraderELossIt != inputs.degraderELoss.end()
           && conf.target.degPos == TargetConf::DegPos::AFTER) {
            Ek = degraderELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek, conf.target.degThickness, normAng);
        }
        if(targetELossIt != inputs.targetELoss.end()) {
            Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek,
                conf.target.thickness * (1 - conf.reaction.reaction_position),
                normAng);
        }
    } else {
        if(conf.target.degPresent
           && degraderELossIt != inputs.degraderELoss.end()
           && conf.target.degPos == TargetConf::DegPos::BEFORE) {
            Ek = degraderELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek, conf.target.degThickness, normAng);
        }
        if(targetELossIt != inputs.targetELoss.end()) {
            Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek, conf.target.thickness * (conf.reaction.reaction_position),
                normAng);
        }
    }

    if(settings.verbosity_level > 2) {
        std::cout << "EvalInTarget final energy: " << Ek << std::endl;
    }

    double oldP = p.Vect().Mag();
    double newP = sqrt(Ek * Ek + 2 * Ek * p.M());
    p.SetPxPyPzE(p.X() * newP / oldP, p.Y() * newP / oldP, p.Z() * newP / oldP,
                 p.M() + Ek);
}

bool DetectorAnalysis::FileIsPresent(const std::string& fileName) {
    std::ifstream fileCheck(fileName);
    return fileCheck.good();
}
*/



void UserSelector::SlaveTerminate() {
    
    AgataSelector::SlaveTerminate();
    // User terminate

    std::string sRun = fChain->GetDirectory()->GetName();
    sRun.erase(sRun.begin() + sRun.find("Analysis/")+9, sRun.end());
    std::cout << sRun << std::endl;
    std::string pattern = "Tree_"; // Replace with your pattern

    int numTrees = 0;

    for (const auto& entry : std::filesystem::directory_iterator(sRun)) {
        if (entry.is_regular_file() && entry.path().filename().string().find(pattern) == 0) {
            numTrees++;
        }
    }
    nTrees=numTrees;


   // std::cout << "Number of files starting with '" << pattern << "': " << numTrees << std::endl;

    if(conf.user.create_agatarunhisto) {
        if(conf.detectorsPresent.Agata) {
            UserHistograms::histoRuns::AgataRuns& hA = userhistograms.historuns.hagataruns;
            NormalizeRunsMatrix(hA.m_EnoDC_vs_runNr,hA.m_EnoDC_vs_runNr_norm);
            NormalizeRunsMatrix(hA.m_CR_vs_runNr,hA.m_CR_vs_runNr_norm);
            NormalizeRunsMatrix(hA.m_CrysId_vs_runNr,hA.m_CrysId_vs_runNr_norm);

        }
    }
    if(conf.user.create_prismarunhisto) {
        if(conf.detectorsPresent.Prisma) {
            UserHistograms::histoRuns::PrismaRuns& hP = userhistograms.historuns.hprismaruns;
            NormalizeRunsMatrix(hP.m_MCP_X_vs_runNr,hP.m_MCP_X_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_MCP_Y_vs_runNr,hP.m_MCP_Y_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_X_FP_vs_runNr,hP.m_X_FP_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Y_FP_vs_runNr,hP.m_Y_FP_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_TOF_vs_runNr,hP.m_TOF_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Beta_vs_runNr,hP.m_Beta_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Radius_vs_runNr,hP.m_Radius_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_RBeta_vs_runNr,hP.m_RBeta_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_A_over_q_vs_runNr,hP.m_A_over_q_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_vs_runNr,hP.m_Mass_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Z_Nr_vs_runNr,hP.m_Z_Nr_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Q_Nr_vs_runNr,hP.m_Q_Nr_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_A_Nr_vs_runNr,hP.m_A_Nr_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z38_vs_runNr,hP.m_Mass_z38_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z37_vs_runNr,hP.m_Mass_z37_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z36_vs_runNr,hP.m_Mass_z36_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z35_vs_runNr,hP.m_Mass_z35_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z34_vs_runNr,hP.m_Mass_z34_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z33_vs_runNr,hP.m_Mass_z33_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z32_vs_runNr,hP.m_Mass_z32_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z31_vs_runNr,hP.m_Mass_z31_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z30_vs_runNr,hP.m_Mass_z30_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_Mass_z29_vs_runNr,hP.m_Mass_z29_vs_runNr_norm);
            NormalizeRunsMatrix(hP.m_TOF_z34_vs_runNr,hP.m_TOF_z34_vs_runNr_norm);            

        }
    }
    if(conf.user.create_agataprismarunhisto) {
        if(conf.detectorsPresent.Prisma&&conf.detectorsPresent.Agata) {
            UserHistograms::histoRuns::AgataPrismaRuns& hAP = userhistograms.historuns.hagataprismaruns;
            NormalizeRunsMatrix(hAP.m_Tdiff_PrismaAgata_vs_runNr,hAP.m_Tdiff_PrismaAgata_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_86Kr_vs_runNr,hAP.m_EDC_86Kr_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_88Kr_vs_runNr,hAP.m_EDC_88Kr_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_84Se_vs_runNr,hAP.m_EDC_84Se_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_90Se_vs_runNr,hAP.m_EDC_90Se_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_78Ge_vs_runNr,hAP.m_EDC_78Ge_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_82Ge_vs_runNr,hAP.m_EDC_82Ge_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_76Zn_vs_runNr,hAP.m_EDC_76Zn_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_78Zn_vs_runNr,hAP.m_EDC_78Zn_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_79Zn_vs_runNr,hAP.m_EDC_79Zn_vs_runNr_norm);
            NormalizeRunsMatrix(hAP.m_EDC_80Zn_vs_runNr,hAP.m_EDC_80Zn_vs_runNr_norm);

        }
    }
        

}
