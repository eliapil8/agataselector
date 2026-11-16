#pragma once

#include "CoincidenceAnalysis.h"
#include "Monitor.h"
#include "DetectorAnalysis/McpAnalysis.h"
#include "DetectorAnalysis/MonitorAnalysis.h"
#include "Mcp.h"
#include "GateSelector.h"

class McpMonitorAnalysis : public CoincidenceAnalysis {
    struct McpMonitorHistograms : public CoincidenceHistograms {
    
    TH3D* m_ionTrack{nullptr};
    TH3D* m_layout_XYZ{nullptr};
    TH2D* m_Energy_monitorID{nullptr};
    TH2D* m_TSdiff_monitorID{nullptr};
    std::unordered_map<unsigned int, TH2D*> m_XY_target;
    std::unordered_map<unsigned int, TH2D*> m_X1X2;
    std::unordered_map<unsigned int, TH2D*> m_Y1Y2;
    std::unordered_map<unsigned int,std::unordered_map<unsigned int, TH2D*>> m_layout_front;
    std::unordered_map<unsigned int,std::unordered_map<unsigned int, TH2D*>> m_X_Energy;
    std::unordered_map<unsigned int,std::unordered_map<unsigned int, TH2D*>> m_layout_front_gated;
    std::unordered_map<unsigned int,std::unordered_map<unsigned int, TH2D*>> m_Energy_TOF_monitorID;
    std::unordered_map<unsigned int,std::unordered_map<unsigned int, TH3D*>> m_ionTrack_gatedETOF;
    
    
    } mcpMonitorHistograms;

  public:
    McpMonitorAnalysis(Inputs&, Mcp&, Monitor&, 
                        McpAnalysis::McpReader&,
                        MonitorAnalysis::MonitorReader&);
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  private:
    Mcp&                        mcp;
    Monitor&                    monitor;
    McpMonitorConf&             mcpMonitorConf;
    McpAnalysis::McpReader&     mcpReader;
    MonitorAnalysis::MonitorReader&     monitorReader;

  private:

    GateSelector<2,1> ETOFgates; 
};

