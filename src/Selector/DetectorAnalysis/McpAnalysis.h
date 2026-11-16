#pragma once

#include "DetectorAnalysis.h"
#include "GateSelector.h"
#include "Mcp.h"

class McpAnalysis : public DetectorAnalysis
{
  public:
    explicit McpAnalysis(Inputs, Mcp &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Mcp &mcp;

    // Histograms
    struct McpHistograms : public DetectorHistograms
    {
        TH2D *m_chId_minTSdiff{nullptr};
        TH3D *m_layout_XYZ{nullptr};
        TH3D *m_expected_layout_XYZ{nullptr};
        TH3D *m_ionTrack{nullptr};
        TH3D *m_ionTrackSource{nullptr};
        TH2D *m_obsX_expectedX{nullptr};
        TH2D *m_obsY_expectedY{nullptr};
        TH2D *m_expectedXY_gatedMCP{nullptr};
        TH2D *m_XY_target{nullptr};
        TH2D *m_XY_target_bline{nullptr};
        TH2D *m_X1X2{nullptr};
        TH2D *m_Y1Y2{nullptr};
        TH1D *h_E{nullptr};
        TH1D *h_beta{nullptr};
        //   TH1D* h_mcpID{nullptr};
        TH2D                                    *m_goodEvtsStat_mcpID{nullptr};
        std::unordered_map<unsigned int, TH2D *> m_layout_front;
        std::unordered_map<unsigned int, TH2D *> m_layout_top;
        std::unordered_map<unsigned int, TH2D *> m_layout_side;
        std::unordered_map<unsigned int, TH2D *> m_XY_raw;
        std::unordered_map<unsigned int, TH1D *> h_rate_mcp;
        std::unordered_map<unsigned int, TH2D *> m_mcpID_TS;
        std::unordered_map<unsigned int, std::unordered_map<unsigned int, TH2D *>> m_tof_mcpRawX;
        std::unordered_map<unsigned int, std::unordered_map<unsigned int, TH2D *>> m_tof_mcpX;
        std::unordered_map<unsigned int, std::unordered_map<unsigned int, TH1D *>> h_TOF;
        TH2D *m_mcp_mcp_theta{nullptr};
        TH2D *m_mcp_mcp_phi{nullptr};
    } mcpHistograms;

    // Readers
  public:
    struct McpReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbMCP{nullptr};
        TTreeReaderValue<ULong64_t> *TSMCP{nullptr};
        TTreeReaderArray<ULong64_t> *TSMCP_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *MCPChannel{nullptr};
        TTreeReaderArray<UShort_t>  *MCPBoard{nullptr};
        TTreeReaderArray<Float_t>   *MCPEnergy{nullptr};
        TTreeReaderArray<Float_t>   *MCPTime{nullptr};
    } mcpReader;

  private:
    std::unordered_map<unsigned int, std::array<int, McpLut::lenMcpType>> mcpPresentMask;
    std::unordered_map<unsigned int, std::array<int, McpLut::lenMcpType>> mcpPresent;

    GateSelector<2, 1> xTOFgate; // BGS
    GateSelector<2, 1> rawxTOFgate;
    GateSelector<1, 1> rawXYgate;
};
