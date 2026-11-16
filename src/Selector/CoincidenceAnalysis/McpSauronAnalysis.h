#pragma once

#include "CoincidenceAnalysis.h"
#include "DetectorAnalysis/McpAnalysis.h"
#include "DetectorAnalysis/SauronAnalysis.h"

#include "Sauron.h"
#include "Mcp.h"
#include "SauronMcp.h"

#include "GateSelector.h"

class McpSauronAnalysis : public CoincidenceAnalysis {
  protected:
    McpSauronAnalysis(Inputs&, Mcp&, Sauron&, SauronConf&,
                        McpSauronConf&, McpAnalysis::McpReader&,
                        SauronAnalysis::SauronReader&, SauronMcp&);
    bool AllocateHistos() override;
    void Analyze() override;
    void Finalize() override;

  private:
    Mcp&                        mcp;
    Sauron&                     sauron;
    SauronMcp&                  sauronMcp;
    const McpSauronConf&        mcpSauronConf;
    const SauronConf& sauronConf;
    McpAnalysis::McpReader&     mcpReader;
    SauronAnalysis::SauronReader& sauronReader;

  protected:
    struct McpSauronHistograms : public CoincidenceHistograms {
        // put histo here
      TH1D* h_Ex{nullptr};
      TH2D* m_Ex_TSmTS{nullptr};
      TH2D* m_Ex_posX{nullptr};
      TH2D* m_Ex_posY{nullptr};
      
      struct ReactionHistograms : public HistogramStructure {
	TH1D* h_Ex{nullptr};
	TH1D* m_Ex_sauronId{nullptr};
	TH2D* m_Ex_Theta{nullptr};
	TH2D* m_Ex_TSmTS{nullptr};
      };
      std::unordered_map<std::vector<ReactionFragment::Ion>,
                         ReactionHistograms,
                         ReactionFragment::Ion::vec_hash_fn>
          reactionHistos;
    } mcpSauronHistograms;
};

class McpSauronFwAnalysis : public McpSauronAnalysis {
  public:
    McpSauronFwAnalysis(Inputs&, Mcp&, Sauron&,
                          McpAnalysis::McpReader&,
                          SauronAnalysis::SauronReader&, SauronMcp&);
    bool AllocateHistos() override;
};

class McpSauronBwAnalysis : public McpSauronAnalysis {
  public:
    McpSauronBwAnalysis(Inputs&, Mcp&, Sauron&,
                          McpAnalysis::McpReader&,
                          SauronAnalysis::SauronReader&, SauronMcp&);
    bool AllocateHistos() override;
};
