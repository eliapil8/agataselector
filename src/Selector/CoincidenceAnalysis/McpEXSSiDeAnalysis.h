#pragma once

#include "CoincidenceAnalysis.h"
#include "EXSSiDe.h"
#include "DetectorAnalysis/McpAnalysis.h"
#include "DetectorAnalysis/EXSSiDeAnalysis.h"
#include "Mcp.h"

class McpEXSSiDeAnalysis : public CoincidenceAnalysis {
    struct McpEXSSiDeHistograms : public CoincidenceHistograms {
        TH3D* m_layout_XYZ{nullptr};
        TH3D* m_ionTrack{nullptr};
        TH3D* m_ionTrackSource{nullptr};
        TH2D* m_XY_gatedSi{nullptr};
        TH2D* m_XY_gatedMCP{nullptr};
        TH2D* m_expectedXY_gatedMCP{nullptr};
        TH2D* m_obsX_expectedX{nullptr};
        TH2D* m_obsY_expectedY{nullptr};
        TH2D* m_obsX_expectedY{nullptr};
        TH2D* m_obsY_expectedX{nullptr};
        TH2D* m_chan_TS{nullptr};
        TH2D* m_XY_target{nullptr};
        TH2D* m_XY_Si{nullptr};
        TH1D* h_cTime_gated{nullptr};
        // TH1D* h_TSexssidemTSmcp{nullptr};
        //std::unordered_map<unsigned int, TH2D*> m_mcpId_TSmTS;
    } mcpExssideHistograms;

  public:
    McpEXSSiDeAnalysis(Inputs&, Mcp&, EXSSiDe&, 
                        McpAnalysis::McpReader&,
                        EXSSiDeAnalysis::EXSSiDeReader&);
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Mcp&                        mcp;
    EXSSiDe&                    exsside;
    McpEXSSiDeConf&             mcpEXSSiDeConf;
    McpAnalysis::McpReader&     mcpReader;
    EXSSiDeAnalysis::EXSSiDeReader&     exssideReader;

  private:
};

