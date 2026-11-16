#include "CoincidenceAnalysis.h"
#include "Dante.h"
#include "DetectorAnalysis/McpAnalysis.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "Mcp.h"
#include "GateSelector.h"

class McpDanteAnalysis : public CoincidenceAnalysis {
    struct McpDanteHistograms : public CoincidenceHistograms {
     
        TH3D* m_ionTrack{nullptr};
        TH3D* m_layout_XYZ{nullptr};
        TH2D* m_TSdiff_danteID{nullptr};
        std::unordered_map<unsigned int, TH2D*> m_dante;
        std::unordered_map<unsigned int, TH2D*> m_XY_target;
        std::unordered_map<unsigned int, TH2D*> m_X1X2;
        std::unordered_map<unsigned int, TH2D*> m_Y1Y2;
        std::unordered_map<unsigned int,std::unordered_map<unsigned int, TH2D*>> m_layout_front;
        std::unordered_map<unsigned int,std::unordered_map<unsigned int, TH2D*>> m_layout_front_gated;
        
        
        } mcpDanteHistograms;
    
      public:
        McpDanteAnalysis(Inputs&, Mcp&, Dante&, 
                            McpAnalysis::McpReader&,
                            DanteAnalysis::DanteReader&);
        bool AllocateHistos() override;
        void Analyze() override;
        bool FillHistos() override;
        void Finalize() override;
    
      private:
        Mcp&                        mcp;
        Dante&                   dante;
        McpDanteConf&             mcpDanteConf;
        McpAnalysis::McpReader&     mcpReader;
        DanteAnalysis::DanteReader&     danteReader;
    
      private:
    
        //GateSelector<2,1> ETOFgates; 
    };
    
    