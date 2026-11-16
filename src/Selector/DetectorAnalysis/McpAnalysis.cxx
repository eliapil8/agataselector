#include "McpAnalysis.h"
#include "Lut.h"
#include "Mcp.h"
#include <algorithm>

McpAnalysis::McpAnalysis(DetectorAnalysis::Inputs in, Mcp &d)
    : DetectorAnalysis(in, d, ConfManager::getInstance().mcp, mcpHistograms), mcp(d),
      xTOFgate(conf.replay.conf_path + "/CUT/MCP", "xmcptof"),
      rawxTOFgate(conf.replay.conf_path + "/CUT/MCP", "rawxmcptof"),
      rawXYgate(conf.replay.conf_path + "/CUT/MCP", "rawxy")
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Mcp", &mcp);
}

bool McpAnalysis::AllocateHistos()
{
    McpHistograms &h = mcpHistograms;
    h.dir            = input.outFile.mkdir("Mcp", "EXOTIC Mcp histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    Initialize<TH2D>(
        h.m_chId_minTSdiff,
        new TH2D("m_chId_minTSdiff",
                 "channel ID vs minTS (good events ONLY) ;Time difference [10 ns];Channel ID", 50,
                 0, 10, 16, 0, 16),
        h.ptrs);
    if (h.m_chId_minTSdiff != nullptr)
    {
        h.m_chId_minTSdiff->GetYaxis()->SetBinLabel(1 + 0, "X1");    // 1
        h.m_chId_minTSdiff->GetYaxis()->SetBinLabel(1 + 1, "Y1");    // 31
        h.m_chId_minTSdiff->GetYaxis()->SetBinLabel(1 + 2, "X2");    // 4
        h.m_chId_minTSdiff->GetYaxis()->SetBinLabel(1 + 3, "Y2");    // 111
        h.m_chId_minTSdiff->GetYaxis()->SetBinLabel(1 + 7, "TOF12"); // 111
    }
    Initialize<TH3D>(h.m_layout_XYZ,
                     new TH3D("m_layout_XYZ", "MCP X vs Y vs Z;<-AGATA (-X)[mm];Y[mm];Z[mm]", 100,
                              -200, 200, 100, -200, 200, 400, -3500, -2200),
                     h.ptrs);

    if (conf.mcp.debug)
    {
        Initialize<TH3D>(h.m_expected_layout_XYZ,
                         new TH3D("m_expected_layout_XYZ",
                                  "Expected MCP1 X vs Y vs Z;<-AGATA (-X)[mm];Y[mm];Z[mm]", 100,
                                  -200, 200, 100, -200, 200, 400, -3500, -2200),
                         h.ptrs);

        Initialize<TH2D>(h.m_expectedXY_gatedMCP,
                         new TH2D("m_expectedXY_gatedMCP",
                                  "Expected MCP1 XY gated on MCP2 based on given "
                                  "source position; AGATA<-(-X)(mm); Y(mm)",
                                  500, -99.5, 100.5, 500, -99.5, 100.5),
                         h.ptrs);

        Initialize<TH2D>(h.m_obsX_expectedX,
                         new TH2D("m_obsX_expectedX",
                                  "Observed X MCP1 vs expected X gated MCP2; "
                                  "X_{obs}(mm); X_{exp}(mm)",
                                  100, -99.5, 100.5, 100, -99.5, 100.5),
                         h.ptrs);

        Initialize<TH2D>(h.m_obsY_expectedY,
                         new TH2D("m_obsY_expectedY",
                                  "Observed Y MCP1 vs expected Y gated MCP2; "
                                  "Y_{obs}(mm); Y_{exp}(mm)",
                                  100, -99.5, 100.5, 100, -99.5, 100.5),
                         h.ptrs);
    }

    // ************
    Initialize<TH3D>(h.m_ionTrackSource,
                     new TH3D("m_ionTrackSource",
                              "Ion trajectories (with condition on "
                              "targetPos);AGATA<-(-X)[mm];Y[mm];Z[mm]",
                              100, -49, 51, 100, -49, 51, 600, -3500, -2200),
                     h.ptrs);

    //

    Initialize<TH3D>(h.m_ionTrack,
                     new TH3D("m_ionTrack", "Ion tracks;(-X)[mm];Y[mm];Z[mm]", 100, -49, 51, 100,
                              -49, 51, 600, -3500, 0),
                     h.ptrs);

    Initialize<TH2D>(h.m_XY_target,
                     new TH2D("m_XY_target", "target hit position XY;(-X)[mm];Y[mm]", 1000, -100,
                              100, 1000, -100, 100),
                     h.ptrs);
    Initialize<TH2D>(h.m_XY_target_bline,
                     new TH2D("m_XY_target_bline",
                              "target hit position XY with conditions on beam pipes;(-X)[mm];Y[mm]",
                              1000, -100, 100, 1000, -100, 100),
                     h.ptrs);

    Initialize<TH2D>(h.m_X1X2,
                     new TH2D("m_X1X2", "MCP1_X vs MCP2_X;<-AGATA(-X)[mm];<-AGATA(-X)[mm]", 1000,
                              -100, 100, 1000, -100, 100),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_Y1Y2,
        new TH2D("m_Y1Y2", "MCP1_Y vs MCP2_Y;(Y)[mm];(Y)[mm]", 1000, -100, 100, 1000, -100, 100),
        h.ptrs);

    Initialize<TH2D>(h.m_goodEvtsStat_mcpID,
                     new TH2D("m_goodEvtsStat_mcpID",
                              "Statistics of good vs incomplete events for "
                              "Mcp ID;ID;Goodness;Counts",
                              4, 0, 4, 8, 0, 8),
                     h.ptrs);
    if (h.m_goodEvtsStat_mcpID != nullptr)
    {
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(1, "OK");
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(2, "Missing X");
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(3, "Missing Y");
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(4, "Missing TOF");
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(5, "Missing X and Y");
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(6, "Missing X and TOF");
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(7, "Missing Y and TOF");
        h.m_goodEvtsStat_mcpID->GetYaxis()->SetBinLabel(8, "Missing T");
    }

    auto brdchanTOF = conf.mcp.getBrdChanFromId(1, McpLut::TOF12);
    for (const auto &id : conf.mcp.getAllMcpId())
    {
        auto   brdchanX = conf.mcp.getBrdChanFromId(id, McpLut::X);
        auto   brdchanY = conf.mcp.getBrdChanFromId(id, McpLut::Y);
        auto   xThr     = conf.mcp.getThr(brdchanX.first, brdchanX.second);
        auto   yThr     = conf.mcp.getThr(brdchanY.first, brdchanY.second);
        double tofThrhigh;
        double tofThrlow;
        if (conf.mcp.hasTofCal(brdchanTOF.first, brdchanTOF.second))
        {
            tofThrhigh = conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).first,
                                         brdchanTOF.first, brdchanTOF.second);
            tofThrlow = conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).second,
                                        brdchanTOF.first, brdchanTOF.second);
        }
        else
        {
            tofThrlow = conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).first,
                                        brdchanTOF.first, brdchanTOF.second);
            tofThrhigh =
                conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).second,
                                brdchanTOF.first, brdchanTOF.second);
        }

        Initialize<TH2D>(h.m_layout_front[id],
                         new TH2D(Form("m_layout_front_%i", id),
                                  // "MCP back (beam dump) view;<-PRISMA | AGATA-> X[mm];Y[mm]",
                                  "MCP front (beam) view;<-AGATA | Prisma->(-X)[mm];Y[mm]", 1000,
                                  -100, 100, 1000, -100, 100),
                         h.ptrs);

        Initialize<TH2D>(h.m_layout_top[id],
                         new TH2D(Form("m_layout_top_%i", id),
                                  // "MCP top view;<-PRISMA | AGATA-> X[mm]; beam dir Z[mm]",
                                  "MCP top view;<-AGATA | Prisma->(-X)[mm];beam->(Z)[mm]", 500,
                                  -100, 100, 500, -2500, -1200),
                         h.ptrs);

        Initialize<TH2D>(h.m_layout_side[id],
                         new TH2D(Form("m_layout_side_%i", id),
                                  "MCP side (AGATA) view;<-beam (-Z)[mm];Y[mm]", 1000, 0, 4000,
                                  1000, -200, 200),
                         h.ptrs);

        Initialize<TH1D>(h.h_rate_mcp[id],
                         new TH1D(Form("h_rate_mcp_%i", id),
                                  Form("Rates Mcp detector %i;T [s];Counts", id), 12 * 360, 0,
                                  12 * 3600),
                         h.ptrs);

        Initialize<TH2D>(h.m_XY_raw[id],
                         new TH2D(Form("m_XY_raw_%i", id),
                                  Form("RAW X-Y spectrum Mcp detector %i;X;Y;Counts", id), 1000,
                                  xThr.first, xThr.second, 1000, yThr.first, yThr.second),
                         h.ptrs);

        Initialize<TH2D>(h.m_mcpID_TS[id],
                         new TH2D(Form("m_mcpID%i_TS", id),
                                  Form("TS difference for channels in MCP%i (good events "
                                       "ONLY);#DeltaT [10 ns];Channel",
                                       id),
                                  100, -20, 20, 8, 0, 8),
                         h.ptrs);

        auto tofsPresent = conf.mcp.getTofsPresent(id);
        for (int cnt{0}; cnt < tofsPresent.size(); ++cnt)
        {
            if (!tofsPresent[cnt])
                continue;

            Initialize<TH2D>(
                h.m_tof_mcpRawX[cnt][id],
                new TH2D(
                    Form("m_tof_mcpRawX_%i_%i", cnt, id),
                    Form("RAW X-TOF%i spectrum Mcp detector %i;X [ch];TOF [au];Counts", cnt, id),
                    1000, 0, xThr.second, 1000, tofThrlow, tofThrhigh),
                h.ptrs);

            Initialize<TH2D>(
                h.m_tof_mcpX[cnt][id],
                new TH2D(Form("m_tof_mcpX_%i_%i", cnt, id),
                         Form("X-TOF%i spectrum Mcp detector %i;X;TOF [au];Counts", cnt, id), 480,
                         -200, 100, 1000, tofThrlow, tofThrhigh),
                h.ptrs);
            Initialize<TH1D>(h.h_TOF[cnt][id],
                             new TH1D(Form("h_tof_%i_%i", cnt, id),
                                      Form("TOF%i MCP %i;Tof;Counts", cnt, id), 1000, tofThrlow,
                                      tofThrhigh),
                             h.ptrs);
        }
        if (id == 2)
        {
            Initialize<TH2D>(
                h.m_tof_mcpRawX[McpLut::McpType::TOF12][id],
                new TH2D(Form("m_tof_mcpRawX_%i_%i", McpLut::McpType::TOF12, id),
                         Form("RAW X-TOF%i spectrum Mcp detector %i;X [ch];TOF [au];Counts",
                              McpLut::McpType::TOF12, id),
                         1000, xThr.first, xThr.second, 1000, tofThrlow, tofThrhigh),
                h.ptrs);
            Initialize<TH2D>(h.m_tof_mcpX[McpLut::McpType::TOF12][id],
                             new TH2D(Form("m_tof_mcpX_%i_%i", McpLut::McpType::TOF12, id),
                                      Form("X-TOF%i spectrum Mcp detector %i;X;TOF [au];Counts",
                                           McpLut::McpType::TOF12, id),
                                      480, -200, 100, 1000, tofThrlow, tofThrhigh),
                             h.ptrs);
        }
    }

    // BEAM HISTO ---------------------------------------
    Initialize<TH1D>(h.h_E, new TH1D("h_E", "Beam E;E (MeV)", 300, 0, 100), h.ptrs);
    Initialize<TH1D>(h.h_beta, new TH1D("h_beta", "Beam beta; beta", 500, 0, 1), h.ptrs);
    // mcp - mcp coindence
    Initialize<TH2D>(h.m_mcp_mcp_theta,
                     new TH2D("m_mcp_mcp_theta", "mcp  mcp coindence  ;theta;theta;Counts", 180, 0,
                              180, 180, 0, 180),
                     h.ptrs);
    Initialize<TH2D>(h.m_mcp_mcp_phi,
                     new TH2D("m_mcp_mcp_phi", "mcp  mcp coindence  ;phi;phi;Counts", 360, -180,
                              180, 360, -180, 180),
                     h.ptrs);

    for (const auto &id : conf.mcp.getAllMcpId())
    {
        auto &d = mcpPresentMask[id];

        d[McpLut::McpType::TOF1DANTE] = -2;
        d[McpLut::McpType::TOF2DANTE] = -2;

        //-1 means mandatory, -2 means not mandatory
        if (conf.mcp.mandatoryX)
            d[McpLut::McpType::X] = -1;
        else
            d[McpLut::McpType::X] = -2;

        if (conf.mcp.mandatoryY)
            d[McpLut::McpType::Y] = -1;
        else
            d[McpLut::McpType::Y] = -2;

        if (conf.mcp.mandatoryTOF)
            d[McpLut::McpType::TOF12] = -1;
        else
            d[McpLut::McpType::TOF12] = -2;

        if (std::any_of(d.begin(), d.end(), [](const auto &p) { return p > 0; }))
            throw std::runtime_error("Mcp should have at least one mandatory channel");
    }
    return true;
}

void McpAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();
    if (settings.verbosity_level > 1)
    {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW MCP EVENT\n " << std::endl;
    }

    McpHistograms &h = mcpHistograms;

    mcpPresent = mcpPresentMask;
    // int multx{0}, multy{0};
    mcp.histopos = false;
    // Find what is present
    for (unsigned int i{0}; i < mcpReader.MCPEnergy->GetSize(); ++i)
    {
        const int          &brd = mcpReader.MCPBoard->At(i);
        const int          &ch  = mcpReader.MCPChannel->At(i);
        const unsigned int &id  = conf.mcp.getMcpId(brd, ch);

        const unsigned int &type = conf.mcp.getMcpType(brd, ch);

        /*std::cout<<"brd "<<brd<<std::endl;
        std::cout<<"ch "<<ch<<std::endl;
        std::cout<<"id "<<id<<std::endl;
        std::cout<<"type "<<type<<std::endl;
        std::cout<<"energy "<<mcpReader.MCPEnergy->At(i)<<std::endl;
        */

        // Energy threshold
        if (mcpReader.MCPEnergy->At(i) > conf.mcp.getThr(brd, ch).first &&
            mcpReader.MCPEnergy->At(i) < conf.mcp.getThr(brd, ch).second)
        {

            mcpPresent[id][type] = static_cast<int>(i);
            // if(conf.mcp.getMcpType(brd,ch)==McpLut::McpType::X) multx++;
            // if(conf.mcp.getMcpType(brd,ch)==McpLut::McpType::Y) multy++;
        }
    }

    // if(multx>1 || multy>1)
    //     throw std::runtime_error("MCP's event multiplicity x "+std::to_string(multx)+" and y
    //     "+std::to_string(multy) );

    for (const auto &mcpPresentIt : mcpPresent)
    {
        if (mcpPresentIt.second.at(0) > -1 && mcpPresentIt.second.at(1) > -1 &&
            mcpPresentIt.second.at(2) > -1)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 0.);
        if (mcpPresentIt.second.at(0) < 0 && mcpPresentIt.second.at(1) > -1 &&
            mcpPresentIt.second.at(2) > -1)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 1);
        if (mcpPresentIt.second.at(0) > -1 && mcpPresentIt.second.at(1) < 0 &&
            mcpPresentIt.second.at(2) > -1)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 2);
        if (mcpPresentIt.second.at(0) > -1 && mcpPresentIt.second.at(1) > -1 &&
            mcpPresentIt.second.at(2) < 0)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 3);
        if (mcpPresentIt.second.at(0) < 0 && mcpPresentIt.second.at(1) < 0 &&
            mcpPresentIt.second.at(2) > -1)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 4);
        if (mcpPresentIt.second.at(0) < 0 && mcpPresentIt.second.at(1) > -1 &&
            mcpPresentIt.second.at(2) < 0)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 5);
        if (mcpPresentIt.second.at(0) > -1 && mcpPresentIt.second.at(1) < 0 &&
            mcpPresentIt.second.at(2) < 0)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 6);
        if (mcpPresentIt.second.at(3) < 0)
            Fill(h.m_goodEvtsStat_mcpID, mcpPresentIt.first, 7);
    }

    // Remove incomplete events
    for (auto it{mcpPresent.begin()}; it != mcpPresent.end();)
    {

        if (settings.verbosity_level > 1)
        {
            std::cout << "\n\nID " << it->first << " X " << it->second[McpLut::McpType::X] << " Y "
                      << it->second[McpLut::McpType::Y] << " TOF "
                      << it->second[McpLut::McpType::TOF12] << std::endl;
        }
        bool   test    = false;
        bool   tofTest = false;
        double tof1    = 0;
        double tof12   = 0;

        if (it->first == 1)
        {

            if ((it->second[McpLut::McpType::X] == -1 && conf.mcp.mandatoryX) ||
                (it->second[McpLut::McpType::Y] == -1 && conf.mcp.mandatoryY) ||
                (it->second[McpLut::McpType::TOF12] == -1 && conf.mcp.mandatoryTOF))
            {
                if (it->second[McpLut::McpType::TOF12] == -1 && conf.mcp.mandatoryTOF)
                    test = true;
                mcpPresent.erase(it++);
                // if(it==mcpPresent.end()){
                //     if(settings.verbosity_level>0)std::cout<<"breaking"<<std::endl;
                //     break;
                // }
                if (settings.verbosity_level > 0)
                    std::cout << "not OK mcp1" << std::endl;
            }
            else
            {
                // ensure that we passed by MCP1 otherwise there is no tof for sure and that we have
                // tof info
                if (it->second[McpLut::McpType::TOF12] > -1)
                    tofTest = true;
                auto posx        = mcpReader.MCPEnergy->At(it->second.at(McpLut::McpType::X));
                auto tofsPresent = conf.mcp.getTofsPresent(it->first);
                for (int cnt{0}; cnt < tofsPresent.size(); ++cnt)
                {

                    if ((it->second[McpLut::McpType::TOF12] < 0 && cnt == 2) ||
                        (it->second[McpLut::McpType::TOF1DANTE] < 0 && cnt == 3) ||
                        (it->second[McpLut::McpType::TOF12C] < 0 && cnt == 7))
                    {
                        tof1  = 99999;
                        tof12 = tof1;
                        break;
                    }

                    if (!tofsPresent[cnt])
                        continue;
                    auto ch  = mcpReader.MCPChannel->At(it->second.at(cnt));
                    auto brd = mcpReader.MCPBoard->At(it->second.at(cnt));

                    if (conf.mcp.getMcpId(brd, ch) != 1 && conf.mcp.getMcpId(brd, ch) != 0)
                        continue;

                    tof1 = conf.mcp.getTof(mcpReader.MCPEnergy->At(it->second.at(cnt)), brd, ch);
                    if (settings.verbosity_level > 1)
                    {
                        std::cout << "---------RAW\n\n" << std::endl;
                        std::cout << "TOF ch " << ch << std::endl;
                        std::cout << "TOF brd " << brd << std::endl;
                        std::cout << "TOF raw " << mcpReader.MCPEnergy->At(it->second.at(cnt))
                                  << std::endl;
                        std::cout << "TOF calc " << tof1 << std::endl;
                    }

                    if (cnt == McpLut::McpType::TOF12)
                        tof12 = tof1;
                    Fill(h.m_tof_mcpRawX[cnt][it->first], posx, tof1);
                }

                // CUT_TOF MCPs
                if (conf.mcp.mandatoryTOF && conf.mcp.enablecuttof)
                {

                    bool tofcutapplied = false;
                    // std::cout<<"\n\n\n\n\n\n\nApplying TOF cut for MCP1"<<std::endl;
                    for (const auto &idx : conf.mcp.indexgates_xtof)
                    {
                        // std::cout<<"TOF cut for MCP1 iter "<<idx.first<<"
                        // "<<idx.second<<std::endl;
                        if (idx.first == McpLut::McpType::TOF12 && idx.second == it->first)
                        {
                            tofcutapplied = true;
                            auto result   = rawxTOFgate.Id({idx.first, idx.second}, posx, tof12);
                            if (settings.verbosity_level > 0)
                                std::cout << "Cut XTOF for MCP id" << it->first << " TOF id "
                                          << idx.second << " " << result.first << " "
                                          << result.second[0] << std::endl;
                            if (!(result.first))
                            { //} && result.second[0] != 0){
                                if (settings.verbosity_level > 0)
                                    std::cout << "nok1" << std::endl;
                                test = true;
                                mcpPresent.erase(it++);
                                // if(it==mcpPresent.end()){
                                //     if(settings.verbosity_level>0)std::cout<<"breaking"<<std::endl;
                                //     break;
                                //     }
                            }
                            else
                            {
                                if (settings.verbosity_level > 1)
                                {
                                    std::cout << "---------\n\n" << std::endl;
                                    std::cout << "TOF ch "
                                              << mcpReader.MCPChannel->At(
                                                     it->second.at(McpLut::McpType::TOF12))
                                              << std::endl;
                                    std::cout << "TOF brd "
                                              << mcpReader.MCPBoard->At(
                                                     it->second.at(McpLut::McpType::TOF12))
                                              << std::endl;
                                    std::cout << "TOF raw "
                                              << mcpReader.MCPEnergy->At(
                                                     it->second.at(McpLut::McpType::TOF12))
                                              << std::endl;
                                    std::cout << "TOF calc " << tof12 << std::endl;
                                }
                                ++it;
                                // if(it==mcpPresent.end()){
                                //     if(settings.verbosity_level>0)std::cout<<"breaking"<<std::endl;
                                //     break;
                                // }
                                test = false;
                                if (settings.verbosity_level > 1)
                                    std::cout << "OK" << std::endl;
                            };
                        }
                    }
                    if (!tofcutapplied)
                    {
                        ++it;
                        test = false;
                    }
                }
                else
                {

                    ++it;
                    // if(it==mcpPresent.end()){
                    // break;
                    // }
                    test = false;
                    if (settings.verbosity_level > 1)
                        std::cout << "ok1" << std::endl;
                }
            }
        }

        if (settings.verbosity_level > 1)
            std::cout << "ID " << it->first << " X " << it->second[McpLut::McpType::X] << " Y "
                      << it->second[McpLut::McpType::Y] << " TOF "
                      << it->second[McpLut::McpType::TOF12] << std::endl;
        if (it->first == 2)
        {

            if (test || (it->second[McpLut::McpType::X] == -1 && conf.mcp.mandatoryX) ||
                (it->second[McpLut::McpType::Y] == -1 && conf.mcp.mandatoryY))
            {
                if (settings.verbosity_level > 0)
                    std::cout << "not OK mcp" << it->first << std::endl;
                mcpPresent.erase(it++);
                // if(it==mcpPresent.end()){
                //     if(settings.verbosity_level>0)std::cout<<"breaking"<<std::endl;
                //     break;
                // }
            }
            else
            {
                auto posx        = mcpReader.MCPEnergy->At(it->second.at(McpLut::McpType::X));
                auto tofsPresent = conf.mcp.getTofsPresent(it->first);
                for (int cnt{0}; cnt < tofsPresent.size(); ++cnt)
                {
                    // by default no TOF for MCP2
                    if (cnt == McpLut::McpType::TOF12 && tof12 > 0)
                        Fill(h.m_tof_mcpRawX[cnt][it->first], posx, tof12);
                    else if (cnt == McpLut::McpType::TOF2DANTE &&
                             it->second[McpLut::McpType::TOF2DANTE] > -1)
                    {
                        auto brd = mcpReader.MCPBoard->At(it->second.at(cnt));
                        auto ch  = mcpReader.MCPChannel->At(it->second.at(cnt));
                        tof1 =
                            conf.mcp.getTof(mcpReader.MCPEnergy->At(it->second.at(cnt)), brd, ch);
                        Fill(h.m_tof_mcpRawX[cnt][it->first], posx, tof1);
                    }
                    else if ((cnt == McpLut::McpType::TOF1DANTE &&
                              it->second[McpLut::McpType::TOF1DANTE] > -1) ||
                             (cnt == McpLut::McpType::TOF12C &&
                              it->second[McpLut::McpType::TOF12C] > -1))
                        Fill(h.m_tof_mcpRawX[cnt][it->first], posx, tof1);
                }
                // CUT_TOF MCPs
                if (conf.mcp.mandatoryTOF && conf.mcp.enablecuttof && tofTest)
                {
                    bool tofcutapplied = false;
                    for (const auto &idx : conf.mcp.indexgates_xtof)
                    {
                        if (idx.first == McpLut::McpType::TOF12 && idx.second == it->first)
                        {
                            tofcutapplied = true;
                            auto result   = rawxTOFgate.Id({idx.first, idx.second}, posx, tof12);
                            if (settings.verbosity_level > 0)
                                std::cout << "Cut XTOF for MCP id" << it->first << " TOF id "
                                          << idx.second << " " << result.first << " "
                                          << result.second[0] << std::endl;
                            if (!(result.first))
                            { //} && result.second[0] != 0){
                                if (settings.verbosity_level > 0)
                                    std::cout << "nok2" << std::endl;
                                mcpPresent.erase(it++);
                                // if(it==mcpPresent.end()){
                                //  if(settings.verbosity_level>0)std::cout<<"breaking"<<std::endl;
                                //    break;
                                // }
                            }
                            else
                            {
                                ++it;
                                // if(it==mcpPresent.end()){
                                //     if(settings.verbosity_level>0)std::cout<<"breaking"<<std::endl;
                                //     break;
                                // }
                                if (settings.verbosity_level > 0)
                                    std::cout << "OK" << std::endl;
                            };
                        }
                    }
                    if (!tofcutapplied)
                    {
                        ++it;
                    }
                }
                else
                {
                    ++it;
                    // if(it==mcpPresent.end()){
                    // break;
                    // }
                    if (settings.verbosity_level > 2)
                        std::cout << "ok2" << std::endl;
                }
            } // else (not erased event)
        }
    }

    if (mcpPresent.empty())
    {
        if (settings.verbosity_level > 0)
            std::cout << "Empty event... skipped" << std::endl;
        return;
    }

    // RAW XY GATES
    bool skip = false;
    // it->first = MCP ID: it->second = {X,Y,TOF}
    for (auto it{mcpPresent.begin()}; it != mcpPresent.end();)
    {
        skip = false;
        if (conf.mcp.enablecutxy)
        {
            if (settings.verbosity_level > 1)
                std::cout << "CUT XY ID " << it->first << " X " << it->second[McpLut::McpType::X]
                          << " Y " << it->second[McpLut::McpType::Y] << " TOF "
                          << it->second[McpLut::McpType::TOF12] << std::endl;
            for (const auto &ite : conf.mcp.indexgates_xy)
            {
                if (settings.verbosity_level > 0)
                    std::cout << "XY gates iter " << ite << std::endl;
                if (it->first == ite && !skip)
                {
                    if (settings.verbosity_level > 0)
                        std::cout << "XY gate MCP id " << it->first << std::endl;
                    auto posx   = mcpReader.MCPEnergy->At(it->second.at(McpLut::McpType::X));
                    auto posy   = mcpReader.MCPEnergy->At(it->second.at(McpLut::McpType::Y));
                    auto result = rawXYgate.Id({ite}, posx, posy);
                    if (settings.verbosity_level > 0)
                        std::cout << result.first << " " << result.second[0] << std::endl;
                    if (!(result.first))
                    {
                        if (settings.verbosity_level > 0)
                            std::cout << "Event outside the cut XY" << std::endl;
                        mcpPresent.erase(it++);
                        skip = true;
                        if (it == mcpPresent.end())
                        {
                            if (settings.verbosity_level > 0)
                                std::cout << "breaking" << std::endl;
                            break;
                        }
                    }
                    else
                    {
                        ++it;
                        skip = true;
                        if (it == mcpPresent.end())
                        {
                            if (settings.verbosity_level > 0)
                                std::cout << "breaking" << std::endl;
                            break;
                        }
                        if (settings.verbosity_level > 0)
                            std::cout << "Inside XY cut ok!" << std::endl;
                    }
                }
            }
        }
        if (!skip)
        {
            ++it;
        }
        skip = false;
    }

    if (mcpPresent.empty())
    {
        if (settings.verbosity_level > 2)
            std::cout << "Empty event... skipped" << std::endl;
        return;
    }

    // Get timestamp

    std::unordered_map<unsigned int,
                       std::pair<unsigned long long, std::pair<unsigned int, unsigned int>>>
        timestamps;
    for (const auto &d : mcpPresent)
    {

        auto tx   = std::numeric_limits<unsigned long long>::max();
        auto ty   = std::numeric_limits<unsigned long long>::max();
        auto ttof = std::numeric_limits<unsigned long long>::max();
        if (d.second.at(McpLut::McpType::X) >= 0)
            tx = mcpReader.TSMCP_Hit->At(d.second.at(McpLut::McpType::X)) +
                 conf.mcp.getTime(mcpReader.MCPTime->At(d.second.at(McpLut::McpType::X)),
                                  conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::X).first,
                                  conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::X).second);
        if (d.second.at(McpLut::McpType::Y) >= 0)
            ty = mcpReader.TSMCP_Hit->At(d.second.at(McpLut::McpType::Y)) +
                 conf.mcp.getTime(mcpReader.MCPTime->At(d.second.at(McpLut::McpType::Y)),
                                  conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::Y).first,
                                  conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::Y).second);
        if (d.second.at(McpLut::McpType::TOF12) >= 0)
            ttof =
                mcpReader.TSMCP_Hit->At(d.second.at(McpLut::McpType::TOF12)) +
                conf.mcp.getTime(mcpReader.MCPTime->At(d.second.at(McpLut::McpType::TOF12)),
                                 conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::TOF12).first,
                                 conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::TOF12).second);
        if (tx <= ttof && tx <= ty)
        {
            timestamps[d.first] = {tx, conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::X)};
        }
        else if (ty <= ttof && ty < tx)
        {
            timestamps[d.first] = {ty, conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::Y)};
        }
        else if (ttof < tx && ttof < ty)
        {
            timestamps[d.first] = {ttof,
                                   conf.mcp.getBrdChanFromId(d.first, McpLut::McpType::TOF12)};
        }
        else
        {
            throw std::runtime_error("Something is wrong with mcp conditions\n");
        }
    }

    mcp.TS =
        std::min_element(timestamps.begin(), timestamps.end(), [](const auto &l, const auto &r) {
            return l.second.first < r.second.first;
        })->second.first;
    auto minId =
        std::min_element(timestamps.begin(), timestamps.end(), [](const auto &l, const auto &r) {
            return l.second.first < r.second.first;
        })->second.second;
    if (settings.verbosity_level > 4)
    {
        std::cout << "MCP TS " << mcp.TS * conf.mcp.time_unit / (10 * UNITS::ns) << " / 10 ns"
                  << std::endl;
    }
    // Time gate
    for (const auto &ts : timestamps)
    {
        auto diffTS = (ts.second.first - mcp.TS) * conf.mcp.time_unit / (10 * UNITS::ns);
        if (unsignedDiff<int, unsigned long long>(ts.second.first, mcp.TS) > conf.mcp.windowRight)
        {
            if (settings.verbosity_level > 2)
                std::cout << "MCP ID " << ts.first << " chan " << ts.second.second.second << " TS "
                          << ts.second.first << "\tMCP TS " << mcp.TS << "\t diff " << diffTS
                          << std::endl;

            mcpPresent.erase(mcpPresent.find(ts.first));
        }
        else
        {
            // GOOD events ONLY
            Fill(h.m_chId_minTSdiff, diffTS, ts.second.second.second);
        }
    }

    if (mcpPresent.empty())
    {
        if (settings.verbosity_level > 1)
            std::cout << "Empty event after time coinc window... skipped" << std::endl;
        return;
    }
    else
        mcp.resize(2);

    for (const auto mcpPresentIt : mcpPresent)
    {

        // Extract the 3D position
        auto     idx = mcpPresentIt.first - 1;
        TVector3 offsetpos(conf.mcp.sourcex, conf.mcp.sourcey, 0);
        if (mcpPresentIt.second[McpLut::McpType::X] >= 0 &&
            mcpPresentIt.second[McpLut::McpType::Y] >= 0)
        {
            // X,Y both present

            //	  std::cout<<"x1
            //"<<mcpReader.MCPEnergy->At(mcpPresentIt.second[McpLut::McpType::X])<<" y1
            //"<<mcpReader.MCPEnergy->At(mcpPresentIt.second[McpLut::McpType::Y])<<" ID
            //"<<mcpPresentIt.first<<std::endl;
            mcp.pos.at(idx) = conf.mcp.getPosition(
                mcpReader.MCPEnergy->At(mcpPresentIt.second[McpLut::McpType::X]),
                mcpReader.MCPEnergy->At(mcpPresentIt.second[McpLut::McpType::Y]),
                mcpPresentIt.first);

            mcp.pos.at(idx) += offsetpos;
            mcp.Time.at(idx) =
                mcpReader.TSMCP_Hit->At(mcpPresentIt.second[McpLut::McpType::X]) - mcp.TS;
        }
        else if (mcpPresentIt.second[McpLut::McpType::X] >= 0 &&
                 mcpPresentIt.second[McpLut::McpType::Y] < 0)
        {
            // X only present
            mcp.pos.at(idx) = conf.mcp.getPosition(
                mcpReader.MCPEnergy->At(mcpPresentIt.second[McpLut::McpType::X]),
                conf.mcp.getAvgY(mcpPresentIt.first), mcpPresentIt.first);

            mcp.pos.at(idx) += offsetpos;
            mcp.Time.at(idx) =
                mcpReader.TSMCP_Hit->At(mcpPresentIt.second[McpLut::McpType::X]) - mcp.TS;
        }
        else if (mcpPresentIt.second[McpLut::McpType::X] < 0 &&
                 mcpPresentIt.second[McpLut::McpType::Y] >= 0)
        {
            // Y only present
            mcp.pos.at(idx) = conf.mcp.getPosition(
                conf.mcp.getAvgX(mcpPresentIt.first),
                mcpReader.MCPEnergy->At(mcpPresentIt.second[McpLut::McpType::Y]) - mcp.TS,
                mcpPresentIt.first);

            mcp.pos.at(idx) += offsetpos;
            mcp.Time.at(idx) =
                mcpReader.TSMCP_Hit->At(mcpPresentIt.second[McpLut::McpType::Y]) - mcp.TS;
        }
        else
        {
            // Neither is present
            mcp.pos.at(idx) =
                conf.mcp.getPosition(conf.mcp.getAvgX(mcpPresentIt.first),
                                     conf.mcp.getAvgY(mcpPresentIt.first), mcpPresentIt.first);

            mcp.pos.at(idx) += offsetpos;
        }

        for (size_t ridx{0}; ridx < mcpPresentIt.second.size(); ++ridx)
        {
            if (mcpPresentIt.second.at(ridx) < 0)
                continue;
            mcp.TOF.at(idx).at(ridx) =
                conf.mcp.getTof(mcpReader.MCPEnergy->At(mcpPresentIt.second.at(ridx)),
                                mcpReader.MCPBoard->At(mcpPresentIt.second.at(ridx)),
                                mcpReader.MCPChannel->At(mcpPresentIt.second.at(ridx)));

            if (settings.verbosity_level > 1 && mcp.TOF.at(idx).at(ridx) < 0)
            {
                std::cout << "----------------\n\n\nNegative TOF for MCP ID " << mcpPresentIt.first
                          << " type " << ridx << " TOF " << mcp.TOF.at(idx).at(ridx) << std::endl;
                std::cout << "Energy " << mcpReader.MCPEnergy->At(mcpPresentIt.second.at(ridx))
                          << std::endl;
                std::cout << "Board " << mcpReader.MCPBoard->At(mcpPresentIt.second.at(ridx))
                          << std::endl;
                std::cout << "Channel " << mcpReader.MCPChannel->At(mcpPresentIt.second.at(ridx))
                          << std::endl;
            }
        }
    }

    // GOOD EVENTS ONLY
    if (conf.mcp.nDetectors == 1)
    {
        mcp.histopos = false;
        mcp.bothMCP  = false;
    }
    if (conf.mcp.nDetectors == 2 && (mcp.pos.at(0).x() == 9999 || mcp.pos.at(1).x() == 9999 ||
                                     mcp.pos.at(0).y() == 9999 || mcp.pos.at(1).y() == 9999))
    {
        mcp.histopos = true;
        mcp.bothMCP  = false;
    }
    else
    {
        mcp.histopos = true;
        mcp.bothMCP  = (mcpPresent.size() == 2);
        // std::cout<<"-------------------\nBoth MCP present "<<mcp.bothMCP<<std::endl;
    }
    if (settings.verbosity_level > 3)
    {
        std::cout << "mcp.histopos " << mcp.histopos << std::endl;
        std::cout << "mcp.bothMCP " << mcp.bothMCP << std::endl;
    }

    // Reconstruct input impulse

    TVector3 dir;
    mcp.bline1 = false;
    mcp.bline2 = false;
    if (mcp.bothMCP && conf.mcp.nDetectors > 1)
    {

        if (settings.verbosity_level > 5)
        {
            std::cout << "MCP1 pos\n";
            std::cout << mcp.pos.at(0).x() << " " << mcp.pos.at(0).y() << " " << mcp.pos.at(0).z()
                      << std::endl;
        }
        // mcp.pos.at(1).SetX(-10);
        // mcp.pos.at(1).SetY(-20);
        // mcp.pos.at(1).SetZ(-1300);

        dir = mcp.pos.at(1) - mcp.pos.at(0);
        TVector3 center(0, 0, 0); // position of the target
        // plane of the target (x*conf.target.targetNorm.x() + y*conf.target.targetNorm.y() +
        // z*conf.target.targetNorm.z() = 0)
        if (dir.x() != 0 and dir.y() != 0 and dir.z() != 0)
        {
            double denom = (conf.target.targetNorm.x() * dir.x() / dir.z()) +
                           (conf.target.targetNorm.y() * dir.y() / dir.z()) +
                           conf.target.targetNorm.z();
            mcp.postarget.SetZ(
                ((mcp.pos[0].z() / dir.z()) *
                     (conf.target.targetNorm.x() * dir.x() + conf.target.targetNorm.y() * dir.y()) -
                 (mcp.pos[0].x() * conf.target.targetNorm.x() +
                  mcp.pos[0].y() * conf.target.targetNorm.y())) /
                denom);
            double zshift = (mcp.postarget.z() - mcp.pos[0].z()) / dir.z();
            mcp.postarget.SetX(zshift * dir.x() + mcp.pos[0].x());
            mcp.postarget.SetY(zshift * dir.y() + mcp.pos[0].y());
        }
        if (dir.x() == 0 and (dir.y() != 0 and dir.z() != 0))
        {
            mcp.postarget.SetX(mcp.pos[0].x());
            double denom =
                dir.y() * conf.target.targetNorm.y() / dir.z() + conf.target.targetNorm.z();
            mcp.postarget.SetZ((-mcp.pos[0].x() * conf.target.targetNorm.x() +
                                (mcp.pos[0].z() * dir.y() * conf.target.targetNorm.y() / dir.z()) -
                                mcp.pos[0].y() * conf.target.targetNorm.y()) /
                               denom);
            double zshift = (mcp.postarget.z() - mcp.pos[0].z()) / dir.z();
            mcp.postarget.SetY(zshift * dir.y() + mcp.pos[0].y());
        }
        if (dir.y() == 0 and (dir.x() != 0 and dir.z() != 0))
        {
            mcp.postarget.SetY(mcp.pos[0].y());
            double denom =
                dir.x() * conf.target.targetNorm.x() / dir.z() + conf.target.targetNorm.z();
            mcp.postarget.SetZ((-mcp.pos[0].y() * conf.target.targetNorm.y() +
                                (mcp.pos[0].z() * dir.x() * conf.target.targetNorm.x() / dir.z()) -
                                mcp.pos[0].x() * conf.target.targetNorm.x()) /
                               denom);
            double zshift = (mcp.postarget.z() - mcp.pos[0].z()) / dir.z();
            mcp.postarget.SetX(zshift * dir.x() + mcp.pos[0].x());
        }
        if (dir.z() == 0 and (dir.x() != 0 and dir.y() != 0))
        {
            mcp.postarget.SetZ(mcp.pos[0].z());
            double denom =
                dir.y() * conf.target.targetNorm.y() / dir.x() + conf.target.targetNorm.x();
            mcp.postarget.SetX((-mcp.pos[0].z() * conf.target.targetNorm.z() +
                                (mcp.pos[0].x() * dir.y() * conf.target.targetNorm.y() / dir.x()) -
                                mcp.pos[0].y() * conf.target.targetNorm.y()) /
                               denom);
            double zshift = (mcp.postarget.x() - mcp.pos[0].x()) / dir.x();
            mcp.postarget.SetY(zshift * dir.y() + mcp.pos[0].y());
        }
        if (dir.y() == 0 and dir.x() == 0 and dir.z() != 0)
        {
            mcp.postarget.SetX(mcp.pos[0].x());
            mcp.postarget.SetY(mcp.pos[0].y());
            mcp.postarget.SetZ((-mcp.pos[0].x() * conf.target.targetNorm.x() -
                                mcp.pos[0].y() * conf.target.targetNorm.y()) /
                               conf.target.targetNorm.z());
        }
        if (dir.y() == 0 and dir.z() == 0 and dir.x() != 0)
        {
            mcp.postarget.SetZ(mcp.pos[0].z());
            mcp.postarget.SetY(mcp.pos[0].y());
            mcp.postarget.SetX((-mcp.pos[0].y() * conf.target.targetNorm.y() -
                                mcp.pos[0].z() * conf.target.targetNorm.z()) /
                               conf.target.targetNorm.x());
        }
        if (dir.z() == 0 and dir.x() == 0 and dir.y() != 0)
        {
            mcp.postarget.SetZ(mcp.pos[0].z());
            mcp.postarget.SetX(mcp.pos[0].x());
            mcp.postarget.SetY((-mcp.pos[0].x() * conf.target.targetNorm.x() -
                                mcp.pos[0].z() * conf.target.targetNorm.z()) /
                               conf.target.targetNorm.y());
        }
        if (dir.y() == 0 and dir.x() == 0 and dir.z() == 0)
        {
            mcp.postarget.SetX(mcp.pos[0].x());
            mcp.postarget.SetY(mcp.pos[0].y());
            mcp.postarget.SetZ(mcp.pos[0].z());
        }

        TVector3 track = mcp.postarget - mcp.pos[0];

        if (fabs(mcp.postarget.x() - center.x()) < conf.mcp.tolerance and
            fabs(mcp.postarget.y() - center.y()) < conf.mcp.tolerance)
        {
            mcp.good = true;
        }
        else
        {
            mcp.good    = false;
            mcp.bothMCP = false;
        }

        // *****************************************************************
        // BEAM PIPES
        // BEAM PIPE of 100 mm diam
        // you can use the flag bline1 && bline2 to pass only events inside the bline to the
        // following analyses
        int nstep{300};
        for (int jj{-100}; jj < nstep + 1; jj++)
        {
            if (settings.verbosity_level > 12)
            {
                std::cout << "Z " << mcp.pos[0].z() + (track.z()) * (jj) / nstep << std::endl;
                std::cout << "X^2+Y^2 "
                          << (mcp.pos[0].x() + (track.x()) * (jj) / nstep) *
                                     (mcp.pos[0].x() + (track.x()) * (jj) / nstep) +
                                 (mcp.pos[0].y() + (track.y()) * (jj) / nstep) *
                                     (mcp.pos[0].y() + (track.y()) * (jj) / nstep)
                          << std::endl;
            }
            if ((mcp.pos[0].z() + (track.z()) * (jj) / nstep) <= conf.mcp.bline1zmin)
            {
                mcp.bline1 = true;
                mcp.bline2 = true;
            }
            if ((mcp.pos[0].z() + (track.z()) * (jj) / nstep) > conf.mcp.bline1zmin &&
                (mcp.pos[0].z() + (track.z()) * (jj) / nstep) < conf.mcp.bline1zmax)
            {
                if (settings.verbosity_level > 12)
                {
                    std::cout << "Z " << mcp.pos[0].z() + (track.z()) * (jj) / nstep << std::endl;
                    std::cout << "X^2+Y^2 "
                              << (mcp.pos[0].x() + (track.x()) * (jj) / nstep) *
                                         (mcp.pos[0].x() + (track.x()) * (jj) / nstep) +
                                     (mcp.pos[0].y() + (track.y()) * (jj) / nstep) *
                                         (mcp.pos[0].y() + (track.y()) * (jj) / nstep)
                              << std::endl;
                }

                if ((mcp.pos[0].x() + (track.x()) * (jj) / nstep) *
                            (mcp.pos[0].x() + (track.x()) * (jj) / nstep) +
                        (mcp.pos[0].y() + (track.y()) * (jj) / nstep) *
                            (mcp.pos[0].y() + (track.y()) * (jj) / nstep) <
                    conf.mcp.bline1radius * conf.mcp.bline1radius)
                {
                    mcp.bline1 = true;

                    if (settings.verbosity_level > 12)
                    {
                        std::cout << "Event inside beampipe 1" << std::endl;
                    }
                } // BEAM PIPE of 100 mm diam
                else
                    mcp.bline1 = false;
            }

            if ((mcp.pos[0].z() + (track.z()) * (jj) / nstep) > conf.mcp.bline2zmin &&
                (mcp.pos[0].z() + (track.z()) * (jj) / nstep) < conf.mcp.bline2zmax)
            {
                if ((mcp.pos[0].x() + (track.x()) * (jj) / nstep) *
                            (mcp.pos[0].x() + (track.x()) * (jj) / nstep) +
                        (mcp.pos[0].y() + (track.y()) * (jj) / nstep) *
                            (mcp.pos[0].y() + (track.y()) * (jj) / nstep) <
                    conf.mcp.bline2radius * conf.mcp.bline2radius)
                {
                    mcp.bline2 = true;
                    if (settings.verbosity_level > 12)
                    {
                        std::cout << "Event inside beampipe 2" << std::endl;
                    }
                } // BEAM PIPE of 50 mm diam
                else
                    mcp.bline2 = false;
            }

            if (mcp.bline1 && mcp.bline2)
            {
                Fill(h.m_ionTrack, -(mcp.pos[0].x() + (track.x()) * (jj) / nstep),
                     (mcp.pos[0].y() + (track.y()) * (jj) / nstep),
                     (mcp.pos[0].z() + (track.z()) * (jj) / nstep));
            }
        }

        // SOURCE POSITION checking reconstruction
        // only for debug (is a repetition of the code before)
        TVector3 dirDeb;
        if (conf.mcp.debug)
        {
            TVector3 postargetDeb;
            TVector3 source(conf.mcp.sourcex, conf.mcp.sourcey, conf.mcp.sourcez);
            dir = -source + mcp.pos.at(1);

            if (dirDeb.x() != 0 and dirDeb.y() != 0 and dirDeb.z() != 0)
            {
                double denom = (conf.target.targetNorm.x() * dirDeb.x() / dirDeb.z()) +
                               (conf.target.targetNorm.y() * dirDeb.y() / dirDeb.z()) +
                               conf.target.targetNorm.z();
                postargetDeb.SetZ(
                    ((mcp.pos[1].z() / dirDeb.z()) * (conf.target.targetNorm.x() * dirDeb.x() +
                                                      conf.target.targetNorm.y() * dirDeb.y()) -
                     (mcp.pos[1].x() * conf.target.targetNorm.x() +
                      mcp.pos[1].y() * conf.target.targetNorm.y())) /
                    denom);
                double zshift = (postargetDeb.z() - mcp.pos[1].z()) / dirDeb.z();
                postargetDeb.SetX(zshift * dirDeb.x() + mcp.pos[1].x());
                postargetDeb.SetY(zshift * dirDeb.y() + mcp.pos[1].y());
            }
            if (dirDeb.x() == 0 and (dirDeb.y() != 0 and dirDeb.z() != 0))
            {
                postargetDeb.SetX(mcp.pos[1].x());
                double denom = dirDeb.y() * conf.target.targetNorm.y() / dirDeb.z() +
                               conf.target.targetNorm.z();
                postargetDeb.SetZ(
                    (-mcp.pos[1].x() * conf.target.targetNorm.x() +
                     (mcp.pos[1].z() * dirDeb.y() * conf.target.targetNorm.y() / dirDeb.z()) -
                     mcp.pos[1].y() * conf.target.targetNorm.y()) /
                    denom);
                double zshift = (postargetDeb.z() - mcp.pos[1].z()) / dirDeb.z();
                postargetDeb.SetY(zshift * dirDeb.y() + mcp.pos[1].y());
            }
            if (dirDeb.y() == 0 and (dirDeb.x() != 0 and dirDeb.z() != 0))
            {
                postargetDeb.SetY(mcp.pos[1].y());
                double denom = dirDeb.x() * conf.target.targetNorm.x() / dirDeb.z() +
                               conf.target.targetNorm.z();
                postargetDeb.SetZ(
                    (-mcp.pos[1].y() * conf.target.targetNorm.y() +
                     (mcp.pos[1].z() * dirDeb.x() * conf.target.targetNorm.x() / dirDeb.z()) -
                     mcp.pos[1].x() * conf.target.targetNorm.x()) /
                    denom);
                double zshift = (postargetDeb.z() - mcp.pos[1].z()) / dirDeb.z();
                postargetDeb.SetX(zshift * dirDeb.x() + mcp.pos[1].x());
            }
            if (dirDeb.z() == 0 and (dirDeb.x() != 0 and dirDeb.y() != 0))
            {
                postargetDeb.SetZ(mcp.pos[1].z());
                double denom = dirDeb.y() * conf.target.targetNorm.y() / dirDeb.x() +
                               conf.target.targetNorm.x();
                postargetDeb.SetX(
                    (-mcp.pos[1].z() * conf.target.targetNorm.z() +
                     (mcp.pos[1].x() * dirDeb.y() * conf.target.targetNorm.y() / dirDeb.x()) -
                     mcp.pos[1].y() * conf.target.targetNorm.y()) /
                    denom);
                double zshift = (postargetDeb.x() - mcp.pos[1].x()) / dirDeb.x();
                postargetDeb.SetY(zshift * dirDeb.y() + mcp.pos[1].y());
            }
            if (dirDeb.y() == 0 and dirDeb.x() == 0 and dirDeb.z() != 0)
            {
                postargetDeb.SetX(mcp.pos[1].x());
                postargetDeb.SetY(mcp.pos[1].y());
                postargetDeb.SetZ((-mcp.pos[1].x() * conf.target.targetNorm.x() -
                                   mcp.pos[1].y() * conf.target.targetNorm.y()) /
                                  conf.target.targetNorm.z());
            }
            if (dirDeb.y() == 0 and dirDeb.z() == 0 and dirDeb.x() != 0)
            {
                postargetDeb.SetZ(mcp.pos[1].z());
                postargetDeb.SetY(mcp.pos[1].y());
                postargetDeb.SetX((-mcp.pos[1].y() * conf.target.targetNorm.y() -
                                   mcp.pos[1].z() * conf.target.targetNorm.z()) /
                                  conf.target.targetNorm.x());
            }
            if (dirDeb.z() == 0 and dirDeb.x() == 0 and dirDeb.y() != 0)
            {
                postargetDeb.SetZ(mcp.pos[1].z());
                postargetDeb.SetX(mcp.pos[1].x());
                postargetDeb.SetY((-mcp.pos[1].x() * conf.target.targetNorm.x() -
                                   mcp.pos[1].z() * conf.target.targetNorm.z()) /
                                  conf.target.targetNorm.y());
            }
            if (dirDeb.y() == 0 and dirDeb.x() == 0 and dirDeb.z() == 0)
            {
                postargetDeb.SetX(mcp.pos[1].x());
                postargetDeb.SetY(mcp.pos[1].y());
                postargetDeb.SetZ(mcp.pos[1].z());
            }
            track = postargetDeb - mcp.pos[1];
            if (fabs(postargetDeb.x() - center.x()) < conf.mcp.tolerance and
                fabs(postargetDeb.y() - center.y()) < conf.mcp.tolerance)
            {
                int nstep{500};
                for (int jj{-600}; jj < nstep + 1; jj++)
                    Fill(h.m_ionTrackSource, -(mcp.pos[1].x() + (track.x()) * (jj) / nstep),
                         (mcp.pos[1].y() + (track.y()) * (jj) / nstep),
                         (mcp.pos[1].z() + (track.z()) * (jj) / nstep));
            }

            // Intersezione piano MCP1 con retta  || dirDeb
            TVector3 mcp0(0, 0, 0);
            double   t = (conf.mcp.dcentremcp1 + conf.mcp.sourcey - conf.mcp.sourcez) /
                       (dirDeb.x() + dirDeb.z());
            mcp0.SetX(conf.mcp.sourcex + dirDeb.x() * t);
            mcp0.SetY(conf.mcp.sourcey + dirDeb.y() * t);
            mcp0.SetZ(conf.mcp.sourcez + dirDeb.z() * t);

            if (settings.verbosity_level > 4)
            {
                std::cout << "Expected MCP1 pos" << "\tx " << mcp0.x() << "\ty " << mcp0.y()
                          << "\tz " << -mcp0.z() << "\n"
                          << std::endl;
            }
            Fill(mcpHistograms.m_expectedXY_gatedMCP, -mcp0.x(), mcp0.y());
            Fill(mcpHistograms.m_obsX_expectedX, mcp.pos.at(0).x(), mcp0.x());
            Fill(mcpHistograms.m_obsY_expectedY, mcp.pos.at(0).y(), mcp0.y());
            Fill(h.m_expected_layout_XYZ, -mcp0.x(), mcp0.y(), mcp0.z());
            Fill(h.m_expected_layout_XYZ, -mcp.pos.at(1).x(), mcp.pos.at(1).y(), mcp.pos.at(1).z());
        }
        // end of SOURCE POSITION checking reconstruction

        /* BEAM DIRECTION */
        ////////////////////////////

        const ReactionFragment::Ion &ion{conf.mcp.ion};
        auto                         reactionsIt = input.reactions.find(ion);
        if (reactionsIt != input.reactions.end())
        {
            auto   beam = reactionsIt->second.GetReactionFragment(1);
            double tof  = -1;
            if (mcp.TOF.at(0).at(McpLut::McpType::TOF12) > 0)
            {
                tof = mcp.TOF.at(0).at(McpLut::McpType::TOF12);
            }
            else if (mcp.TOF.at(1).at(McpLut::McpType::TOF12) > 0)
            {
                tof = mcp.TOF.at(1).at(McpLut::McpType::TOF12);
            }
            if (tof > 0)
            {
                TVector3 beta(dir.X() * UNITS::mm / (tof * UNITS::ns * UNITS::PHYSICS::c_light),
                              dir.Y() * UNITS::mm / (tof * UNITS::ns * UNITS::PHYSICS::c_light),
                              dir.Z() * UNITS::mm / (tof * UNITS::ns * UNITS::PHYSICS::c_light));
                beam.Set_Beta(beta);
                beam.Set_Theta_Phi(dir.Theta(), dir.Phi());
                mcp.p1 = beam.Get_P();
                // std::cout<<"Reconstructed beam momentum: "<<mcp.p1.Mag()<<" MeV/c"<<std::endl;
                // std::cout<<"MCP -SIDE- p1 (x,y,z) = "<<mcp.p1.Vect().x()<<"
                // "<<mcp.p1.Vect().y()<<" "<<mcp.p1.Vect().z()<<std::endl; std::cout<<"MCPTS
                // "<<mcp.TS<<std::endl;
                EvalInTarget(mcp.p1, ion);
            }
        }
        if (settings.verbosity_level > 2)
        {
            std::cout << "TOF " << mcp.TOF.at(0).at(McpLut::McpType::TOF12) * UNITS::ns << " [s]"
                      << std::endl;
            std::cout << "Reconstructed beam momentum: " << mcp.p1.Mag() << " MeV/c" << std::endl;
            std::cout << "Reconstructed beam energy: " << mcp.p1.E() - mcp.p1.M() << " MeV"
                      << std::endl;
            std::cout << "Beam direction Theta: " << dir.Theta() * TMath::RadToDeg()
                      << " deg Phi: " << dir.Phi() * TMath::RadToDeg() << " deg" << std::endl;
            std::cout << "Beam beta: " << mcp.p1.Beta() << std::endl;
            std::cout << "Beam direction unit vector: " << dir.Unit().X() << " " << dir.Unit().Y()
                      << " " << dir.Unit().Z() << std::endl;
            std::cout << "Beam direction unit vector 2: " << mcp.p1.Vect().Unit().X() << " "
                      << mcp.p1.Vect().Unit().Y() << " " << mcp.p1.Vect().Unit().Z() << std::endl;
        }

    } // nDetectors>1

    if (settings.verbosity_level > 1)
        std::cout << "--------------------------------------------------------------" << std::endl;
}

bool McpAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Filling histograms here
    McpHistograms &h = mcpHistograms;
    if (mcp.bothMCP)
    {

        for (const auto &mcpPresentIt : mcpPresent)
        {

            // TEST TOF
            if (settings.verbosity_level > 4)
            {
                unsigned int cnt = 0;
                for (const auto &id : conf.mcp.getTofsPresent(mcpPresentIt.first))
                {
                    std::cout << " TOFs present: " << mcpPresentIt.first << " " << cnt << " " << id
                              << std::endl;
                    cnt++;
                }
            }

            Fill(h.m_XY_raw.at(mcpPresentIt.first), mcpReader.MCPEnergy->At(mcpPresentIt.second[0]),
                 mcpReader.MCPEnergy->At(mcpPresentIt.second[1]));

            // TS X-Y-TOF-T  vs TS X
            if (mcpPresentIt.second.at(0) > -1)
            {

                Fill(h.m_mcpID_TS[mcpPresentIt.first],
                     (mcpReader.TSMCP_Hit->At(mcpPresentIt.second.at(0)) +
                      conf.mcp.getTime(
                          mcpReader.MCPTime->At(mcpPresentIt.second.at(0)),
                          conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X).first,
                          conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X)
                              .second)) *
                             conf.mcp.time_unit / (10 * UNITS::ns) -
                         (mcpReader.TSMCP_Hit->At(mcpPresentIt.second.at(0)) +
                          conf.mcp.getTime(
                              mcpReader.MCPTime->At(mcpPresentIt.second.at(0)),
                              conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X)
                                  .first,
                              conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X)
                                  .second)) *
                             conf.mcp.time_unit / (10 * UNITS::ns),
                     0.);
            }
            if (mcpPresentIt.second.at(0) > -1 && mcpPresentIt.second.at(1) > -1)
            {
                Fill(h.m_mcpID_TS[mcpPresentIt.first],
                     (mcpReader.TSMCP_Hit->At(mcpPresentIt.second.at(1)) +
                      conf.mcp.getTime(
                          mcpReader.MCPTime->At(mcpPresentIt.second.at(0)),
                          conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::Y).first,
                          conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::Y)
                              .second)) *
                             conf.mcp.time_unit / (10 * UNITS::ns) -
                         (mcpReader.TSMCP_Hit->At(mcpPresentIt.second.at(0)) +
                          conf.mcp.getTime(
                              mcpReader.MCPTime->At(mcpPresentIt.second.at(0)),
                              conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X)
                                  .first,
                              conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X)
                                  .second)) *
                             conf.mcp.time_unit / (10 * UNITS::ns),
                     1);
            }
            if (mcpPresentIt.second.at(0) > -1 && mcpPresentIt.second.at(2) > -1)
            {
                Fill(h.m_mcpID_TS[mcpPresentIt.first],
                     (mcpReader.TSMCP_Hit->At(mcpPresentIt.second.at(2)) +
                      conf.mcp.getTime(
                          mcpReader.MCPTime->At(mcpPresentIt.second.at(0)),
                          conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::TOF12)
                              .first,
                          conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::TOF12)
                              .second)) *
                             conf.mcp.time_unit / (10 * UNITS::ns) -
                         (mcpReader.TSMCP_Hit->At(mcpPresentIt.second.at(0)) +
                          conf.mcp.getTime(
                              mcpReader.MCPTime->At(mcpPresentIt.second.at(0)),
                              conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X)
                                  .first,
                              conf.mcp.getBrdChanFromId(mcpPresentIt.first, McpLut::McpType::X)
                                  .second)) *
                             conf.mcp.time_unit / (10 * UNITS::ns),
                     2);
            }
            // and rate
            for (unsigned int jj{0}; jj < mcpPresentIt.second.size(); jj++)
                if (mcpPresentIt.second.at(jj) >= 0)
                {
                    Fill(h.h_rate_mcp[mcpPresentIt.first],
                         (mcpReader.TSMCP_Hit->At(mcpPresentIt.second.at(jj)) - input.firstTS) *
                             conf.mcp.time_unit);
                }

            Fill(h.m_layout_front[mcpPresentIt.first], -mcp.pos.at(mcpPresentIt.first - 1).x(),
                 mcp.pos.at(mcpPresentIt.first - 1).y());
            Fill(h.m_layout_top[mcpPresentIt.first], -mcp.pos.at(mcpPresentIt.first - 1).x(),
                 mcp.pos.at(mcpPresentIt.first - 1).z());
            Fill(h.m_layout_side[mcpPresentIt.first], -mcp.pos.at(mcpPresentIt.first - 1).z(),
                 mcp.pos.at(mcpPresentIt.first - 1).y());
            Fill(h.m_layout_XYZ, -mcp.pos.at(mcpPresentIt.first - 1).x(),
                 mcp.pos.at(mcpPresentIt.first - 1).y(), mcp.pos.at(mcpPresentIt.first - 1).z());

            // TOFs
            // if(mcp.bothMCP){
            auto tofsPresent = conf.mcp.getTofsPresent(mcpPresentIt.first);
            for (int cnt{0}; cnt < tofsPresent.size(); ++cnt)
            {
                if (!tofsPresent[cnt])
                {
                    if (cnt == McpLut::McpType::TOF12 && mcpPresentIt.first == 2)
                        Fill(h.m_tof_mcpX[cnt][mcpPresentIt.first],
                             -mcp.pos.at(mcpPresentIt.first - 1).x(), mcp.TOF.at(0).at(cnt));
                    continue;
                }
                Fill(h.h_TOF[cnt][mcpPresentIt.first], mcp.TOF.at(mcpPresentIt.first - 1).at(cnt));
                Fill(h.m_tof_mcpX[cnt][mcpPresentIt.first], -mcp.pos.at(mcpPresentIt.first - 1).x(),
                     mcp.TOF.at(mcpPresentIt.first - 1).at(cnt));
                //
                //}
            }

        } // mcp present

        Fill(h.h_E, mcp.p1.E() - mcp.p1.M()); // kinetic energy
        Fill(h.h_beta, mcp.p1.Beta());

        // mcp-mcp coincidence
        if (mcp.bothMCP)
        {

            Fill(h.m_X1X2, -mcp.pos.at(0).x(), -mcp.pos.at(1).x());
            Fill(h.m_Y1Y2, mcp.pos.at(0).y(), mcp.pos.at(1).y());

            for (unsigned int i{0}; i < conf.mcp.nDetectors; i++)
            {
                for (unsigned int j{0}; j < i; j++)
                {
                    Fill(h.m_mcp_mcp_theta, mcp.pos.at(i).Theta() / UNITS::deg,
                         mcp.pos.at(j).Theta() / UNITS::deg);
                    Fill(h.m_mcp_mcp_phi, mcp.pos.at(i).Phi() / UNITS::deg,
                         mcp.pos.at(j).Phi() / UNITS::deg);
                }
            }

            Fill(h.m_XY_target, -mcp.postarget.x(), mcp.postarget.y());
            if (mcp.bline1 && mcp.bline2)
            {
                Fill(h.m_XY_target_bline, -mcp.postarget.x(), mcp.postarget.y());
            }
        }
    } // good positions

    return true;
}

void McpAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Mcp)
    {
        if (input.treeToRead->FindLeaf("nbMCP") == nullptr)
            throw std::runtime_error("EXOTIC MCP enabled in the config but not found in the tree");

        mcpReader.nbMCP     = new TTreeReaderValue<Int_t>(fReader, "nbMCP");
        mcpReader.TSMCP     = new TTreeReaderValue<ULong64_t>(fReader, "TSMCP");
        mcpReader.TSMCP_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSMCP_Hit");

        if (input.treeToRead->FindLeaf("MCPChannelId") != nullptr)
            mcpReader.MCPChannel =
                new TTreeReaderArray<UShort_t>(fReader, "MCPChannel.MCPChannelId");
        else
            mcpReader.MCPChannel = new TTreeReaderArray<UShort_t>(fReader, "MCPChannel");

        if (input.treeToRead->FindLeaf("MCPBoardId") != nullptr)
            mcpReader.MCPBoard = new TTreeReaderArray<UShort_t>(fReader, "MCPBoard.MCPBoardId");
        else
            mcpReader.MCPBoard = new TTreeReaderArray<UShort_t>(fReader, "MCPBoard");

        mcpReader.MCPEnergy = new TTreeReaderArray<Float_t>(fReader, "MCPEnergy");
        mcpReader.MCPTime   = new TTreeReaderArray<Float_t>(fReader, "MCPTime");
    }
}

void McpAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
}
